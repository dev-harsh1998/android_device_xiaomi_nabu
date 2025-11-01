/*
 * Copyright (C) 2025 Harshit Jain
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal Xiaomi CS35L41 Quad Amplifier HAL
 *
 * This HAL focuses only on hardware-specific operations not handled by
 * QCOM primary HAL and vendor mixer paths:
 * - Physical amplifier enable/disable
 * - DSP firmware verification and health monitoring
 * - Hardware-specific error detection
 *
 * All configuration (DSP settings, routing, multimedia tracks) is handled
 * by vendor mixer paths and QCOM primary HAL.
 */

 #define LOG_TAG "audio_amplifier_cs35l41_xiaomi"

 #include <errno.h>
 #include <hardware/audio_amplifier.h>
 #include <hardware/hardware.h>
 #include <log/log.h>
 #include <stdbool.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/time.h>
 #include <tinyalsa/asoundlib.h>
 
 #define UNUSED __attribute__((unused))
 
 /* CS35L41 Hardware Configuration */
 #define MAX_CS35L41_AMPS 4
 #define CS35L41_MIXER_CARD 0
 
 /* CS35L41 quad amplifier channels */
 static const char *CS35L41_CHANNELS[MAX_CS35L41_AMPS] = {"TL", "TR", "BL",
                                                          "BR"};
 
 /* Only controls not handled by mixer paths */
 #define CS35L41_AMP_ENABLE "AMP Enable"
 #define CS35L41_DSP_FIRMWARE "DSP1 Firmware"
 
 /* Amplifier states */
 typedef enum {
   AMP_STATE_UNINITIALIZED = 0,
   AMP_STATE_IDLE = 1,
   AMP_STATE_ACTIVE = 2,
   AMP_STATE_ERROR = 3
 } amp_state_t;
 
 /* Minimal device structure */
 typedef struct {
   amplifier_device_t amp_dev;
   struct mixer *mixer;
   amp_state_t state;
   bool dsp_healthy;
   struct timeval last_health_check;
   int error_count;
 } cs35l41_device_t;
 
 static cs35l41_device_t *g_amp_device = NULL;
 
 /**
  * Format channel-specific control name
  */
 static int format_control_name(const char *channel, const char *control,
                                char *buffer, size_t size) {
   if (!channel || !control || !buffer)
     return -EINVAL;
   return snprintf(buffer, size, "%s %s", channel, control);
 }
 
 /**
  * Set mixer control value
  */
 static int set_mixer_control(const char *name, int value) {
   struct mixer_ctl *ctl;
 
   if (!g_amp_device || !g_amp_device->mixer || !name) {
     ALOGE("%s: Invalid parameters", __func__);
     return -EINVAL;
   }
 
   ALOGV("%s: Setting control '%s' = %d", __func__, name, value);
 
   ctl = mixer_get_ctl_by_name(g_amp_device->mixer, name);
   if (!ctl) {
     ALOGW("%s: Control '%s' not found in mixer", __func__, name);
     return -ENOENT;
   }
 
   int ret = mixer_ctl_set_value(ctl, 0, value);
   if (ret < 0) {
     ALOGE("%s: Failed to set '%s'=%d: %s", __func__, name, value, strerror(-ret));
   }
 
   return ret;
 }
 
 /**
  * Verify DSP firmware is loaded and healthy
  * This is hardware-specific verification not done by mixer paths
  */
 static bool check_dsp_health(void) {
   struct timeval current_time;
   long time_diff;
   char control_name[64];
   struct mixer_ctl *ctl;
   int firmware_state;
   int healthy_count = 0;
 
   if (!g_amp_device) {
     ALOGE("%s: Device not initialized", __func__);
     return false;
   }
 
   if (!g_amp_device->mixer) {
     ALOGE("%s: Mixer not initialized", __func__);
     return false;
   }
 
   /* Use cached result if recent (within 10 seconds) */
   gettimeofday(&current_time, NULL);
   time_diff =
       (current_time.tv_sec - g_amp_device->last_health_check.tv_sec) * 1000 +
       (current_time.tv_usec - g_amp_device->last_health_check.tv_usec) / 1000;
 
   if (time_diff < 10000 && time_diff >= 0) {
     ALOGV("%s: Using cached DSP health result: %s (checked %ldms ago)",
               __func__, g_amp_device->dsp_healthy ? "healthy" : "unhealthy",
               time_diff);
     return g_amp_device->dsp_healthy;
   }
 
   ALOGI(
       "%s: Performing DSP health check (last check %ldms ago, error_count=%d)",
       __func__, time_diff, g_amp_device->error_count);
 
   /* Check DSP firmware state for all channels */
   for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
     if (format_control_name(CS35L41_CHANNELS[i], CS35L41_DSP_FIRMWARE,
                             control_name, sizeof(control_name)) < 0) {
       ALOGE("%s: Failed to format control name for %s", __func__,
                  CS35L41_CHANNELS[i]);
       continue;
     }
 
     ctl = mixer_get_ctl_by_name(g_amp_device->mixer, control_name);
     if (!ctl) {
       ALOGW("%s: DSP control '%s' not found for %s", __func__,
                  control_name, CS35L41_CHANNELS[i]);
       continue;
     }
 
     firmware_state = mixer_ctl_get_value(ctl, 0);
     if (firmware_state >= 0) {
       healthy_count++;
     } else {
       ALOGE("%s: DSP error on %s channel", __func__, CS35L41_CHANNELS[i]);
     }
   }
 
   /* Update cached health status */
   g_amp_device->dsp_healthy = (healthy_count == MAX_CS35L41_AMPS);
   g_amp_device->last_health_check = current_time;
 
   if (!g_amp_device->dsp_healthy) {
     g_amp_device->error_count++;
     ALOGE("%s: DSP health check failed: %d/%d channels healthy", __func__,
           healthy_count, MAX_CS35L41_AMPS);
   } else {
     if (g_amp_device->error_count > 0) {
       ALOGI("%s: DSP recovered", __func__);
       g_amp_device->error_count = 0;
     }
   }
 
   return g_amp_device->dsp_healthy;
 }
 
 /**
  * Enable/disable physical amplifier channel
  * This handles hardware-specific physical amplifier control
  */
 static int control_amp_channel(const char *channel, bool enable) {
   char control_name[64];
   int ret;
 
   if (!channel) {
     ALOGE("%s: Invalid channel parameter", __func__);
     return -EINVAL;
   }
 
   /* Physical amplifier enable/disable */
   if (format_control_name(channel, CS35L41_AMP_ENABLE, control_name,
                           sizeof(control_name)) >= 0) {
     ret = set_mixer_control(control_name, enable ? 1 : 0);
     if (ret < 0) {
       ALOGE("%s: Failed to %s %s amplifier", __func__,
                  enable ? "enable" : "disable", channel);
       return ret;
     }
   } else {
     ALOGE("%s: Failed to format control name for %s", __func__, channel);
     return -EINVAL;
   }
   return 0;
 }
 
 /**
  * Enable all amplifier channels
  */
 static int enable_all_amplifiers(void) {
   int ret = 0;
   int failed_count = 0;
 
   if (!check_dsp_health()) {
     ALOGE("%s: DSP unhealthy - refusing to enable amplifiers", __func__);
     if (g_amp_device) {
       g_amp_device->state = AMP_STATE_ERROR;
     }
     return -EIO;
   }
 
   for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
     int channel_ret = control_amp_channel(CS35L41_CHANNELS[i], true);
     if (channel_ret < 0) {
       failed_count++;
       ret = channel_ret;
     }
   }
 
   if (failed_count == 0) {
     g_amp_device->state = AMP_STATE_ACTIVE;
     ALOGI("%s: All amplifiers enabled", __func__);
   } else if (failed_count < MAX_CS35L41_AMPS) {
     g_amp_device->state = AMP_STATE_ACTIVE;
     ALOGW("%s: Partial enable: %d/%d failed", __func__, failed_count, MAX_CS35L41_AMPS);
   } else {
     g_amp_device->state = AMP_STATE_ERROR;
     ALOGE("%s: All amplifiers failed to enable", __func__);
     return -EIO;
   }
   return ret;
 }
 
 /**
  * Disable all amplifier channels
  */
 static int disable_all_amplifiers(void) {
   int failed_count = 0;
 
   for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
     if (control_amp_channel(CS35L41_CHANNELS[i], false) < 0) {
       failed_count++;
     }
   }
 
   g_amp_device->state = AMP_STATE_IDLE;
 
   if (failed_count == 0) {
     ALOGI("%s: All amplifiers disabled", __func__);
   } else {
     ALOGW("%s: Partial disable: %d/%d failed", __func__, failed_count, MAX_CS35L41_AMPS);
   }
   return 0;
 }
 
 /* ========================================================================
  * Android Amplifier HAL Interface Implementation
  * ======================================================================== */
 
 static int cs35l41_enable_output_devices(UNUSED struct amplifier_device *device,
                                          UNUSED uint32_t devices, bool enable) {
   if (!g_amp_device) {
     ALOGE("%s: Amplifier device not initialized", __func__);
     return -ENODEV;
   }
 
   if (!g_amp_device->mixer) {
     ALOGE("%s: Mixer not initialized", __func__);
     return -ENODEV;
   }
 
   int ret = enable ? enable_all_amplifiers() : disable_all_amplifiers();
 
   if (ret != 0) {
     ALOGE("%s: Amplifier %s failed: %d", __func__,
           enable ? "enable" : "disable", ret);
   }
 
   return ret;
 }
 
 static int cs35l41_calibrate(UNUSED struct amplifier_device *device,
                              UNUSED void *adev) {
   if (!g_amp_device) {
     ALOGE("%s: Amplifier device not initialized", __func__);
     return -ENODEV;
   }
 
   ALOGI("%s: Starting CS35L41 initialization", __func__);
 
   /* Open mixer for hardware control */
   g_amp_device->mixer = mixer_open(CS35L41_MIXER_CARD);
   if (!g_amp_device->mixer) {
     ALOGE("%s: Failed to open mixer card %d", __func__, CS35L41_MIXER_CARD);
     g_amp_device->state = AMP_STATE_ERROR;
     return -ENODEV;
   }
 
   /* Verify DSP health */
   if (!check_dsp_health()) {
     ALOGE("%s: DSP health check failed during initialization", __func__);
     g_amp_device->state = AMP_STATE_ERROR;
     return -EIO;
   }
 
   g_amp_device->state = AMP_STATE_IDLE;
   ALOGI("%s: CS35L41 initialization complete", __func__);
 
   return 0;
 }
 
 /**
  * Close device
  */
 static int cs35l41_dev_close(hw_device_t *device) {
   cs35l41_device_t *dev = (cs35l41_device_t *)device;
 
   if (!dev) {
     ALOGE("%s: Invalid device parameter", __func__);
     return -EINVAL;
   }
 
   ALOGI("%s: Closing CS35L41 amplifier device (state: %d)", __func__,
              dev->state);
 
   /* Disable amplifiers before closing */
   if (dev->mixer && dev->state == AMP_STATE_ACTIVE) {
     ALOGD("%s: Disabling active amplifiers before closing", __func__);
     disable_all_amplifiers();
   }
 
   /* Close mixer */
   if (dev->mixer) {
     ALOGD("%s: Closing mixer handle", __func__);
     mixer_close(dev->mixer);
     dev->mixer = NULL;
   }
 
   /* Free device structure */
   free(dev);
   g_amp_device = NULL;
 
   ALOGI("%s: CS35L41 amplifier device closed successfully", __func__);
   return 0;
 }
 
 /**
  * Open amplifier module
  */
 static int cs35l41_module_open(const hw_module_t *module, const char *name,
                                hw_device_t **device) {
   cs35l41_device_t *dev;
 
   ALOGI("%s: Opening minimal Xiaomi CS35L41 amplifier HAL", __func__);
 
   if (strcmp(name, AMPLIFIER_HARDWARE_INTERFACE) != 0) {
     ALOGE("%s: Invalid interface name: %s (expected: %s)", __func__, name,
                AMPLIFIER_HARDWARE_INTERFACE);
     return -EINVAL;
   }
 
   if (g_amp_device) {
     ALOGE("%s: Device already opened", __func__);
     return -EBUSY;
   }
 
   dev = calloc(1, sizeof(cs35l41_device_t));
   if (!dev) {
     ALOGE("%s: Failed to allocate device structure", __func__);
     return -ENOMEM;
   }
 
   /* Initialize device structure */
   dev->amp_dev.common.tag = HARDWARE_DEVICE_TAG;
   dev->amp_dev.common.module = (hw_module_t *)module;
   dev->amp_dev.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
   dev->amp_dev.common.close = cs35l41_dev_close;
 
   /* Set HAL interface functions - only implement what's needed */
   dev->amp_dev.set_input_devices = NULL;    /* Handled by QCOM HAL */
   dev->amp_dev.set_output_devices = NULL;   /* Handled by QCOM HAL */
   dev->amp_dev.enable_input_devices = NULL; /* Handled by QCOM HAL */
   dev->amp_dev.enable_output_devices = cs35l41_enable_output_devices;
   dev->amp_dev.calibrate = cs35l41_calibrate;
   dev->amp_dev.input_stream_start = NULL;    /* Not needed */
   dev->amp_dev.input_stream_standby = NULL;  /* Not needed */
   dev->amp_dev.output_stream_start = NULL;   /* Not needed */
   dev->amp_dev.output_stream_standby = NULL; /* Not needed */
   dev->amp_dev.set_mode = NULL;              /* Not needed */
   dev->amp_dev.set_parameters = NULL;        /* Not needed */
   dev->amp_dev.in_set_parameters = NULL;     /* Not needed */
   dev->amp_dev.out_set_parameters = NULL;    /* Not needed */
   dev->amp_dev.set_feedback = NULL;          /* Not needed */
 
   /* Initialize state */
   dev->state = AMP_STATE_UNINITIALIZED;
   dev->dsp_healthy = false;
   dev->error_count = 0;
   memset(&dev->last_health_check, 0, sizeof(dev->last_health_check));
 
   g_amp_device = dev;
   *device = (hw_device_t *)dev;
 
   ALOGI(
       "%s: Minimal CS35L41 amplifier HAL opened successfully (state: %d)",
       __func__, dev->state);
   return 0;
 }
 
 /* HAL module methods */
 static struct hw_module_methods_t cs35l41_module_methods = {
     .open = cs35l41_module_open,
 };
 
 /* HAL module info */
 amplifier_module_t HAL_MODULE_INFO_SYM = {
     .common =
         {
             .tag = HARDWARE_MODULE_TAG,
             .module_api_version = AMPLIFIER_MODULE_API_VERSION_0_1,
             .hal_api_version = HARDWARE_HAL_API_VERSION,
             .id = AMPLIFIER_HARDWARE_MODULE_ID,
             .name = "Minimal Xiaomi CS35L41 Amplifier HAL",
             .author = "Harshit Jain",
             .methods = &cs35l41_module_methods,
         },
 }; 
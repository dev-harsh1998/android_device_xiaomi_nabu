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

/* Debug logging control - set to 1 to enable verbose logging, 0 for minimal
 * logging */
#ifndef CS35L41_DEBUG
#define CS35L41_DEBUG 1
#endif

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

/*
 * Debug logging macros - only enabled when CS35L41_DEBUG=1
 * Used for verbose debugging
 */
#if CS35L41_DEBUG
#define ALOGD_DBG(...) ALOGD(__VA_ARGS__)
#define ALOGV_DBG(...) ALOGV(__VA_ARGS__)
#else
#define ALOGD_DBG(...) ((void)0)
#define ALOGV_DBG(...) ((void)0)
#endif

/*
 * Critical logging macros
 * Used for errors, warnings, and important operational status
 */
#define ALOGE_CRIT(...) ALOGE(__VA_ARGS__) /* Always log errors */
#define ALOGW_CRIT(...) ALOGW(__VA_ARGS__) /* Always log warnings */
#define ALOGI_CRIT(...) ALOGI(__VA_ARGS__) /* Always log critical info */

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
    ALOGE_CRIT("%s: Invalid parameters - device=%p, mixer=%p, name=%p",
               __func__, g_amp_device,
               g_amp_device ? g_amp_device->mixer : NULL, name);
    return -EINVAL;
  }

  ALOGV_DBG("%s: Setting control '%s' = %d", __func__, name, value);

  ctl = mixer_get_ctl_by_name(g_amp_device->mixer, name);
  if (!ctl) {
    ALOGW_CRIT("%s: Control '%s' not found in mixer", __func__, name);
    return -ENOENT;
  }

  int ret = mixer_ctl_set_value(ctl, 0, value);
  if (ret < 0) {
    ALOGE_CRIT("%s: Failed to set '%s' = %d: %s (%d)", __func__, name, value,
               strerror(-ret), ret);
  } else {
    ALOGD_DBG("%s: Successfully set '%s' = %d", __func__, name, value);
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
    ALOGE_CRIT("%s: Device not initialized", __func__);
    return false;
  }

  if (!g_amp_device->mixer) {
    ALOGE_CRIT("%s: Mixer not initialized", __func__);
    return false;
  }

  /* Use cached result if recent (within 5 seconds) */
  gettimeofday(&current_time, NULL);
  time_diff =
      (current_time.tv_sec - g_amp_device->last_health_check.tv_sec) * 1000 +
      (current_time.tv_usec - g_amp_device->last_health_check.tv_usec) / 1000;

  if (time_diff < 5000 && time_diff >= 0) {
    ALOGV_DBG("%s: Using cached DSP health result: %s (checked %ldms ago)",
              __func__, g_amp_device->dsp_healthy ? "healthy" : "unhealthy",
              time_diff);
    return g_amp_device->dsp_healthy;
  }

  ALOGI_CRIT(
      "%s: Performing DSP health check (last check %ldms ago, error_count=%d)",
      __func__, time_diff, g_amp_device->error_count);

  /* Check DSP firmware state for all channels */
  for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
    if (format_control_name(CS35L41_CHANNELS[i], CS35L41_DSP_FIRMWARE,
                            control_name, sizeof(control_name)) < 0) {
      ALOGE_CRIT("%s: Failed to format control name for %s", __func__,
                 CS35L41_CHANNELS[i]);
      continue;
    }

    ALOGV_DBG("%s: Checking DSP firmware for %s (%s)", __func__,
              CS35L41_CHANNELS[i], control_name);

    ctl = mixer_get_ctl_by_name(g_amp_device->mixer, control_name);
    if (!ctl) {
      ALOGW_CRIT("%s: DSP control '%s' not found for %s", __func__,
                 control_name, CS35L41_CHANNELS[i]);
      continue;
    }

    firmware_state = mixer_ctl_get_value(ctl, 0);
    if (firmware_state >= 0) {
      healthy_count++;
      ALOGV_DBG("%s: %s DSP firmware state: %d (healthy)", __func__,
                CS35L41_CHANNELS[i], firmware_state);
    } else {
      ALOGE_CRIT("%s: DSP error on %s channel: firmware_state=%d", __func__,
                 CS35L41_CHANNELS[i], firmware_state);
    }
  }

  /* Update cached health status */
  g_amp_device->dsp_healthy = (healthy_count == MAX_CS35L41_AMPS);
  g_amp_device->last_health_check = current_time;

  if (!g_amp_device->dsp_healthy) {
    g_amp_device->error_count++;
    ALOGE_CRIT(
        "%s: DSP health check failed: %d/%d channels healthy, error_count=%d",
        __func__, healthy_count, MAX_CS35L41_AMPS, g_amp_device->error_count);
  } else {
    if (g_amp_device->error_count > 0) {
      ALOGI_CRIT("%s: DSP recovered from error state (was %d errors)", __func__,
                 g_amp_device->error_count);
      g_amp_device->error_count = 0;
    } else {
      ALOGI_CRIT("%s: DSP health check passed: all %d channels healthy",
                 __func__, MAX_CS35L41_AMPS);
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
    ALOGE_CRIT("%s: Invalid channel parameter", __func__);
    return -EINVAL;
  }

  ALOGD_DBG("%s: %s amplifier channel %s", __func__,
            enable ? "Enabling" : "Disabling", channel);

  /* Physical amplifier enable/disable */
  if (format_control_name(channel, CS35L41_AMP_ENABLE, control_name,
                          sizeof(control_name)) >= 0) {
    ret = set_mixer_control(control_name, enable ? 1 : 0);
    if (ret < 0) {
      ALOGE_CRIT("%s: Failed to %s %s physical amplifier: %d", __func__,
                 enable ? "enable" : "disable", channel, ret);
      return ret;
    }
  } else {
    ALOGE_CRIT("%s: Failed to format AMP Enable control name for %s", __func__,
               channel);
    return -EINVAL;
  }

  ALOGD_DBG("%s: %s %s amplifier", __func__, enable ? "Enabled" : "Disabled",
            channel);
  return 0;
}

/**
 * Enable all amplifier channels
 */
static int enable_all_amplifiers(void) {
  int ret = 0;
  int failed_count = 0;

  ALOGI_CRIT("%s: Enabling all CS35L41 amplifiers (current state: %d)",
             __func__, g_amp_device ? g_amp_device->state : -1);

  if (!check_dsp_health()) {
    ALOGE_CRIT("%s: DSP unhealthy - refusing to enable amplifiers", __func__);
    if (g_amp_device) {
      g_amp_device->state = AMP_STATE_ERROR;
    }
    return -EIO;
  }

  for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
    int channel_ret = control_amp_channel(CS35L41_CHANNELS[i], true);
    if (channel_ret < 0) {
      ALOGE_CRIT("%s: Failed to enable %s amplifier: %d", __func__,
                 CS35L41_CHANNELS[i], channel_ret);
      failed_count++;
      ret = channel_ret;
    }
  }

  if (failed_count == 0) {
    g_amp_device->state = AMP_STATE_ACTIVE;
    ALOGI_CRIT("%s: All amplifiers enabled successfully (%d/%d)", __func__,
               MAX_CS35L41_AMPS, MAX_CS35L41_AMPS);
  } else if (failed_count < MAX_CS35L41_AMPS) {
    g_amp_device->state = AMP_STATE_ACTIVE;
    ALOGW_CRIT("%s: Partial amplifier enable: %d/%d succeeded, %d failed",
               __func__, MAX_CS35L41_AMPS - failed_count, MAX_CS35L41_AMPS,
               failed_count);
  } else {
    g_amp_device->state = AMP_STATE_ERROR;
    ALOGE_CRIT("%s: All amplifiers failed to enable (%d/%d failed)", __func__,
               failed_count, MAX_CS35L41_AMPS);
    return -EIO;
  }
  return ret;
}

/**
 * Disable all amplifier channels
 */
static int disable_all_amplifiers(void) {
  int failed_count = 0;

  ALOGI_CRIT("%s: Disabling all CS35L41 amplifiers (current state: %d)",
             __func__, g_amp_device ? g_amp_device->state : -1);

  for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
    if (control_amp_channel(CS35L41_CHANNELS[i], false) < 0) {
      ALOGE_CRIT("%s: Failed to disable %s amplifier", __func__,
                 CS35L41_CHANNELS[i]);
      failed_count++;
    }
  }

  g_amp_device->state = AMP_STATE_IDLE;

  if (failed_count == 0) {
    ALOGI_CRIT("%s: All amplifiers disabled successfully (%d/%d)", __func__,
               MAX_CS35L41_AMPS, MAX_CS35L41_AMPS);
  } else {
    ALOGW_CRIT("%s: Partial amplifier disable: %d/%d succeeded, %d failed",
               __func__, MAX_CS35L41_AMPS - failed_count, MAX_CS35L41_AMPS,
               failed_count);
  }
  return 0;
}

/* ========================================================================
 * Android Amplifier HAL Interface Implementation
 * ======================================================================== */

static int cs35l41_enable_output_devices(UNUSED struct amplifier_device *device,
                                         UNUSED uint32_t devices, bool enable) {
  if (!g_amp_device) {
    ALOGE_CRIT("%s: Amplifier device not initialized", __func__);
    return -ENODEV;
  }

  if (!g_amp_device->mixer) {
    ALOGE_CRIT("%s: Mixer not initialized", __func__);
    return -ENODEV;
  }

  ALOGI_CRIT("%s: Amplifiers %s requested for devices=0x%x", __func__,
             enable ? "enable" : "disable", devices);
  ALOGD_DBG("%s: Current HAL state: %d, DSP healthy: %s, error count: %d",
            __func__, g_amp_device->state,
            g_amp_device->dsp_healthy ? "yes" : "no",
            g_amp_device->error_count);

  int ret = enable ? enable_all_amplifiers() : disable_all_amplifiers();

  ALOGI_CRIT("%s: Amplifier operation %s, result: %s (%d)", __func__,
             enable ? "enable" : "disable", ret == 0 ? "success" : "failed",
             ret);

  return ret;
}

static int cs35l41_calibrate(UNUSED struct amplifier_device *device,
                             UNUSED void *adev) {
  if (!g_amp_device) {
    ALOGE_CRIT("%s: Amplifier device not initialized", __func__);
    return -ENODEV;
  }

  ALOGI_CRIT("%s: Starting CS35L41 quad amplifier initialization", __func__);
  ALOGD_DBG("%s: Note: Firmware loading and configuration handled by vendor "
            "mixer paths",
            __func__);

  /* Open mixer for hardware control */
  ALOGD_DBG("%s: Opening mixer card %d for hardware control", __func__,
            CS35L41_MIXER_CARD);
  g_amp_device->mixer = mixer_open(CS35L41_MIXER_CARD);
  if (!g_amp_device->mixer) {
    ALOGE_CRIT("%s: Failed to open mixer card %d", __func__,
               CS35L41_MIXER_CARD);
    g_amp_device->state = AMP_STATE_ERROR;
    return -ENODEV;
  }
  ALOGI_CRIT("%s: Mixer card %d opened successfully", __func__,
             CS35L41_MIXER_CARD);

  /* Verify DSP health after mixer paths have configured everything */
  ALOGD_DBG(
      "%s: Verifying DSP firmware health (should be loaded by mixer paths)",
      __func__);
  if (!check_dsp_health()) {
    ALOGE_CRIT("%s: DSP health check failed during initialization", __func__);
    ALOGE_CRIT("%s: This likely means vendor mixer paths didn't configure "
               "firmware properly",
               __func__);
    g_amp_device->state = AMP_STATE_ERROR;
    return -EIO;
  }

  g_amp_device->state = AMP_STATE_IDLE;
  ALOGI_CRIT("%s: CS35L41 quad amplifier initialization complete (state: %d, "
             "error count: %d)",
             __func__, g_amp_device->state, g_amp_device->error_count);

  return 0;
}

/**
 * Close device
 */
static int cs35l41_dev_close(hw_device_t *device) {
  cs35l41_device_t *dev = (cs35l41_device_t *)device;

  if (!dev) {
    ALOGE_CRIT("%s: Invalid device parameter", __func__);
    return -EINVAL;
  }

  ALOGI_CRIT("%s: Closing CS35L41 amplifier device (state: %d)", __func__,
             dev->state);

  /* Disable amplifiers before closing */
  if (dev->mixer && dev->state == AMP_STATE_ACTIVE) {
    ALOGD_DBG("%s: Disabling active amplifiers before closing", __func__);
    disable_all_amplifiers();
  }

  /* Close mixer */
  if (dev->mixer) {
    ALOGD_DBG("%s: Closing mixer handle", __func__);
    mixer_close(dev->mixer);
    dev->mixer = NULL;
  }

  /* Free device structure */
  free(dev);
  g_amp_device = NULL;

  ALOGI_CRIT("%s: CS35L41 amplifier device closed successfully", __func__);
  return 0;
}

/**
 * Open amplifier module
 */
static int cs35l41_module_open(const hw_module_t *module, const char *name,
                               hw_device_t **device) {
  cs35l41_device_t *dev;

  ALOGI_CRIT("%s: Opening minimal Xiaomi CS35L41 amplifier HAL", __func__);

  if (strcmp(name, AMPLIFIER_HARDWARE_INTERFACE) != 0) {
    ALOGE_CRIT("%s: Invalid interface name: %s (expected: %s)", __func__, name,
               AMPLIFIER_HARDWARE_INTERFACE);
    return -EINVAL;
  }

  if (g_amp_device) {
    ALOGE_CRIT("%s: Device already opened", __func__);
    return -EBUSY;
  }

  dev = calloc(1, sizeof(cs35l41_device_t));
  if (!dev) {
    ALOGE_CRIT("%s: Failed to allocate device structure", __func__);
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

  ALOGI_CRIT(
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
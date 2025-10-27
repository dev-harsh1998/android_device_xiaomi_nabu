/*
 * Copyright (C) 2025 Harshit Jain
 * SPDX-License-Identifier: Apache-2.0
 *
 * Xiaomi CS35L41 Quad Amplifier HAL
 */

 #define LOG_TAG "audio_amplifier_cs35l41_xiaomi"

 #include <errno.h>
 #include <fcntl.h>
 #include <log/log.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/time.h>
 #include <unistd.h>
 
 #include <hardware/audio_amplifier.h>
 #include <hardware/hardware.h>
 #include <tinyalsa/asoundlib.h>
 
 /* Utility macro for unused parameters */
 #define UNUSED __attribute__((unused))
 
 /* CS35L41 Quad Amplifier Configuration */
 #define MAX_CS35L41_AMPS 4
 #define CS35L41_MIXER_CARD 0
 
 /* Amplifier channel configuration - Quad setup (Top Left, Top Right, Bottom Left, Bottom Right) */
 static const char* CS35L41_AMP_CHANNELS[MAX_CS35L41_AMPS] = {
     "TL",  /* Top Left */
     "TR",  /* Top Right */
     "BL",  /* Bottom Left */
     "BR"   /* Bottom Right */
 };
 
 /* CS35L41 ALSA Mixer Control Names */
 #define CS35L41_CTL_DSP_FIRMWARE        "DSP1 Firmware"
 #define CS35L41_CTL_DSP_PRELOAD         "DSP1 Preload Switch"
 #define CS35L41_CTL_DRE_SWITCH          "DRE DRE Switch"
 #define CS35L41_CTL_PCM_SOURCE          "PCM Source"
 #define CS35L41_CTL_PCM_SOFT_RAMP       "PCM Soft Ramp"
 #define CS35L41_CTL_AMP_PCM_GAIN        "AMP PCM Gain"
 #define CS35L41_CTL_DIGITAL_PCM_VOLUME  "Digital PCM Volume"
 #define CS35L41_CTL_ASP_TX1_SOURCE      "ASP TX1 Source"
 #define CS35L41_CTL_ASP_TX2_SOURCE      "ASP TX2 Source"
 #define CS35L41_CTL_ASP_TX3_SOURCE      "ASP TX3 Source"
 #define CS35L41_CTL_ASP_TX4_SOURCE      "ASP TX4 Source"
 #define CS35L41_CTL_VPBR_CONFIG         "VPBR Config"
 #define CS35L41_CTL_NOISE_GATE_CONFIG   "Noise Gate Config"
 #define CS35L41_CTL_AMP_ENABLE          "AMP Enable"
 #define CS35L41_CTL_MAIN_AMP_ENABLE     "Main AMP Enable Switch"
 #define CS35L41_CTL_ASPRX1_SLOT_POS     "ASPRX1 Slot Position"
 
 /* Hardware configuration values optimized for CS35L41 */
 #define CS35L41_VPBR_CONFIG_VALUE       33575688    /* VPBR brown-out protection configuration */
 #define CS35L41_NOISE_GATE_VALUE        16245       /* Hardware noise gate threshold */
 #define CS35L41_AMP_PCM_GAIN_VALUE      18          /* Amplifier PCM gain in dB */
 #define CS35L41_DIGITAL_PCM_VOLUME      817         /* Digital PCM volume level */
 #define CS35L41_ASPRX1_SLOT_POSITION    4           /* ASP RX1 TDM slot position */
 
 /* TDM interface configuration */
 #define TDM_INTERFACE_NAME              "QUAT_TDM_RX_0"
 #define TDM_CTL_CHANNELS                "QUAT_TDM_RX_0 Channels"
 #define TDM_CTL_FORMAT                  "QUAT_TDM_RX_0 Format"
 #define TDM_CTL_SAMPLE_RATE             "QUAT_TDM_RX_0 SampleRate"
 #define TDM_VALUE_CHANNELS              "Two"       /* Stereo */
 #define TDM_VALUE_FORMAT                "S24_LE"    /* 24-bit little endian */
 #define TDM_VALUE_SAMPLE_RATE           "KHZ_48"    /* 48kHz sample rate */
 
 /* Buffer sizes for mixer control names */
 #define CS35L41_CTL_NAME_MAX_LEN        128
 
 /* Multimedia tracks for audio routing - supports all Android audio stream types */
 static const char* CS35L41_MULTIMEDIA_TRACKS[] = {
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia1",   /* deep-buffer-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia2",   /* low-latency-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia3",   /* multi-channel-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia4",   /* compress-offload-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia5",   /* audio-ull-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia6",   /* compress-offload-playback2 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia7",   /* compress-offload-playback3 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia8",   /* multimedia8-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia9",   /* fm-virtual-playback */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia10",  /* audio-playback-voip */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia11",  /* compress-offload-playback4 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia12",  /* compress-offload-playback5 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia13",  /* compress-offload-playback6 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia14",  /* compress-offload-playback7 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia15",  /* compress-offload-playback8 */
     "QUAT_TDM_RX_0 Audio Mixer MultiMedia16"   /* compress-offload-playback9/mmap-playback */
 };
 
 #define CS35L41_MULTIMEDIA_TRACKS_COUNT \
     (sizeof(CS35L41_MULTIMEDIA_TRACKS) / sizeof(CS35L41_MULTIMEDIA_TRACKS[0]))
 
 /* Amplifier device state */
 typedef enum {
     AMP_STATE_UNINITIALIZED = 0,
     AMP_STATE_IDLE = 1,
     AMP_STATE_ACTIVE = 2,
     AMP_STATE_DSP_ERROR = 3
 } cs35l41_amp_state_t;
 
 /* Main amplifier device structure */
 typedef struct cs35l41_amp_device {
     amplifier_device_t amp_dev;         /* Standard Android amplifier device */
     struct mixer* mixer;                /* ALSA mixer handle */
     cs35l41_amp_state_t state;          /* Current amplifier state */
     bool amplifiers_configured;         /* Track if amplifiers have been configured */
     uint32_t last_devices;              /* Cache last configured device mask to prevent redundant reconfigurations */
     bool last_enable_state;             /* Cache last enable/disable state */
     struct timeval last_operation_time; /* Time of last enable/disable operation */
     bool dsp_error_state;               /* Track DSP error state */
     int dsp_error_count;                /* Count of DSP errors for diagnostics */
     bool dsp_health_cached;             /* Cache DSP health check result */
     struct timeval dsp_health_check_time; /* Time of last DSP health check */
 } cs35l41_amp_device_t;
 
 /* Global device instance */
 static cs35l41_amp_device_t* g_cs35l41_device = NULL;
 
 /* Logging rate limiting to prevent log spam */
 static struct timeval g_last_log_time = {0, 0};
 #define LOG_RATE_LIMIT_MS 2000  /* 2 seconds */
 
 /* Forward declarations */
 static int cs35l41_format_ctl_name(const char* base_name, const char* channel,
                                    char* buf_out, size_t buf_size);
 
 /**
  * Check if logging should occur based on rate limit
  * @return true if logging is allowed, false otherwise
  */
 static bool cs35l41_should_log(void) {
     struct timeval current_time;
     gettimeofday(&current_time, NULL);
 
     long diff_ms = (current_time.tv_sec - g_last_log_time.tv_sec) * 1000 +
                    (current_time.tv_usec - g_last_log_time.tv_usec) / 1000;
 
     if (diff_ms >= LOG_RATE_LIMIT_MS) {
         g_last_log_time = current_time;
         return true;
     }
 
     return false;
 }
 
 /**
  * Check DSP status for a specific channel to detect error states
  * @param channel Channel name (TL, TR, BL, BR)
  * @return 0 if DSP is healthy, negative error code if in error state
  */
 static int cs35l41_check_dsp_status(const char* channel) {
     char ctl_name[CS35L41_CTL_NAME_MAX_LEN];
     struct mixer_ctl* ctl;
     int ret;
     int firmware_state;
 
     if (!channel || !g_cs35l41_device || !g_cs35l41_device->mixer) {
         return -EINVAL;
     }
 
     /* Check DSP firmware status */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_DSP_FIRMWARE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret < 0) {
         return ret;
     }
 
     ctl = mixer_get_ctl_by_name(g_cs35l41_device->mixer, ctl_name);
     if (!ctl) {
         /* Control not found - assume DSP is not available/in error */
         return -ENODEV;
     }
 
     /* Try to read current firmware state - if this fails, DSP might be crashed */
     firmware_state = mixer_ctl_get_value(ctl, 0);
     if (firmware_state < 0) {
         ALOGE("%s: Failed to read DSP firmware state for %s: %d", __func__, channel, firmware_state);
         return -EIO;
     }
 
     /* DSP firmware should be loaded and running */
     ALOGV("%s: DSP firmware state for %s: %d", __func__, channel, firmware_state);
     return 0;
 }
 
 /**
  * Validate DSP health across all channels before amplifier operations
  * Uses intelligent caching to avoid excessive DSP status checks
  * @return true if DSP is healthy, false if in error state
  */
 static bool cs35l41_is_dsp_healthy(void) {
     struct timeval current_time;
     long time_diff_ms;
     int error_count = 0;
 
     if (!g_cs35l41_device) {
         return false;
     }
 
     /* Check if we can use cached DSP health result */
     gettimeofday(&current_time, NULL);
     time_diff_ms = (current_time.tv_sec - g_cs35l41_device->dsp_health_check_time.tv_sec) * 1000 +
                    (current_time.tv_usec - g_cs35l41_device->dsp_health_check_time.tv_usec) / 1000;
 
     /* Use cached result if check was recent (within 5 seconds) */
     if (time_diff_ms < 5000 && time_diff_ms >= 0) {
         ALOGV("%s: Using cached DSP health result: %s (checked %ldms ago)",
               __func__, g_cs35l41_device->dsp_health_cached ? "healthy" : "error", time_diff_ms);
         return g_cs35l41_device->dsp_health_cached;
     }
 
     /* If we're in persistent error state, extend cache time and return cached result */
     if (g_cs35l41_device->dsp_error_state && g_cs35l41_device->dsp_error_count > 5) {
         ALOGW("%s: DSP in persistent error state (%d errors), extending cache time",
               __func__, g_cs35l41_device->dsp_error_count);
         g_cs35l41_device->dsp_health_check_time = current_time;
         g_cs35l41_device->dsp_health_cached = false;
         return false;
     }
 
     ALOGV("%s: Performing fresh DSP health check (last check %ldms ago)", __func__, time_diff_ms);
 
     /* Perform fresh DSP status check for all channels */
     for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
         int ret = cs35l41_check_dsp_status(CS35L41_AMP_CHANNELS[i]);
         if (ret < 0) {
             error_count++;
             ALOGW("%s: DSP error detected on %s channel: %d",
                   __func__, CS35L41_AMP_CHANNELS[i], ret);
         }
     }
 
     /* Update DSP error state and cache result */
     if (error_count > 0) {
         if (!g_cs35l41_device->dsp_error_state) {
             ALOGE("%s: DSP errors detected on %d/%d channels - entering error state",
                   __func__, error_count, MAX_CS35L41_AMPS);
             g_cs35l41_device->dsp_error_state = true;
             g_cs35l41_device->state = AMP_STATE_DSP_ERROR;
         }
         g_cs35l41_device->dsp_error_count++;
         g_cs35l41_device->dsp_health_cached = false;
     } else {
         if (g_cs35l41_device->dsp_error_state) {
             /* DSP recovered */
             ALOGI("%s: DSP recovered from error state", __func__);
             g_cs35l41_device->dsp_error_state = false;
             g_cs35l41_device->dsp_error_count = 0;
             g_cs35l41_device->state = AMP_STATE_IDLE;
         }
         g_cs35l41_device->dsp_health_cached = true;
     }
 
     /* Update cache timestamp */
     g_cs35l41_device->dsp_health_check_time = current_time;
 
     return g_cs35l41_device->dsp_health_cached;
 }
 
 /**
  * Format mixer control name with channel prefix
  * @param base_name Base control name
  * @param channel Channel name (can be NULL for global controls)
  * @param buf_out Output buffer
  * @param buf_size Size of output buffer
  * @return Number of characters written, or negative error code
  */
 static int cs35l41_format_ctl_name(const char* base_name, const char* channel,
                                    char* buf_out, size_t buf_size) {
     if (!base_name || !buf_out || buf_size == 0) {
         return -EINVAL;
     }
 
     memset(buf_out, 0, buf_size);
 
     if (channel) {
         return snprintf(buf_out, buf_size, "%s %s", channel, base_name);
     } else {
         return snprintf(buf_out, buf_size, "%s", base_name);
     }
 }
 
 /**
  * Set integer mixer control value by name
  * @param ctl_name Control name
  * @param value Value to set
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_mixer_set_int(const char* ctl_name, int value) {
     struct mixer_ctl* ctl;
     int ret;
 
     if (!g_cs35l41_device || !g_cs35l41_device->mixer) {
         ALOGE("%s: Invalid mixer handle", __func__);
         return -EINVAL;
     }
 
     if (!ctl_name) {
         ALOGE("%s: Invalid control name", __func__);
         return -EINVAL;
     }
 
     ctl = mixer_get_ctl_by_name(g_cs35l41_device->mixer, ctl_name);
     if (!ctl) {
         if (cs35l41_should_log()) {
             ALOGW("%s: Control '%s' not found", __func__, ctl_name);
         }
         return -ENOENT;
     }
 
     /* Check if this is a multimedia mixer control that needs both stereo channels set */
     bool is_multimedia_mixer = (strstr(ctl_name, "Audio Mixer MultiMedia") != NULL);
 
     if (is_multimedia_mixer) {
         /* Set both stereo channels for multimedia mixers */
         ret = mixer_ctl_set_value(ctl, 0, value);
         if (ret < 0) {
             ALOGE("%s: Failed to set '%s' channel 0 to %d: %d", __func__, ctl_name, value, ret);
             return ret;
         }
 
         ret = mixer_ctl_set_value(ctl, 1, value);
         if (ret < 0) {
             ALOGE("%s: Failed to set '%s' channel 1 to %d: %d", __func__, ctl_name, value, ret);
             return ret;
         }
 
         ALOGV("%s: Set '%s' = %d (both stereo channels)", __func__, ctl_name, value);
     } else {
         /* Set single channel for other controls */
         ret = mixer_ctl_set_value(ctl, 0, value);
         if (ret < 0) {
             ALOGE("%s: Failed to set '%s' to %d: %d", __func__, ctl_name, value, ret);
             return ret;
         }
 
         ALOGV("%s: Set '%s' = %d", __func__, ctl_name, value);
     }
 
     return 0;
 }
 
 /**
  * Set enum mixer control value by string
  * @param ctl_name Control name
  * @param value String value to set
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_mixer_set_enum(const char* ctl_name, const char* value) {
     struct mixer_ctl* ctl;
     int ret;
 
     if (!g_cs35l41_device || !g_cs35l41_device->mixer) {
         ALOGE("%s: Invalid mixer handle", __func__);
         return -EINVAL;
     }
 
     if (!ctl_name || !value) {
         ALOGE("%s: Invalid parameters", __func__);
         return -EINVAL;
     }
 
     ctl = mixer_get_ctl_by_name(g_cs35l41_device->mixer, ctl_name);
     if (!ctl) {
         if (cs35l41_should_log()) {
             ALOGW("%s: Control '%s' not found", __func__, ctl_name);
         }
         return -ENOENT;
     }
 
     ret = mixer_ctl_set_enum_by_string(ctl, value);
     if (ret < 0) {
         ALOGE("%s: Failed to set '%s' to '%s': %d", __func__, ctl_name, value, ret);
         return ret;
     }
 
     ALOGV("%s: Set '%s' = '%s'", __func__, ctl_name, value);
     return 0;
 }
 
 /**
  * Load DSP firmware for specific amplifier channel
  * @param channel Channel name (TL, TR, BL, BR)
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_load_dsp_firmware(const char* channel) {
     char ctl_name[CS35L41_CTL_NAME_MAX_LEN];
     int ret;
 
     if (!channel) {
         ALOGE("%s: Invalid channel", __func__);
         return -EINVAL;
     }
 
     ALOGI("%s: Loading DSP firmware for %s channel", __func__, channel);
 
     /* Set DSP firmware to Protection mode */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_DSP_FIRMWARE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret < 0) {
         ALOGE("%s: Failed to format firmware control name", __func__);
         return ret;
     }
 
     ret = cs35l41_mixer_set_enum(ctl_name, "Protection");
     if (ret < 0) {
         ALOGE("%s: Failed to set Protection firmware for %s", __func__, channel);
         /* Mark DSP as in error state on critical firmware failure */
         if (g_cs35l41_device) {
             g_cs35l41_device->dsp_error_state = true;
             g_cs35l41_device->dsp_error_count++;
             g_cs35l41_device->state = AMP_STATE_DSP_ERROR;
             /* Invalidate cache to force fresh check next time */
             g_cs35l41_device->dsp_health_cached = false;
             memset(&g_cs35l41_device->dsp_health_check_time, 0, sizeof(g_cs35l41_device->dsp_health_check_time));
         }
         return ret;
     }
 
     /* Enable DSP preload */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_DSP_PRELOAD, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret < 0) {
         ALOGE("%s: Failed to format preload control name", __func__);
         return ret;
     }
 
     ret = cs35l41_mixer_set_int(ctl_name, 1);
     if (ret < 0) {
         ALOGW("%s: Failed to enable DSP preload for %s (non-critical)", __func__, channel);
         /* Non-critical, continue */
     }
 
     ALOGI("%s: DSP firmware loaded for %s", __func__, channel);
     return 0;
 }
 
 /**
  * Configure a single CS35L41 amplifier channel
  * @param channel Channel name (TL, TR, BL, BR)
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_configure_amplifier(const char* channel) {
     char ctl_name[CS35L41_CTL_NAME_MAX_LEN];
     int ret;
 
     if (!channel) {
         ALOGE("%s: Invalid channel", __func__);
         return -EINVAL;
     }
 
     ALOGI("%s: Configuring %s amplifier", __func__, channel);
 
     /* Verify DSP is accessible before configuration */
     if (!cs35l41_is_dsp_healthy()) {
         ALOGE("%s: DSP in error state - skipping %s amplifier configuration", __func__, channel);
         return -EIO;
     }
 
     /* Step 1: Load DSP firmware */
     ret = cs35l41_load_dsp_firmware(channel);
     if (ret < 0) {
         ALOGE("%s: DSP firmware load failed for %s: %d", __func__, channel, ret);
         return ret;
     }
 
     /* Step 2: Enable DRE (Dynamic Range Enhancement) */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_DRE_SWITCH, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, 1);
     }
 
     /* Step 3: Set PCM Source to ASP */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_PCM_SOURCE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_enum(ctl_name, "ASP");
     }
 
     /* Step 4: Set PCM Soft Ramp to 4ms */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_PCM_SOFT_RAMP, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_enum(ctl_name, "4ms");
     }
 
     /* Step 5: Set AMP PCM Gain */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_AMP_PCM_GAIN, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, CS35L41_AMP_PCM_GAIN_VALUE);
     }
 
     /* Step 6: Set Digital PCM Volume */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_DIGITAL_PCM_VOLUME, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, CS35L41_DIGITAL_PCM_VOLUME);
     }
 
     /* Step 7: Configure ASP TX Sources */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_ASP_TX1_SOURCE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_enum(ctl_name, "DSPTX1");
     }
 
     /* Set TX2-TX4 to Zero */
     const char* tx_sources[] = {
         CS35L41_CTL_ASP_TX2_SOURCE,
         CS35L41_CTL_ASP_TX3_SOURCE,
         CS35L41_CTL_ASP_TX4_SOURCE
     };
 
     for (size_t i = 0; i < sizeof(tx_sources) / sizeof(tx_sources[0]); i++) {
         ret = cs35l41_format_ctl_name(tx_sources[i], channel,
                                       ctl_name, sizeof(ctl_name));
         if (ret >= 0) {
             cs35l41_mixer_set_enum(ctl_name, "Zero");
         }
     }
 
     /* Step 8: Set ASPRX1 Slot Position */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_ASPRX1_SLOT_POS, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, CS35L41_ASPRX1_SLOT_POSITION);
     }
 
     /* Step 9: Configure VPBR (brown-out protection) */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_VPBR_CONFIG, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, CS35L41_VPBR_CONFIG_VALUE);
     }
 
     /* Step 10: Configure Noise Gate */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_NOISE_GATE_CONFIG, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, CS35L41_NOISE_GATE_VALUE);
     }
 
     /* Step 11: Ensure Main AMP Enable is set */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_MAIN_AMP_ENABLE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret >= 0) {
         cs35l41_mixer_set_int(ctl_name, 1);
     }
 
     ALOGI("%s: %s amplifier configured successfully", __func__, channel);
     return 0;
 }
 
 /**
  * Configure TDM interface for CS35L41 operation
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_configure_tdm_interface(void) {
     int ret;
 
     ALOGI("%s: Configuring %s interface", __func__, TDM_INTERFACE_NAME);
 
     /* Set to stereo (Two channels) */
     ret = cs35l41_mixer_set_enum(TDM_CTL_CHANNELS, TDM_VALUE_CHANNELS);
     if (ret < 0) {
         ALOGE("%s: Failed to set TDM channels", __func__);
         return ret;
     }
 
     /* Set format to 24-bit little endian */
     ret = cs35l41_mixer_set_enum(TDM_CTL_FORMAT, TDM_VALUE_FORMAT);
     if (ret < 0) {
         ALOGE("%s: Failed to set TDM format", __func__);
         return ret;
     }
 
     /* Set sample rate to 48kHz */
     ret = cs35l41_mixer_set_enum(TDM_CTL_SAMPLE_RATE, TDM_VALUE_SAMPLE_RATE);
     if (ret < 0) {
         ALOGE("%s: Failed to set TDM sample rate", __func__);
         return ret;
     }
 
     ALOGI("%s: TDM interface configured (Stereo, 24-bit, 48kHz)", __func__);
     return 0;
 }
 
 /**
  * Enable audio routing for multimedia tracks
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_enable_audio_routing(void) {
     int success_count = 0;
 
     ALOGI("%s: Enabling audio routing for %zu multimedia tracks",
           __func__, CS35L41_MULTIMEDIA_TRACKS_COUNT);
 
     for (size_t i = 0; i < CS35L41_MULTIMEDIA_TRACKS_COUNT; i++) {
         int ret = cs35l41_mixer_set_int(CS35L41_MULTIMEDIA_TRACKS[i], 1);
         if (ret == 0) {
             success_count++;
             ALOGV("%s: Enabled %s", __func__, CS35L41_MULTIMEDIA_TRACKS[i]);
         }
     }
 
     ALOGI("%s: Audio routing complete: %d/%zu tracks enabled",
           __func__, success_count, CS35L41_MULTIMEDIA_TRACKS_COUNT);
 
     return (success_count > 0) ? 0 : -EIO;
 }
 
 /**
  * Initialize all CS35L41 amplifiers and audio routing
  * This function performs complete system initialization synchronously
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_initialize_system(void) {
     int ret;
     int failed_amps = 0;
 
     if (!g_cs35l41_device) {
         ALOGE("%s: Device not initialized", __func__);
         return -EINVAL;
     }
 
     if (g_cs35l41_device->amplifiers_configured) {
         ALOGW("%s: Amplifiers already configured, skipping initialization", __func__);
         return 0;
     }
 
     ALOGI("%s: Starting CS35L41 quad amplifier system initialization", __func__);
 
     /* Initialize all four amplifiers */
     for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
         ret = cs35l41_configure_amplifier(CS35L41_AMP_CHANNELS[i]);
         if (ret < 0) {
             ALOGE("%s: Failed to configure %s amplifier: %d",
                   __func__, CS35L41_AMP_CHANNELS[i], ret);
             failed_amps++;
             /* Continue with other amplifiers */
         }
     }
 
     if (failed_amps == MAX_CS35L41_AMPS) {
         ALOGE("%s: All amplifiers failed to configure", __func__);
         return -EIO;
     }
 
     /* Configure TDM interface */
     ret = cs35l41_configure_tdm_interface();
     if (ret < 0) {
         ALOGE("%s: TDM interface configuration failed: %d", __func__, ret);
         /* Non-critical, continue */
     }
 
     /* Enable audio routing */
     ret = cs35l41_enable_audio_routing();
     if (ret < 0) {
         ALOGE("%s: Audio routing configuration failed: %d", __func__, ret);
         /* Non-critical, continue */
     }
 
     g_cs35l41_device->amplifiers_configured = true;
     g_cs35l41_device->state = AMP_STATE_IDLE;
 
     ALOGI("%s: CS35L41 system initialization complete (%d/%d amps OK)",
           __func__, MAX_CS35L41_AMPS - failed_amps, MAX_CS35L41_AMPS);
 
     return 0;
 }
 
 /**
  * Enable CS35L41 amplifier outputs synchronously
  * Called by AudioFlinger when output device is enabled
  * @param channel Channel name (TL, TR, BL, BR)
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_enable_amp_channel(const char* channel) {
     char ctl_name[CS35L41_CTL_NAME_MAX_LEN];
     int ret;
 
     if (!channel) {
         return -EINVAL;
     }
 
     /* Enable the amplifier */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_AMP_ENABLE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret < 0) {
         return ret;
     }
 
     ret = cs35l41_mixer_set_int(ctl_name, 1);
     if (ret < 0) {
         ALOGE("%s: Failed to enable %s amplifier", __func__, channel);
         return ret;
     }
 
     ALOGD("%s: %s amplifier enabled", __func__, channel);
     return 0;
 }
 
 /**
  * Disable CS35L41 amplifier outputs synchronously
  * Called by AudioFlinger when output device is disabled
  * @param channel Channel name (TL, TR, BL, BR)
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_disable_amp_channel(const char* channel) {
     char ctl_name[CS35L41_CTL_NAME_MAX_LEN];
     int ret;
 
     if (!channel) {
         return -EINVAL;
     }
 
     /* Disable the amplifier */
     ret = cs35l41_format_ctl_name(CS35L41_CTL_AMP_ENABLE, channel,
                                   ctl_name, sizeof(ctl_name));
     if (ret < 0) {
         return ret;
     }
 
     ret = cs35l41_mixer_set_int(ctl_name, 0);
     if (ret < 0) {
         ALOGE("%s: Failed to disable %s amplifier", __func__, channel);
         return ret;
     }
 
     ALOGD("%s: %s amplifier disabled", __func__, channel);
     return 0;
 }
 
 /* ============================================================================
  * Device-Aware Audio Routing Implementation
  * ============================================================================
  */
 
 /* Comprehensive audio device masks for proper routing decisions */
 #define CS35L41_BLUETOOTH_A2DP_DEVICES (AUDIO_DEVICE_OUT_BLUETOOTH_A2DP | \
                                        AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES | \
                                        AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER)
 
 #define CS35L41_BLUETOOTH_SCO_DEVICES  (AUDIO_DEVICE_OUT_BLUETOOTH_SCO | \
                                        AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET | \
                                        AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT)
 
 #define CS35L41_ALL_BLUETOOTH_DEVICES  (CS35L41_BLUETOOTH_A2DP_DEVICES | \
                                        CS35L41_BLUETOOTH_SCO_DEVICES)
 
 #define CS35L41_SPEAKER_DEVICES        (AUDIO_DEVICE_OUT_SPEAKER | \
                                        AUDIO_DEVICE_OUT_SPEAKER_SAFE)
 
 #define CS35L41_WIRED_DEVICES          (AUDIO_DEVICE_OUT_WIRED_HEADSET | \
                                        AUDIO_DEVICE_OUT_WIRED_HEADPHONE | \
                                        AUDIO_DEVICE_OUT_LINE)
 
 #define CS35L41_USB_DEVICES            (AUDIO_DEVICE_OUT_USB_DEVICE | \
                                        AUDIO_DEVICE_OUT_USB_HEADSET | \
                                        AUDIO_DEVICE_OUT_USB_ACCESSORY)
 
 /**
  * Enable all CS35L41 multimedia mixer controls
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_enable_all_multimedia_mixers(void) {
     int ret;
     int failed_count = 0;
 
     ALOGD("%s: Enabling all CS35L41 multimedia mixers", __func__);
 
     /* Enable all multimedia tracks for CS35L41 amplifier */
     for (size_t i = 0; i < CS35L41_MULTIMEDIA_TRACKS_COUNT; i++) {
         ret = cs35l41_mixer_set_int(CS35L41_MULTIMEDIA_TRACKS[i], 1);
         if (ret < 0) {
             ALOGE("%s: Failed to enable %s: %d", __func__,
                   CS35L41_MULTIMEDIA_TRACKS[i], ret);
             failed_count++;
         } else {
             ALOGV("%s: Enabled %s", __func__, CS35L41_MULTIMEDIA_TRACKS[i]);
         }
     }
 
     if (failed_count > 0) {
         ALOGW("%s: %d/%zu mixer controls failed to enable",
               __func__, failed_count, CS35L41_MULTIMEDIA_TRACKS_COUNT);
     }
 
     return (failed_count == CS35L41_MULTIMEDIA_TRACKS_COUNT) ? -EIO : 0;
 }
 
 /**
  * Disable all CS35L41 multimedia mixer controls
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_disable_all_multimedia_mixers(void) {
     int ret;
     int failed_count = 0;
 
     ALOGD("%s: Disabling all CS35L41 multimedia mixers", __func__);
 
     /* Disable all multimedia tracks for CS35L41 amplifier */
     for (size_t i = 0; i < CS35L41_MULTIMEDIA_TRACKS_COUNT; i++) {
         ret = cs35l41_mixer_set_int(CS35L41_MULTIMEDIA_TRACKS[i], 0);
         if (ret < 0) {
             ALOGE("%s: Failed to disable %s: %d", __func__,
                   CS35L41_MULTIMEDIA_TRACKS[i], ret);
             failed_count++;
         } else {
             ALOGV("%s: Disabled %s", __func__, CS35L41_MULTIMEDIA_TRACKS[i]);
         }
     }
 
     if (failed_count > 0) {
         ALOGW("%s: %d/%zu mixer controls failed to disable",
               __func__, failed_count, CS35L41_MULTIMEDIA_TRACKS_COUNT);
     }
 
     return (failed_count == CS35L41_MULTIMEDIA_TRACKS_COUNT) ? -EIO : 0;
 }
 
 /**
  * Configure CS35L41 amplifiers based on output devices
  * This prevents dual-output when Bluetooth is active
  *
  * @param devices Current output device mask
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_configure_for_devices(uint32_t devices) {
     int ret = 0;
     bool bluetooth_active = (devices & CS35L41_ALL_BLUETOOTH_DEVICES) != 0;
     bool speaker_active = (devices & CS35L41_SPEAKER_DEVICES) != 0;
     bool wired_active = (devices & CS35L41_WIRED_DEVICES) != 0;
     bool usb_active = (devices & CS35L41_USB_DEVICES) != 0;
     bool other_active = wired_active || usb_active;
 
     ALOGI("%s: devices=0x%x, bt=%d, spk=%d, wired=%d, usb=%d",
           __func__, devices, bluetooth_active, speaker_active, wired_active, usb_active);
 
     if (bluetooth_active && !speaker_active) {
         /* Bluetooth-only: Disable CS35L41 amplifiers completely */
         ALOGI("%s: Bluetooth-only mode - disabling CS35L41", __func__);
 
         ret = cs35l41_disable_all_multimedia_mixers();
         if (ret < 0) {
             ALOGE("%s: Failed to disable multimedia mixers: %d", __func__, ret);
             return ret;
         }
 
         /* Disable amplifier channels */
         for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
             int channel_ret = cs35l41_disable_amp_channel(CS35L41_AMP_CHANNELS[i]);
             if (channel_ret < 0) {
                 ALOGE("%s: Failed to disable channel %s: %d",
                       __func__, CS35L41_AMP_CHANNELS[i], channel_ret);
                 ret = channel_ret; /* Keep trying other channels */
             }
         }
 
         if (ret == 0) {
             g_cs35l41_device->state = AMP_STATE_IDLE;
         }
 
     } else if (speaker_active) {
         /* Speaker mode (with or without other devices): Enable CS35L41 */
         ALOGI("%s: Speaker mode - enabling CS35L41", __func__);
 
         /* Enable amplifier channels first */
         for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
             int channel_ret = cs35l41_enable_amp_channel(CS35L41_AMP_CHANNELS[i]);
             if (channel_ret < 0) {
                 ALOGE("%s: Failed to enable channel %s: %d",
                       __func__, CS35L41_AMP_CHANNELS[i], channel_ret);
                 ret = channel_ret; /* Keep trying other channels */
             }
         }
 
         /* Enable multimedia mixers */
         int mixer_ret = cs35l41_enable_all_multimedia_mixers();
         if (mixer_ret < 0) {
             ALOGE("%s: Failed to enable multimedia mixers: %d", __func__, mixer_ret);
             ret = mixer_ret;
         }
 
         if (ret == 0) {
             g_cs35l41_device->state = AMP_STATE_ACTIVE;
         }
 
         /* Log warning for dual output scenario */
         if (bluetooth_active) {
             ALOGW("%s: Dual output detected - both speaker and Bluetooth active", __func__);
         }
 
     } else if (other_active) {
         /* Wired/USB devices: Disable CS35L41 to avoid conflicts */
         ALOGI("%s: Wired/USB mode - disabling CS35L41", __func__);
 
         ret = cs35l41_disable_all_multimedia_mixers();
         if (ret == 0) {
             g_cs35l41_device->state = AMP_STATE_IDLE;
         }
 
     } else {
         /* No relevant devices or unknown configuration: Disable everything */
         ALOGI("%s: No active devices or unknown config - disabling CS35L41", __func__);
 
         ret = cs35l41_disable_all_multimedia_mixers();
         if (ret == 0) {
             g_cs35l41_device->state = AMP_STATE_IDLE;
         }
     }
 
     return ret;
 }
 
 /**
  * Set output devices - Handle device routing changes
  * This is the main function that prevents Bluetooth+Speaker dual output
  *
  * Called by AudioFlinger when output device routing changes.
  * Must complete synchronously and handle all device combinations properly.
  *
  * @param device Amplifier device handle (unused)
  * @param devices Current output device mask from AudioFlinger
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_amp_set_output_devices(UNUSED struct amplifier_device* device,
                                           uint32_t devices) {
     if (!g_cs35l41_device) {
         ALOGE("%s: Device not initialized", __func__);
         return -ENODEV;
     }
 
     if (!g_cs35l41_device->mixer) {
         ALOGE("%s: Mixer not initialized", __func__);
         return -ENODEV;
     }
 
     /* Only skip if this is truly a redundant call with no state change needed */
     bool devices_changed = (g_cs35l41_device->last_devices != devices);
     bool needs_initial_config = !g_cs35l41_device->amplifiers_configured;
 
     if (!devices_changed && !needs_initial_config && g_cs35l41_device->state != AMP_STATE_UNINITIALIZED) {
         ALOGV("%s: Devices unchanged (0x%x), skipping amplifier reconfiguration (state: %d)",
               __func__, devices, g_cs35l41_device->state);
         /* Still need to ensure multimedia mixers are properly configured for this device */
         return cs35l41_configure_for_devices(devices);
     }
 
     ALOGI("%s: Setting output devices to 0x%x (prev: 0x%x, state: %d)",
           __func__, devices, g_cs35l41_device->last_devices, g_cs35l41_device->state);
 
     /* Configure amplifiers based on active devices */
     int ret = cs35l41_configure_for_devices(devices);
     if (ret < 0) {
         ALOGE("%s: Failed to configure for devices 0x%x: %d", __func__, devices, ret);
         return ret;
     }
 
     /* Cache the configured device mask to prevent redundant reconfigurations */
     g_cs35l41_device->last_devices = devices;
 
     ALOGI("%s: Successfully configured for devices 0x%x (new state: %d)",
           __func__, devices, g_cs35l41_device->state);
 
     return 0;
 }
 
 /* ============================================================================
  * Android Audio Amplifier HAL Interface Implementation
  * ============================================================================
  */
 
 /**
  * Enable output devices - Synchronous implementation
  * This function is called by AudioFlinger and MUST complete synchronously
  *
  * @param device Amplifier device handle
  * @param devices Device mask
  * @param enable true to enable, false to disable
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_amp_enable_output_devices(UNUSED struct amplifier_device* device,
                                              uint32_t devices, bool enable) {
     int ret;
     int failed_count = 0;
     struct timeval current_time;
     long time_diff_ms;
 
     if (!g_cs35l41_device) {
         ALOGE("%s: Device not initialized", __func__);
         return -ENODEV;
     }
 
     if (!g_cs35l41_device->mixer) {
         ALOGE("%s: Mixer not initialized", __func__);
         return -ENODEV;
     }
 
     /* Check for redundant operations to prevent rapid disable/enable cycles */
     gettimeofday(&current_time, NULL);
     time_diff_ms = (current_time.tv_sec - g_cs35l41_device->last_operation_time.tv_sec) * 1000 +
                    (current_time.tv_usec - g_cs35l41_device->last_operation_time.tv_usec) / 1000;
 
     /* Skip redundant operations within 500ms window for same device and enable state */
     if (g_cs35l41_device->last_devices == devices &&
         g_cs35l41_device->last_enable_state == enable &&
         time_diff_ms < 500 && time_diff_ms >= 0) {
         ALOGV("%s: Skipping redundant operation - devices=0x%x, enable=%d, last_op=%ldms ago",
               __func__, devices, enable, time_diff_ms);
         return 0;
     }
 
     ALOGI("%s: devices=0x%x, enable=%d, state=%d, last_op=%ldms ago",
           __func__, devices, enable, g_cs35l41_device->state, time_diff_ms);
 
     /* Check DSP health before attempting amplifier operations */
     if (enable && !cs35l41_is_dsp_healthy()) {
         ALOGE("%s: DSP is in error state - refusing to enable amplifiers", __func__);
         return -EIO;
     }
 
     if (enable) {
         /* Enable all amplifier channels synchronously */
         for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
             ret = cs35l41_enable_amp_channel(CS35L41_AMP_CHANNELS[i]);
             if (ret < 0) {
                 ALOGE("%s: Failed to enable %s: %d",
                       __func__, CS35L41_AMP_CHANNELS[i], ret);
                 failed_count++;
             }
         }
 
         if (failed_count < MAX_CS35L41_AMPS) {
             /* Enable multimedia mixers for QUAT_TDM_RX_0 routing */
             ret = cs35l41_enable_all_multimedia_mixers();
             if (ret < 0) {
                 ALOGE("%s: Failed to enable multimedia mixers: %d", __func__, ret);
                 /* Continue - amplifiers are already enabled */
             }
 
             g_cs35l41_device->state = AMP_STATE_ACTIVE;
             ALOGI("%s: Amplifiers enabled (%d/%d channels OK)",
                   __func__, MAX_CS35L41_AMPS - failed_count, MAX_CS35L41_AMPS);
         } else {
             ALOGE("%s: All amplifier channels failed to enable", __func__);
             return -EIO;
         }
     } else {
         /* Disable all amplifier channels synchronously */
         for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
             ret = cs35l41_disable_amp_channel(CS35L41_AMP_CHANNELS[i]);
             if (ret < 0) {
                 ALOGE("%s: Failed to disable %s: %d",
                       __func__, CS35L41_AMP_CHANNELS[i], ret);
                 failed_count++;
             }
         }
 
         /* Disable multimedia mixers for QUAT_TDM_RX_0 routing */
         ret = cs35l41_disable_all_multimedia_mixers();
         if (ret < 0) {
             ALOGE("%s: Failed to disable multimedia mixers: %d", __func__, ret);
             /* Continue - amplifiers are already disabled */
         }
 
         g_cs35l41_device->state = AMP_STATE_IDLE;
         ALOGI("%s: Amplifiers disabled (%d/%d channels OK)",
               __func__, MAX_CS35L41_AMPS - failed_count, MAX_CS35L41_AMPS);
     }
 
     /* Update cached values for future optimizations */
     g_cs35l41_device->last_devices = devices;
     g_cs35l41_device->last_enable_state = enable;
     g_cs35l41_device->last_operation_time = current_time;
 
     return 0;
 }
 
 /**
  * Calibrate amplifier - Initialize the amplifier system
  * Called once during audio HAL initialization
  *
  * @param device Amplifier device handle
  * @param adev Audio device context (unused for CS35L41)
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_amp_calibrate(UNUSED struct amplifier_device* device,
                                  UNUSED void* adev) {
     int ret;
 
     ALOGI("%s: Starting amplifier calibration", __func__);
 
     if (!g_cs35l41_device) {
         ALOGE("%s: Device not initialized", __func__);
         return -ENODEV;
     }
 
     /* Open ALSA mixer */
     if (!g_cs35l41_device->mixer) {
         g_cs35l41_device->mixer = mixer_open(CS35L41_MIXER_CARD);
         if (!g_cs35l41_device->mixer) {
             ALOGE("%s: Failed to open mixer card %d", __func__, CS35L41_MIXER_CARD);
             return -ENODEV;
         }
         ALOGI("%s: Mixer opened for card %d", __func__, CS35L41_MIXER_CARD);
     }
 
     /* Initialize amplifier system */
     ret = cs35l41_initialize_system();
     if (ret < 0) {
         ALOGE("%s: System initialization failed: %d", __func__, ret);
         return ret;
     }
 
     ALOGI("%s: Amplifier calibration complete", __func__);
     return 0;
 }
 
 /**
  * Close amplifier device - Cleanup resources
  *
  * @param device Hardware device handle
  * @return 0 on success
  */
 static int cs35l41_amp_dev_close(hw_device_t* device) {
     cs35l41_amp_device_t* dev = (cs35l41_amp_device_t*)device;
 
     ALOGI("%s: Closing amplifier device", __func__);
 
     if (!dev) {
         ALOGE("%s: Device is NULL", __func__);
         return -EINVAL;
     }
 
     /* Disable all amplifiers before closing */
     if (dev->mixer && dev->state == AMP_STATE_ACTIVE) {
         for (int i = 0; i < MAX_CS35L41_AMPS; i++) {
             cs35l41_disable_amp_channel(CS35L41_AMP_CHANNELS[i]);
         }
     }
 
     /* Close mixer */
     if (dev->mixer) {
         mixer_close(dev->mixer);
         dev->mixer = NULL;
         ALOGD("%s: Mixer closed", __func__);
     }
 
     /* Free device structure */
     free(dev);
     g_cs35l41_device = NULL;
 
     ALOGI("%s: Amplifier device closed", __func__);
     return 0;
 }
 
 /**
  * Open amplifier module
  *
  * @param module Hardware module
  * @param name Module name (must be AMPLIFIER_HARDWARE_INTERFACE)
  * @param device Output device handle
  * @return 0 on success, negative error code on failure
  */
 static int cs35l41_amp_module_open(const hw_module_t* module,
                                    const char* name,
                                    hw_device_t** device) {
     cs35l41_amp_device_t* dev;
 
     ALOGI("%s: Opening CS35L41 amplifier module", __func__);
 
     /* Validate interface name */
     if (strcmp(name, AMPLIFIER_HARDWARE_INTERFACE) != 0) {
         ALOGE("%s: Invalid interface name: %s", __func__, name);
         return -EINVAL;
     }
 
     /* Check if already opened */
     if (g_cs35l41_device) {
         ALOGE("%s: Device already opened", __func__);
         return -EBUSY;
     }
 
     /* Allocate device structure */
     dev = calloc(1, sizeof(cs35l41_amp_device_t));
     if (!dev) {
         ALOGE("%s: Failed to allocate device structure", __func__);
         return -ENOMEM;
     }
 
     /* Initialize common device structure */
     dev->amp_dev.common.tag = HARDWARE_DEVICE_TAG;
     dev->amp_dev.common.module = (hw_module_t*)module;
     dev->amp_dev.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
     dev->amp_dev.common.close = cs35l41_amp_dev_close;
 
     /* Set amplifier HAL interface functions */
     dev->amp_dev.set_input_devices = NULL;
     dev->amp_dev.set_output_devices = cs35l41_amp_set_output_devices;
     dev->amp_dev.enable_input_devices = NULL;
     dev->amp_dev.enable_output_devices = cs35l41_amp_enable_output_devices;
     dev->amp_dev.input_stream_standby = NULL;
     dev->amp_dev.output_stream_standby = NULL;
     dev->amp_dev.set_mode = NULL;
     dev->amp_dev.input_stream_start = NULL;
     dev->amp_dev.output_stream_start = NULL;
     dev->amp_dev.set_parameters = NULL;
     dev->amp_dev.in_set_parameters = NULL;
     dev->amp_dev.out_set_parameters = NULL;
     dev->amp_dev.set_feedback = NULL;
     dev->amp_dev.calibrate = cs35l41_amp_calibrate;
 
     /* Initialize device state */
     dev->mixer = NULL;
     dev->state = AMP_STATE_UNINITIALIZED;
     dev->amplifiers_configured = false;
     dev->last_devices = 0;
     dev->last_enable_state = false;
     memset(&dev->last_operation_time, 0, sizeof(dev->last_operation_time));
     dev->dsp_error_state = false;
     dev->dsp_error_count = 0;
     dev->dsp_health_cached = true; /* Initially assume healthy */
     memset(&dev->dsp_health_check_time, 0, sizeof(dev->dsp_health_check_time));
 
     /* Set global device handle */
     g_cs35l41_device = dev;
 
     /* Return device handle */
     *device = (hw_device_t*)dev;
 
     ALOGI("%s: CS35L41 amplifier module opened successfully", __func__);
     return 0;
 }
 
 /* HAL module methods */
 static struct hw_module_methods_t cs35l41_module_methods = {
     .open = cs35l41_amp_module_open,
 };
 
 /* HAL module info */
 amplifier_module_t HAL_MODULE_INFO_SYM = {
     .common = {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = AMPLIFIER_MODULE_API_VERSION_0_1,
         .hal_api_version = HARDWARE_HAL_API_VERSION,
         .id = AMPLIFIER_HARDWARE_MODULE_ID,
         .name = "Xiaomi CS35L41 Quad Amplifier HAL",
         .author = "Harshit Jain",
         .methods = &cs35l41_module_methods,
     },
 };
 
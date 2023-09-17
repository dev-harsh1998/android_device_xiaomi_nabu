/*
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_NIDEBUG 0

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "QTI PowerHAL"
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <log/log.h>

#include "performance.h"
#include "power-common.h"
#include "utils.h"

static int display_fd;
#define SYS_DISPLAY_PWR "/sys/kernel/hbtp/display_pwr"

int set_interactive_override(int on) {
    static const char* display_on = "1";
    static const char* display_off = "0";
    char err_buf[80];
    static int init_interactive_hint = 0;
    int rc = 0;

    if (init_interactive_hint == 0) {
        // First time the display is turned off
        display_fd = TEMP_FAILURE_RETRY(open(SYS_DISPLAY_PWR, O_RDWR));
        if (display_fd < 0) {
            strerror_r(errno, err_buf, sizeof(err_buf));
            ALOGE("Error opening %s: %s\n", SYS_DISPLAY_PWR, err_buf);
        } else
            init_interactive_hint = 1;
    } else if (!on) {
        /* Display off */
        rc = TEMP_FAILURE_RETRY(write(display_fd, display_off, strlen(display_off)));
        if (rc < 0) {
            strerror_r(errno, err_buf, sizeof(err_buf));
            ALOGE("Error writing %s to  %s: %s\n", display_off, SYS_DISPLAY_PWR, err_buf);
        }
    } else {
        /* Display on */
        rc = TEMP_FAILURE_RETRY(write(display_fd, display_on, strlen(display_on)));
        if (rc < 0) {
            strerror_r(errno, err_buf, sizeof(err_buf));
            ALOGE("Error writing %s to  %s: %s\n", display_on, SYS_DISPLAY_PWR, err_buf);
        }
    }
    return HINT_HANDLED;
}

int power_hint_override(power_hint_t hint, void* data) {
    int ret_val = HINT_NONE;
    switch (hint) {
        case POWER_HINT_INTERACTION: {
            int resources[] = {MIN_FREQ_LITTLE_CORE_0, 0x514};
            int duration = 100;
            interaction(duration, ARRAY_SIZE(resources), resources);
            ret_val = HINT_HANDLED;
        }
        default:
            break;
    }
    return ret_val;
}

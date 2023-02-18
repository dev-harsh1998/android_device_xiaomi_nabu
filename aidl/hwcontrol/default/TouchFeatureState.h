//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include <sys/ioctl.h>
#include <android-base/unique_fd.h>

// TouchFeature device declarations
#define TOUCH_FEATURE_DEVICE "/dev/xiaomi-touch"
#define TOUCH_FEATURE_MAGIC 'T'
#define TOUCH_FEATURE_SETMODE 0
#define TOUCH_FEATURE_IOCTL_SETMODE _IO(TOUCH_FEATURE_MAGIC, TOUCH_FEATURE_SETMODE)

// TouchFeature device modes that we wish to use. Taken from kernel driver's xiomi_touch.h
#define TOUCH_FEATURE_TAP2WAKE 14
#define TOUCH_FEATURE_STYLUS 20

// Use android's unique_fd to ensure that the file descriptor is closed when it goes out of scope.
using file_fd = android::base::unique_fd;

// SetState for touch feature
void setTouchFeatureState(int feature, int state);
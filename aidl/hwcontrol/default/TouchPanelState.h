//
// Copyright (C) 2024 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include <string>

#define SYSFS_TOUCHPANEL_PARENT_PATH "/sys/touchpanel/"

// TouchFeature device modes that we wish to use. Taken from kernel driver's xiomi_touch.h
// DATE: 22 September 2024
// keeping these same for compatibility with the java extension
// TODO: REMOVE THESE AND BRUSH JAVA EXTENSION
#define TOUCH_FEATURE_TAP2WAKE 14
#define TOUCH_FEATURE_STYLUS 20

// SetState for touch feature
void setTouchPanelState(int feature, int state);
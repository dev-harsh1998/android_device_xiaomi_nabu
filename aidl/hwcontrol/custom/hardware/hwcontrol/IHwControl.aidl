//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
package custom.hardware.hwcontrol;

import custom.hardware.hwcontrol.HwType;

interface IHwControl {
    int getHwState(in HwType hwType);
    oneway void setHwState(in HwType hwType, int state);
}
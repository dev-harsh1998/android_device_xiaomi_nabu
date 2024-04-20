//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
package custom.hardware.hwcontrol;

@VintfStability
@Backing(type="int")
enum HwType {
    KEYBOARD,
    STYLUS,
    TAP2WAKE,
}

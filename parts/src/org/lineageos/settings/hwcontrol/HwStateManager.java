//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//

package org.lineageos.settings.hwcontrol;

import custom.hardware.hwcontrol.IHwControl;
import android.os.ServiceManager;
import android.util.Log;

import java.io.IOException;

public class HwStateManager {

    private static final boolean DEBUG = false;
    private static final String TAG = "HwStateManager";
    private static final String IHWCONTROL_AIDL_INTERFACE = "custom.hardware.hwcontrol.IHwControl/default";
    private static IHwControl mHwControl = null;

    // Private method to get the IHwControl AIDL interface.
    private static void getHwControl() throws IOException {
        if (mHwControl == null) {
            try {
                mHwControl = IHwControl.Stub.asInterface(
                        ServiceManager.waitForDeclaredService(IHWCONTROL_AIDL_INTERFACE));
            } catch (Exception e) {
                if (DEBUG) Log.e(TAG, "Failed to getHwControl()", e);
            }
            if (mHwControl == null) {
                if (DEBUG) Log.e(TAG, "IHwControl AIDL interface not available!");
            }
        }
    }

    // private method to set the state of a hardware control from other classes.
    private static void setHwControl(int hw, int state) {
        if (mHwControl == null) {
            if (DEBUG) Log.e(TAG, "IHwControl AIDL interface not initialized!, Was BootResetState() called?");
        } else {
            try {
                mHwControl.setHwState(hw, state);
            } catch (Exception e) {
                if (DEBUG) Log.e(TAG, "Failed to setHw", e);
            }
        }
    }

    // Public method to attempt and set the state of a hardware control from other classes.
    public static void HwState(int hw, int state) {
        if (mHwControl == null) {
            try {
                if (DEBUG) Log.i(TAG, "IHwControl AIDL interface not initialized!, Attempting to getHwControl()");
                getHwControl();
            } catch (IOException e) {
                if (DEBUG) Log.e(TAG, "Failed to getHwControl()", e);
            }
        }
        if (mHwControl != null) {
            setHwControl(hw, state);
        }
    }

    // public method to just initialize the IHwControl AIDL interface after boot reset.
    public static void BootResetState() {
        try {
            getHwControl();
        } catch (IOException e) {
            if (DEBUG) Log.e(TAG, "Failed to getHwControl()", e);
        }
    }
}

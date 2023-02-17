package org.lineageos.settings.hwcontrol;

import custom.hardware.hwcontrol.IHwControl;
import android.os.ServiceManager;
import android.os.IBinder;
import android.util.Log;

import java.io.IOException;

public class HwStateManager {

    private static final String TAG = "HwStateManager";
    private static final String IHWCONTROL_AIDL_INTERFACE = "custom.hardware.hwcontrol.IHwControl/default";
    private static IHwControl mHwControl = null;

    // Private method to get the IHwControl AIDL interface.
    private static void getHwControl() throws IOException {
        if (mHwControl == null) {
            IBinder binder = ServiceManager.getService(IHWCONTROL_AIDL_INTERFACE);
            if (binder == null) {
                Log.e(TAG, "Getting " + IHWCONTROL_AIDL_INTERFACE + " service daemon binder failed!");
            } else {
                mHwControl = IHwControl.Stub.asInterface(binder);
                if (mHwControl == null) {
                    Log.e(TAG, "Getting IHwControl AIDL daemon interface failed!");
                } else {
                    Log.d(TAG, "Getting IHwControl AIDL interface binding success!");
                }
            }
        }
        if (mHwControl == null) {
            Log.e(TAG, "IHwControl AIDL interface not initialization failed!");
            // Throw exception here
            throw new IOException("IHwControl AIDL interface initialization failed!");
        }
    }

    // private method to set the state of a hardware control from other classes.
    private static void setHwControl(int hw, int state) {
        if (mHwControl == null) {
            Log.e(TAG, "IHwControl AIDL interface not initialized!, Was BootResetState() called?");
        } else {
            try {
                mHwControl.setHwState(hw, state);
            } catch (Exception e) {
                Log.e(TAG, "Failed to setHw", e);
            }
        }
    }

    // Public method to attempt and set the state of a hardware control from other classes.
    public static void HwState(int hw, int state) {
        if (mHwControl == null) {
            try {
                Log.i(TAG, "IHwControl AIDL interface not initialized!, Attempting to getHwControl()");
                getHwControl();
            } catch (IOException e) {
                Log.e(TAG, "Failed to getHwControl()", e);
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
            Log.e(TAG, "Failed to getHwControl()", e);
        }
    }
}

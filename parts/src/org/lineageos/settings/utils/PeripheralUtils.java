package org.lineageos.settings.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.UserHandle;
import android.util.Log;

import vendor.xiaomi.hardware.touchfeature.V1_0.ITouchFeature;

import custom.hardware.hwcontrol.IHwControl;
import custom.hardware.hwcontrol.HwType;
import android.os.ServiceManager;
import android.os.IBinder;

import static org.lineageos.settings.stylus.StylusSettingsFragment.SHARED_STYLUS;
import static org.lineageos.settings.keyboard.XiaomiKeyboardSettingsFragment.SHARED_KEYBOARD;
import static org.lineageos.settings.tap2wake.Tap2WakeSettingsFragment.SHARED_TAP2WAKE;

public class PeripheralUtils {
    private static final String TAG = "PeripheralUtils";
    private static final boolean DEBUG = false;
    private static final String IHWCONTROL_AIDL_INTERFACE = "custom.hardware.hwcontrol.IHwControl/default";
    private static IHwControl mHwControl;
    private static ITouchFeature mTouchFeature;
    private static SharedPreferences stylus;
    private static SharedPreferences keyboard;
    private static SharedPreferences tap2wake;

    public static void BootResetState(Context context) {
        if (DEBUG) Log.d(TAG, "Starting service");
        // Initialize stylus and keyboard shared preferences
        stylus = context.getSharedPreferences(SHARED_STYLUS, Context.MODE_PRIVATE);
        keyboard = context.getSharedPreferences(SHARED_KEYBOARD, Context.MODE_PRIVATE);
        tap2wake = context.getSharedPreferences(SHARED_TAP2WAKE, Context.MODE_PRIVATE);

        // try to get the touchfeature service.
        try {
            mTouchFeature = ITouchFeature.getService();
        } catch (Exception e) {
            Log.e(TAG, "Failed to get touchfeature service", e);
        }

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

        // Sync all peripherals
        SyncAll();
    }

    // Enable stylus based on shared preference.
    private static void SyncStylus() {
        if (DEBUG) Log.d(TAG, "Enabling stylus");
        if (mTouchFeature != null) {
            try {
                mTouchFeature.setTouchMode(20, stylus.getInt(SHARED_STYLUS, 0));
            } catch (Exception e) {
                Log.e(TAG, "Failed to enable stylus", e);
            }
        }
    }

    // Enable keyboard based on shared preference.
    private static void SyncKeyboard() {
        if (DEBUG) Log.d(TAG, "Enabling keyboard");
        if (mHwControl != null) {
            try {
                mHwControl.setHwState(HwType.KEYBOARD, keyboard.getInt(SHARED_KEYBOARD, 0));
            } catch (Exception e) {
                Log.e(TAG, "Failed to enable keyboard", e);
            }
        } else {
            Log.d(TAG, "mHwControl is null");
        }
    }

    // Enable tap2wake based on shared preference.
    public static void SyncTap2Wake() {
        if (DEBUG) Log.d(TAG, "Enabling tap2wake");
        if (mTouchFeature != null) {
            try {
                mTouchFeature.setTouchMode(14, tap2wake.getInt(SHARED_TAP2WAKE, 0));
            } catch (Exception e) {
                Log.e(TAG, "Failed to enable tap2wake", e);
            }
        }
    }

    // Sync all peripherals
    private static void SyncAll() {
        SyncStylus();
        SyncKeyboard();
        SyncTap2Wake();
    }
}

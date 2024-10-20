//
// Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
//
// SPDX-License-Identifier: Apache-2.0
//
package com.harshit.nabuextensions;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.UserHandle;
import android.os.SystemProperties;
import android.util.Log;

import com.harshit.nabuextensions.HwStateManager;
import custom.hardware.hwcontrol.HwType;

import static com.harshit.nabuextensions.stylus.StylusSettingsFragment.SHARED_STYLUS;
import static com.harshit.nabuextensions.stylus.StylusSettingsFragment.SHARED_STYLUS_GEN;
import static com.harshit.nabuextensions.stylus.StylusSettingsFragment.SHARED_STYLUS_GEN_PROP;
import static com.harshit.nabuextensions.keyboard.XiaomiKeyboardSettingsFragment.SHARED_KEYBOARD;
import static com.harshit.nabuextensions.tap2wake.Tap2WakeSettingsFragment.SHARED_TAP2WAKE;

public class PeripheralUtils {
    private static final String TAG = "PeripheralUtils";
    private static final boolean DEBUG = false;
    private static HwStateManager mHwStateManager;
    private static SharedPreferences stylus;
    private static SharedPreferences keyboard;
    private static SharedPreferences tap2wake;
    private static SharedPreferences stylusGen;

    public static void BootResetState(Context context) {
        if (DEBUG)
            Log.d(TAG, "Starting service");
        // Initialize stylus and keyboard shared preferences
        stylus = context.getSharedPreferences(SHARED_STYLUS, Context.MODE_PRIVATE);
        keyboard = context.getSharedPreferences(SHARED_KEYBOARD, Context.MODE_PRIVATE);
        tap2wake = context.getSharedPreferences(SHARED_TAP2WAKE, Context.MODE_PRIVATE);
        stylusGen = context.getSharedPreferences(SHARED_STYLUS_GEN, Context.MODE_PRIVATE);

        // Initialize HwStateManager
        HwStateManager.BootResetState();

        // Sync all peripherals
        SyncAll();
    }

    // Enable stylus based on shared preference.
    private static void SyncStylus() {
        if (DEBUG)
            Log.d(TAG, "Enabling stylus");
        mHwStateManager.HwState(HwType.STYLUS, stylus.getInt(SHARED_STYLUS, 0));
        SystemProperties.set(SHARED_STYLUS_GEN_PROP, stylusGen.getString(SHARED_STYLUS_GEN, "1"));
    }

    // Enable keyboard based on shared preference.
    private static void SyncKeyboard() {
        if (DEBUG)
            Log.d(TAG, "Enabling keyboard");
        mHwStateManager.HwState(HwType.KEYBOARD, keyboard.getInt(SHARED_KEYBOARD, 0));
    }

    // Enable tap2wake based on shared preference.
    public static void SyncTap2Wake() {
        if (DEBUG)
            Log.d(TAG, "Enabling tap2wake");
        mHwStateManager.HwState(HwType.TAP2WAKE, tap2wake.getInt(SHARED_TAP2WAKE, 0));
    }

    // Sync all peripherals
    private static void SyncAll() {
        SyncStylus();
        SyncKeyboard();
        SyncTap2Wake();
    }
}
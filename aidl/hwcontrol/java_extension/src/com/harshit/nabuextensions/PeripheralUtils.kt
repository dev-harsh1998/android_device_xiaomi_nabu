/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions

import android.content.Context
import android.os.SystemProperties
import android.util.Log
import custom.hardware.hwcontrol.HwType

/**
 * Utility class for managing peripheral devices (stylus, keyboard, tap2wake).
 * Handles synchronization of hardware states on boot and preference changes.
 */
object PeripheralUtils {
    
    private val TAG = PeripheralUtils::class.java.simpleName
    
    /**
     * Initialize all peripherals on boot completion.
     * Restores saved preferences and applies hardware states.
     */
    fun bootResetState(context: Context) {
        Log.i(TAG, "Initializing peripheral devices")
        
        // Initialize hardware control manager
        HwStateManager.initialize()
        
        // Restore all peripheral states from preferences
        syncAll(context)
        
        Log.i(TAG, "Peripheral initialization complete")
    }
    
    /**
     * Synchronize stylus hardware state with saved preferences
     */
    fun syncStylus(context: Context) {
        val enabled = PreferenceManager.getStylusEnabled(context)
        val generation = PreferenceManager.getStylusGeneration(context)
        
        Log.d(TAG, "Syncing stylus: enabled=$enabled, generation=$generation")
        
        HwStateManager.setHwState(HwType.STYLUS, enabled)
        SystemProperties.set(PreferenceManager.KEY_STYLUS_GEN_PROP, generation)
    }
    
    /**
     * Synchronize keyboard hardware state with saved preferences
     */
    fun syncKeyboard(context: Context) {
        val enabled = PreferenceManager.getKeyboardEnabled(context)
        
        Log.d(TAG, "Syncing keyboard: enabled=$enabled")
        
        HwStateManager.setHwState(HwType.KEYBOARD, enabled)
    }
    
    /**
     * Synchronize tap2wake hardware state with saved preferences
     */
    fun syncTap2Wake(context: Context) {
        val enabled = PreferenceManager.getTap2WakeEnabled(context)
        
        Log.d(TAG, "Syncing tap2wake: enabled=$enabled")
        
        HwStateManager.setHwState(HwType.TAP2WAKE, enabled)
    }
    
    /**
     * Synchronize game mode hardware state with saved preferences
     */
    fun syncGameMode(context: Context) {
        val enabled = PreferenceManager.getGameModeEnabled(context)
        
        Log.d(TAG, "Syncing game mode: enabled=$enabled")
        
        HwStateManager.setHwState(HwType.GAMEMODE, enabled)
    }
    
    /**
     * Synchronize all peripheral hardware states
     */
    private fun syncAll(context: Context) {
        syncStylus(context)
        syncKeyboard(context)
        syncTap2Wake(context)
        syncGameMode(context)
    }
}


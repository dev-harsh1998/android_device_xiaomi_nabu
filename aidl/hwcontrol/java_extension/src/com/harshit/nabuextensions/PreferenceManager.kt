/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions

import android.content.Context
import android.content.SharedPreferences

/**
 * Centralized preference manager for hardware control settings.
 * Provides type-safe access to SharedPreferences with consistent naming.
 */
object PreferenceManager {
    
    // Preference file names
    private const val PREF_STYLUS = "shared_stylus"
    private const val PREF_STYLUS_GEN = "shared_stylus_gen"
    private const val PREF_KEYBOARD = "shared_keyboard"
    private const val PREF_TAP2WAKE = "shared_tap2wake"
    private const val PREF_GAMEMODE = "shared_gamemode"
    
    // Preference keys
    const val KEY_STYLUS = "shared_stylus"
    const val KEY_STYLUS_GEN = "shared_stylus_gen"
    const val KEY_STYLUS_GEN_PROP = "persist.mi_pen.gen"
    const val KEY_KEYBOARD = "shared_keyboard"
    const val KEY_TAP2WAKE = "shared_tap2wake"
    const val KEY_GAMEMODE = "shared_gamemode"
    
    // Default values
    private const val DEFAULT_DISABLED = 0
    private const val DEFAULT_STYLUS_GEN = "1"
    
    /**
     * Get stylus enable/disable state
     */
    fun getStylusEnabled(context: Context): Int {
        return getPreferences(context, PREF_STYLUS).getInt(KEY_STYLUS, DEFAULT_DISABLED)
    }
    
    /**
     * Set stylus enable/disable state
     */
    fun setStylusEnabled(context: Context, enabled: Int) {
        getPreferences(context, PREF_STYLUS).edit().putInt(KEY_STYLUS, enabled).apply()
    }
    
    /**
     * Get stylus generation
     */
    fun getStylusGeneration(context: Context): String {
        return getPreferences(context, PREF_STYLUS_GEN).getString(KEY_STYLUS_GEN, DEFAULT_STYLUS_GEN) ?: DEFAULT_STYLUS_GEN
    }
    
    /**
     * Set stylus generation
     */
    fun setStylusGeneration(context: Context, generation: String) {
        getPreferences(context, PREF_STYLUS_GEN).edit().putString(KEY_STYLUS_GEN, generation).apply()
    }
    
    /**
     * Get keyboard enable/disable state
     */
    fun getKeyboardEnabled(context: Context): Int {
        return getPreferences(context, PREF_KEYBOARD).getInt(KEY_KEYBOARD, DEFAULT_DISABLED)
    }
    
    /**
     * Set keyboard enable/disable state
     */
    fun setKeyboardEnabled(context: Context, enabled: Int) {
        getPreferences(context, PREF_KEYBOARD).edit().putInt(KEY_KEYBOARD, enabled).apply()
    }
    
    /**
     * Get tap2wake enable/disable state
     */
    fun getTap2WakeEnabled(context: Context): Int {
        return getPreferences(context, PREF_TAP2WAKE).getInt(KEY_TAP2WAKE, DEFAULT_DISABLED)
    }
    
    /**
     * Set tap2wake enable/disable state
     */
    fun setTap2WakeEnabled(context: Context, enabled: Int) {
        getPreferences(context, PREF_TAP2WAKE).edit().putInt(KEY_TAP2WAKE, enabled).apply()
    }
    
    /**
     * Get game mode enable/disable state
     */
    fun getGameModeEnabled(context: Context): Int {
        return getPreferences(context, PREF_GAMEMODE).getInt(KEY_GAMEMODE, DEFAULT_DISABLED)
    }
    
    /**
     * Set game mode enable/disable state
     */
    fun setGameModeEnabled(context: Context, enabled: Int) {
        getPreferences(context, PREF_GAMEMODE).edit().putInt(KEY_GAMEMODE, enabled).apply()
    }
    
    /**
     * Helper method to get SharedPreferences instance.
     * Uses device-protected storage to support Direct Boot mode.
     */
    private fun getPreferences(context: Context, name: String): SharedPreferences {
        val storageContext = if (context.isDeviceProtectedStorage) {
            context
        } else {
            context.createDeviceProtectedStorageContext()
        }
        return storageContext.getSharedPreferences(name, Context.MODE_PRIVATE)
    }
}


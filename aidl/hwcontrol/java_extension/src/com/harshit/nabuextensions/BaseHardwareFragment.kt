/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions

import android.os.Bundle
import android.util.Log
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.SwitchPreferenceCompat

/**
 * Abstract base class for hardware control preference fragments.
 * Eliminates code duplication by providing common functionality for
 * switch-based hardware control settings.
 */
abstract class BaseHardwareFragment : PreferenceFragmentCompat(), Preference.OnPreferenceChangeListener {
    
    private val logTag: String get() = this::class.java.simpleName
    
    /**
     * Get the XML resource ID for preferences
     */
    protected abstract fun getPreferenceResource(): Int
    
    /**
     * Get the preference key for the main switch
     */
    protected abstract fun getSwitchKey(): String
    
    /**
     * Get additional switches (for features like stylus generation)
     * Override if fragment has multiple switches
     */
    protected open fun getAdditionalSwitches(): Map<String, SwitchPreferenceCompat> = emptyMap()
    
    /**
     * Handle preference change
     * @param key The preference key
     * @param newValue The new value
     */
    protected abstract fun onSwitchChanged(key: String, newValue: Boolean)
    
    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        addPreferencesFromResource(getPreferenceResource())
        setupPreferences()
    }
    
    /**
     * Setup preferences with listeners
     */
    private fun setupPreferences() {
        // Setup main switch
        findPreference<SwitchPreferenceCompat>(getSwitchKey())?.apply {
            isEnabled = true
            onPreferenceChangeListener = this@BaseHardwareFragment
        }
        
        // Setup additional switches if any
        getAdditionalSwitches().forEach { (_, preference) ->
            preference.apply {
                isEnabled = true
                onPreferenceChangeListener = this@BaseHardwareFragment
            }
        }
    }
    
    override fun onPreferenceChange(preference: Preference, newValue: Any?): Boolean {
        return try {
            val boolValue = newValue as? Boolean ?: false
            onSwitchChanged(preference.key, boolValue)
            true
        } catch (e: Exception) {
            Log.e(logTag, "Error handling preference change for ${preference.key}", e)
            false
        }
    }
    
    /**
     * Helper to convert boolean to int (0/1)
     */
    protected fun Boolean.toInt(): Int = if (this) 1 else 0
}


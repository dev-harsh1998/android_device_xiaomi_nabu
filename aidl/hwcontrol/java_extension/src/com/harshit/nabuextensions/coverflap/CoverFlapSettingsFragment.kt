/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions.coverflap

import android.os.Bundle
import android.provider.Settings
import android.util.Log
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.SwitchPreferenceCompat
import com.harshit.nabuextensions.R

/**
 * Settings fragment for Cover Flap configuration.
 * Controls the lid_behavior setting which determines how the device
 * responds when the cover flap is opened/closed.
 */
class CoverFlapSettingsFragment : PreferenceFragmentCompat(), 
    Preference.OnPreferenceChangeListener {
    
    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        addPreferencesFromResource(R.xml.coverflap_settings)
        
        findPreference<SwitchPreferenceCompat>(COVERFLAP_KEY)?.apply {
            isChecked = getCoverFlapEnabled()
            onPreferenceChangeListener = this@CoverFlapSettingsFragment
        }
    }
    
    override fun onPreferenceChange(preference: Preference, newValue: Any?): Boolean {
        return try {
            val enabled = newValue as? Boolean ?: false
            setCoverFlapEnabled(enabled)
            Log.d(TAG, "Cover flap lid behavior set to: $enabled")
            true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to set cover flap state", e)
            false
        }
    }
    
    private fun getCoverFlapEnabled(): Boolean {
        return Settings.Global.getInt(
            requireContext().contentResolver,
            SETTING_LID_BEHAVIOR,
            DEFAULT_LID_BEHAVIOR
        ) == 1
    }
    
    private fun setCoverFlapEnabled(enabled: Boolean) {
        Settings.Global.putInt(
            requireContext().contentResolver,
            SETTING_LID_BEHAVIOR,
            if (enabled) 1 else 0
        )
    }
    
    companion object {
        private const val TAG = "CoverFlapSettings"
        private const val COVERFLAP_KEY = "coverflap_switch_key"
        private const val SETTING_LID_BEHAVIOR = "lid_behavior"
        private const val DEFAULT_LID_BEHAVIOR = 1
    }
}

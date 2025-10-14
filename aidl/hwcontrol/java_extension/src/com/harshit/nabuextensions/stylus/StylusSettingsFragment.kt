/*
 * Copyright (C) 2018,2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.harshit.nabuextensions.stylus

import android.os.SystemProperties
import androidx.preference.SwitchPreferenceCompat
import com.harshit.nabuextensions.BaseHardwareFragment
import com.harshit.nabuextensions.HwStateManager
import com.harshit.nabuextensions.PreferenceManager
import com.harshit.nabuextensions.R
import custom.hardware.hwcontrol.HwType

class StylusSettingsFragment : BaseHardwareFragment() {
    
    private val stylusGenKey: String
        get() = findPreference<SwitchPreferenceCompat>(STYLUS_GEN_KEY)?.key ?: ""
    
    override fun getPreferenceResource(): Int = R.xml.stylus_settings
    
    override fun getSwitchKey(): String = STYLUS_KEY
    
    override fun getAdditionalSwitches(): Map<String, SwitchPreferenceCompat> {
        val genSwitch = findPreference<SwitchPreferenceCompat>(STYLUS_GEN_KEY)
        return if (genSwitch != null) {
            mapOf(STYLUS_GEN_KEY to genSwitch)
        } else {
            emptyMap()
        }
    }
    
    override fun onSwitchChanged(key: String, newValue: Boolean) {
        when (key) {
            STYLUS_KEY -> enableStylus(newValue.toInt())
            STYLUS_GEN_KEY -> setStylusGeneration(newValue)
        }
    }
    
    private fun enableStylus(status: Int) {
        HwStateManager.setHwState(HwType.STYLUS, status)
        PreferenceManager.setStylusEnabled(requireContext(), status)
    }
    
    private fun setStylusGeneration(gen2: Boolean) {
        val generation = if (gen2) "2" else "1"
        SystemProperties.set(PreferenceManager.KEY_STYLUS_GEN_PROP, generation)
        PreferenceManager.setStylusGeneration(requireContext(), generation)
    }
    
    companion object {
        private const val STYLUS_KEY = "stylus_switch_key"
        private const val STYLUS_GEN_KEY = "stylus_gen_switch_key"
    }
}


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
package com.harshit.nabuextensions.tap2wake

import com.harshit.nabuextensions.BaseHardwareFragment
import com.harshit.nabuextensions.HwStateManager
import com.harshit.nabuextensions.PreferenceManager
import com.harshit.nabuextensions.R
import custom.hardware.hwcontrol.HwType

class Tap2WakeSettingsFragment : BaseHardwareFragment() {
    
    override fun getPreferenceResource(): Int = R.xml.tap2wake_settings
    
    override fun getSwitchKey(): String = TAP2WAKE_KEY
    
    override fun onSwitchChanged(key: String, newValue: Boolean) {
        if (key == TAP2WAKE_KEY) {
            enableTap2Wake(newValue.toInt())
        }
    }
    
    private fun enableTap2Wake(status: Int) {
        HwStateManager.setHwState(HwType.TAP2WAKE, status)
        PreferenceManager.setTap2WakeEnabled(requireContext(), status)
    }
    
    companion object {
        private const val TAP2WAKE_KEY = "tap2wake_switch_key"
    }
}


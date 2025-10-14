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
package com.harshit.nabuextensions.keyboard

import com.harshit.nabuextensions.BaseHardwareFragment
import com.harshit.nabuextensions.HwStateManager
import com.harshit.nabuextensions.PreferenceManager
import com.harshit.nabuextensions.R
import custom.hardware.hwcontrol.HwType

class XiaomiKeyboardSettingsFragment : BaseHardwareFragment() {
    
    override fun getPreferenceResource(): Int = R.xml.keyboard_settings
    
    override fun getSwitchKey(): String = KEYBOARD_KEY
    
    override fun onSwitchChanged(key: String, newValue: Boolean) {
        if (key == KEYBOARD_KEY) {
            enableKeyboard(newValue.toInt())
        }
    }
    
    private fun enableKeyboard(status: Int) {
        HwStateManager.setHwState(HwType.KEYBOARD, status)
        PreferenceManager.setKeyboardEnabled(requireContext(), status)
    }
    
    companion object {
        private const val KEYBOARD_KEY = "keyboard_switch_key"
    }
}


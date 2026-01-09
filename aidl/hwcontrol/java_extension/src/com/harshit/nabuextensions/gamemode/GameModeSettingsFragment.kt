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
package com.harshit.nabuextensions.gamemode

import android.widget.Toast
import com.harshit.nabuextensions.BaseHardwareFragment
import com.harshit.nabuextensions.HwStateManager
import com.harshit.nabuextensions.PreferenceManager
import com.harshit.nabuextensions.R
import custom.hardware.hwcontrol.HwType

class GameModeSettingsFragment : BaseHardwareFragment() {
    
    override fun getPreferenceResource(): Int = R.xml.game_mode_settings
    
    override fun getSwitchKey(): String = GAMEMODE_KEY
    
    override fun onSwitchChanged(key: String, newValue: Boolean) {
        if (key == GAMEMODE_KEY) {
            if (newValue) {
                Toast.makeText(
                    requireContext(),
                    R.string.game_mode_warning_gen2_pen,
                    Toast.LENGTH_LONG
                ).show()
            }
            enableGameMode(newValue.toInt())
        }
    }
    
    private fun enableGameMode(status: Int) {
        HwStateManager.setHwState(HwType.GAMEMODE, status)
        PreferenceManager.setGameModeEnabled(requireContext(), status)
    }
    
    companion object {
        private const val GAMEMODE_KEY = "game_mode_switch_key"
    }
}


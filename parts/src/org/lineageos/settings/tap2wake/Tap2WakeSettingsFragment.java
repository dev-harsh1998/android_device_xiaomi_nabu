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

package org.lineageos.settings.tap2wake;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.Switch;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceFragment;
import androidx.preference.SwitchPreference;
import com.android.settingslib.widget.MainSwitchPreference;

import org.lineageos.settings.hwcontrol.HwStateManager;
import custom.hardware.hwcontrol.HwType;

import org.lineageos.settings.R;

public class Tap2WakeSettingsFragment extends PreferenceFragment implements
OnPreferenceChangeListener {

     private static final String TAP2WAKE_KEY = "tap2wake_switch_key";
     public static final String SHARED_TAP2WAKE = "shared_tap2wake";

     private SwitchPreference mTap2WakePreference;

     @Override
     public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
         addPreferencesFromResource(R.xml.tap2wake_settings);

         mTap2WakePreference = (SwitchPreference) findPreference(TAP2WAKE_KEY);
         mTap2WakePreference.setEnabled(true);
         mTap2WakePreference.setOnPreferenceChangeListener(this);
     }

     @Override
     public boolean onPreferenceChange(Preference preference, Object newValue) {
         if (TAP2WAKE_KEY.equals(preference.getKey())) {
            enableTap2Wake((Boolean) newValue ? 1 : 0);
         }
         return true;
     }

     private void enableTap2Wake(int status) {
         try {
            HwStateManager.HwState(HwType.TAP2WAKE, status);
            SharedPreferences preferences = getActivity().getSharedPreferences(SHARED_TAP2WAKE, Context.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();
            editor.putInt(SHARED_TAP2WAKE, status);
            editor.commit();
         } catch (Exception e) {
         }
     }
}
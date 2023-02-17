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

package org.lineageos.settings.keyboard;

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

import custom.hardware.hwcontrol.HwType;
import org.lineageos.settings.hwcontrol.HwStateManager;

import org.lineageos.settings.R;
import org.lineageos.settings.utils.FileUtils;

public class XiaomiKeyboardSettingsFragment extends PreferenceFragment implements
        OnPreferenceChangeListener {

    private static final String KEYBOARD_KEY = "keyboard_switch_key";
    public static final String SHARED_KEYBOARD = "shared_keyboard";

    private SwitchPreference mKeyboardPreference;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.keyboard_settings);
        mKeyboardPreference = (SwitchPreference) findPreference(KEYBOARD_KEY);

        mKeyboardPreference.setEnabled(true);
        mKeyboardPreference.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {        
        if (KEYBOARD_KEY.equals(preference.getKey())) {
            enableKeyboard((Boolean) newValue ? 1 : 0);
        }
        return true;
    }

    private void enableKeyboard(int status) {
        try {
            HwStateManager.HwState(HwType.KEYBOARD, status);
            SharedPreferences preferences = getActivity().getSharedPreferences(SHARED_KEYBOARD, Context.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();
            editor.putInt(SHARED_KEYBOARD, status);
            editor.commit();
        } catch (Exception e) {
        }
    }
}

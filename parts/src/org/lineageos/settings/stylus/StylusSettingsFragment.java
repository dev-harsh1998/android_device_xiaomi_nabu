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

package org.lineageos.settings.stylus;

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

import vendor.xiaomi.hardware.touchfeature.V1_0.ITouchFeature;

import org.lineageos.settings.R;

public class StylusSettingsFragment extends PreferenceFragment implements
        OnPreferenceChangeListener {

    private static final String STYLUS_KEY = "stylus_switch_key";
    public static final String SHARED_STYLUS = "shared_stylus";

    private SwitchPreference mStylusPreference;
    private ITouchFeature mTouchFeature;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.stylus_settings);

        try {
            mTouchFeature = ITouchFeature.getService();
        } catch (Exception e) {
        }
        mStylusPreference = (SwitchPreference) findPreference(STYLUS_KEY);
        mStylusPreference.setEnabled(true);
        mStylusPreference.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (STYLUS_KEY.equals(preference.getKey())) {
            enableStylus((Boolean) newValue ? 1 : 0);
        }
        return true;
    }

    private void enableStylus(int status) {
        if (mTouchFeature == null) return;
        try {
            mTouchFeature.setTouchMode(20, status);
            SharedPreferences preferences = getActivity().getSharedPreferences(SHARED_STYLUS, Context.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();
            editor.putInt(SHARED_STYLUS, status);
            editor.commit();
        }
        catch (Exception e) {
        }
    }
}

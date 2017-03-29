/*
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
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

package com.cyanogenmod.cmactions;

import android.os.Bundle;
import android.support.v14.preference.PreferenceFragment;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.preference.Preference;
import android.provider.Settings;

public class GesturePreferenceFragment extends PreferenceFragment implements
        Preference.OnPreferenceChangeListener {

    private SwitchPreference mAmbientDisplayPreference;
    private SwitchPreference mPocketPreference;
    private SwitchPreference mHandwavePreference;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.gesture_panel);
        boolean dozeEnabled = isDozeEnabled();
        mAmbientDisplayPreference =
                (SwitchPreference) findPreference(Constants.PREF_AMBIENT_DISPLAY_KEY);
        mAmbientDisplayPreference.setChecked(dozeEnabled);
        mAmbientDisplayPreference.setOnPreferenceChangeListener(this);
        mPocketPreference = (SwitchPreference) findPreference(Constants.PREF_GESTURE_POCKET_KEY);
        mPocketPreference.setEnabled(dozeEnabled);
        mPocketPreference.setOnPreferenceChangeListener(this);
        mHandwavePreference =
                (SwitchPreference) findPreference(Constants.PREF_GESTURE_HAND_WAVE_KEY);
        mHandwavePreference.setEnabled(dozeEnabled);
    }

    private boolean enableDoze(boolean enable) {
        boolean result = Settings.Secure.putInt(getActivity().getContentResolver(),
                Settings.Secure.DOZE_ENABLED, enable ? 1 : 0);
        CMActionsReceiver.notifyChanged(getActivity());
        return result;
    }

    private boolean isDozeEnabled() {
        return Settings.Secure.getInt(getActivity().getContentResolver(),
                Settings.Secure.DOZE_ENABLED, 1) != 0;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference.equals(mAmbientDisplayPreference)) {
            boolean enable = (boolean) newValue;
            boolean ret = enableDoze(enable);
            if (ret) {
                mPocketPreference.setEnabled(enable);
                mHandwavePreference.setEnabled(enable);
            }
            return ret;
        }
        return false;
    }
}

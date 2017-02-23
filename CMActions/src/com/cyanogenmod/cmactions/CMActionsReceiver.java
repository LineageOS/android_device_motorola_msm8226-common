/*
 * Copyright (c) 2016 The CyanogenMod Project
 * Copyright (c) 2017 The LineageOS Project
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

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.util.Log;

import cyanogenmod.preference.RemotePreferenceUpdater;

public class CMActionsReceiver extends RemotePreferenceUpdater {

    private static final boolean DEBUG = false;
    private static final String TAG = "CMActions";

    private static final String DOZE_KEY = "doze_device_settings";

    @Override
    public void onReceive(final Context context, Intent intent) {
        super.onReceive(context, intent);
        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            if (DEBUG) Log.d(TAG, "Starting service");
            context.startService(new Intent(context, CMActionsService.class));
        }
    }

    @Override
    public String getSummary(Context context, String key) {
        if (DOZE_KEY.equals(key)) {
            boolean enabled = Settings.Secure.getInt(context.getContentResolver(),
                Settings.Secure.DOZE_ENABLED, 1) != 0;
            if (enabled) {
                return context.getString(R.string.ambient_display_summary_on);
            } else {
                return context.getString(R.string.ambient_display_summary_off);
            }
        }
        return null;
    }

    public static void notifyChanged(Context context) {
        notifyChanged(context, DOZE_KEY);
    }
}

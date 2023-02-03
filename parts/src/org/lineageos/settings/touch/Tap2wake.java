/*
 * Copyright (C) 2015-2016 The CyanogenMod Project
 *               2017,2021 The LineageOS Project
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

package org.lineageos.settings.touch;

import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings.Secure;

import vendor.xiaomi.hardware.touchfeature.V1_0.ITouchFeature;

public class Tap2wake extends Service {
    private static final String TAG = "Tap2wake";
    private Context mContext;
    private Handler mHandler;
    private CustomSettingsObserver mCustomSettingsObserver;
    private ITouchFeature mTouchFeature;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startid) {
        mContext = this;
        mHandler = new Handler(Looper.getMainLooper());
        mCustomSettingsObserver = new CustomSettingsObserver(mHandler);
        mCustomSettingsObserver.observe();
        mCustomSettingsObserver.update();
        return START_STICKY;
    }

    private class CustomSettingsObserver extends ContentObserver {
        CustomSettingsObserver(Handler handler) {
            super(handler);
        }

        void observe() {
            ContentResolver resolver = mContext.getContentResolver();
            resolver.registerContentObserver(Secure.getUriFor(Secure.DOUBLE_TAP_TO_WAKE),
                    false, this, UserHandle.USER_CURRENT);
        }

        void update() {
            try {
                mTouchFeature = ITouchFeature.getService();
                if (mTouchFeature != null) {
                    mTouchFeature.setTouchMode(14, Secure.getInt(mContext.getContentResolver(), Secure.DOUBLE_TAP_TO_WAKE, 0));
                }
            } catch (Exception e) {
                e.printStackTrace();
            } 
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (uri.equals(Secure.getUriFor(Secure.DOUBLE_TAP_TO_WAKE))) {
                update();
            }
        }
    }
}
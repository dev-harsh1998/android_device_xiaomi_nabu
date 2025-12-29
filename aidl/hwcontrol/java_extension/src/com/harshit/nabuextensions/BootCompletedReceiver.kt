/*
 * Copyright (C) 2015 The CyanogenMod Project
 *               2017-2020 The LineageOS Project
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
package com.harshit.nabuextensions

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log
import com.harshit.nabuextensions.stylus.PenChargingService

class BootCompletedReceiver : BroadcastReceiver() {

    private val tag = this::class.java.simpleName

    override fun onReceive(context: Context, intent: Intent) {
        // Use device-protected storage context for Direct Boot support
        val storageContext = if (context.isDeviceProtectedStorage) {
            context
        } else {
            context.createDeviceProtectedStorageContext()
        }

        Log.i(tag, "Boot completed, initializing hardware controls")
        PeripheralUtils.bootResetState(storageContext)

        // Start pen charging notification service
        PenChargingService.start(storageContext)
    }
}


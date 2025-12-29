/*
 * Copyright (C) 2025 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions.stylus

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.PowerManager
import android.util.Log
import com.harshit.nabuextensions.R

/**
 * Background service that monitors pen charging status and shows
 * a persistent notification when pen is attached and charging.
 *
 * The service only actively polls when the screen is on to minimize
 * battery impact.
 */
class PenChargingService : Service() {

    private val handler = Handler(Looper.getMainLooper())
    private lateinit var notificationManager: NotificationManager
    private lateinit var powerManager: PowerManager

    private var isMonitoring = false
    private var lastBatteryLevel = BATTERY_LEVEL_UNKNOWN
    private var wasCharging = false

    private val pollRunnable = object : Runnable {
        override fun run() {
            if (isMonitoring) {
                updateChargingStatus()
                handler.postDelayed(this, POLL_INTERVAL_MS)
            }
        }
    }

    private val screenReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            when (intent?.action) {
                Intent.ACTION_SCREEN_ON -> {
                    if (DEBUG) Log.d(TAG, "Received ACTION_SCREEN_ON")
                    startMonitoring()
                }
                Intent.ACTION_SCREEN_OFF -> {
                    if (DEBUG) Log.d(TAG, "Received ACTION_SCREEN_OFF")
                    stopMonitoring()
                }
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        if (DEBUG) Log.d(TAG, "onCreate() called")

        notificationManager = getSystemService(NotificationManager::class.java)
        powerManager = getSystemService(PowerManager::class.java)

        createNotificationChannel()
        registerScreenReceiver()

        // Start monitoring if screen is already on
        val screenOn = powerManager.isInteractive
        if (DEBUG) Log.d(TAG, "onCreate() screen isInteractive=$screenOn")

        if (screenOn) {
            startMonitoring()
        }

        Log.i(TAG, "PenChargingService created")
    }

    override fun onDestroy() {
        super.onDestroy()
        if (DEBUG) Log.d(TAG, "onDestroy() called")

        stopMonitoring()
        try {
            unregisterReceiver(screenReceiver)
            if (DEBUG) Log.d(TAG, "Screen receiver unregistered")
        } catch (e: IllegalArgumentException) {
            if (DEBUG) Log.w(TAG, "Screen receiver was not registered")
        }
        Log.i(TAG, "PenChargingService destroyed")
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (DEBUG) Log.d(TAG, "onStartCommand() called, flags=$flags, startId=$startId")
        return START_STICKY
    }

    private fun createNotificationChannel() {
        if (DEBUG) Log.d(TAG, "Creating notification channel: $CHANNEL_ID")

        val channel = NotificationChannel(
            CHANNEL_ID,
            getString(R.string.pen_charging_channel_name),
            NotificationManager.IMPORTANCE_LOW
        ).apply {
            description = getString(R.string.pen_charging_channel_description)
            setShowBadge(false)
            enableLights(false)
            enableVibration(false)
            setSound(null, null)
        }
        notificationManager.createNotificationChannel(channel)
    }

    private fun registerScreenReceiver() {
        if (DEBUG) Log.d(TAG, "Registering screen receiver")

        val filter = IntentFilter().apply {
            addAction(Intent.ACTION_SCREEN_ON)
            addAction(Intent.ACTION_SCREEN_OFF)
        }
        registerReceiver(screenReceiver, filter, RECEIVER_NOT_EXPORTED)
    }

    private fun startMonitoring() {
        if (DEBUG) {
            Log.d(TAG, "startMonitoring() called - isMonitoring=$isMonitoring, " +
                    "hwAvailable=${PenChargingManager.isAvailable()}")
        }

        if (!isMonitoring && PenChargingManager.isAvailable()) {
            isMonitoring = true
            handler.post(pollRunnable)
            if (DEBUG) Log.d(TAG, "Monitoring started, first poll scheduled")
        }
    }

    private fun stopMonitoring() {
        if (DEBUG) Log.d(TAG, "stopMonitoring() called - isMonitoring=$isMonitoring")

        if (isMonitoring) {
            isMonitoring = false
            handler.removeCallbacks(pollRunnable)
            hideNotification()
            wasCharging = false
            lastBatteryLevel = BATTERY_LEVEL_UNKNOWN
            if (DEBUG) Log.d(TAG, "Monitoring stopped, state reset")
        }
    }

    private fun updateChargingStatus() {
        val status = PenChargingManager.getStatus()

        if (DEBUG) {
            Log.d(TAG, "updateChargingStatus() - status=$status, " +
                    "wasCharging=$wasCharging, lastBatteryLevel=$lastBatteryLevel")
        }

        if (status == null || !status.isConnected || !status.isCharging) {
            // Pen not connected or not charging - hide notification
            if (wasCharging) {
                Log.i(TAG, "Pen disconnected or stopped charging")
                hideNotification()
                wasCharging = false
                lastBatteryLevel = BATTERY_LEVEL_UNKNOWN
            }
            return
        }

        // Pen is connected and charging
        // Show notification if: just started charging OR battery level changed
        val justStarted = !wasCharging
        val shouldUpdate = justStarted || status.batteryLevel != lastBatteryLevel

        if (DEBUG) {
            Log.d(TAG, "Pen charging: shouldUpdate=$shouldUpdate, " +
                    "justStarted=$justStarted, levelChanged=${status.batteryLevel != lastBatteryLevel}")
        }

        wasCharging = true

        if (shouldUpdate) {
            if (justStarted) {
                Log.i(TAG, "Pen connected and charging at ${status.batteryLevel}%")
            }
            lastBatteryLevel = status.batteryLevel
            showNotification(status.batteryLevel)
        }
    }

    private fun showNotification(batteryLevel: Int) {
        val title = getString(R.string.pen_charging_notification_title)
        val content = if (batteryLevel >= 0) {
            getString(R.string.pen_charging_notification_content, batteryLevel)
        } else {
            getString(R.string.pen_charging_notification_content_unknown)
        }

        if (DEBUG) Log.d(TAG, "showNotification() - title=$title, content=$content")

        val notification = Notification.Builder(this, CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_stylus_charging)
            .setContentTitle(title)
            .setContentText(content)
            .setOngoing(true)
            .setOnlyAlertOnce(true)
            .setCategory(Notification.CATEGORY_STATUS)
            .setVisibility(Notification.VISIBILITY_PUBLIC)
            .build()

        notificationManager.notify(NOTIFICATION_ID, notification)
    }

    private fun hideNotification() {
        if (DEBUG) Log.d(TAG, "hideNotification() called")
        notificationManager.cancel(NOTIFICATION_ID)
    }

    companion object {
        private const val TAG = "PenChargingService"
        private const val CHANNEL_ID = "pen_charging_status"
        private const val NOTIFICATION_ID = 1001
        private const val POLL_INTERVAL_MS = 7_000L // 7 seconds
        private const val BATTERY_LEVEL_UNKNOWN = Int.MIN_VALUE

        /**
         * Debug flag to enable verbose logging.
         * Set to true for development/debugging, false for production.
         */
        var DEBUG = false
            set(value) {
                field = value
                // Also propagate to PenChargingManager
                PenChargingManager.DEBUG = value
            }

        /**
         * Start the pen charging service if hardware is available
         */
        @JvmStatic
        fun start(context: Context) {
            if (DEBUG) Log.d(TAG, "start() called")

            if (PenChargingManager.isAvailable()) {
                val intent = Intent(context, PenChargingService::class.java)
                context.startService(intent)
                Log.i(TAG, "Starting PenChargingService")
            } else {
                Log.w(TAG, "IDT hardware not available, service not started")
            }
        }

        /**
         * Stop the pen charging service
         */
        @JvmStatic
        fun stop(context: Context) {
            if (DEBUG) Log.d(TAG, "stop() called")
            context.stopService(Intent(context, PenChargingService::class.java))
        }
    }
}

/*
 * Copyright (C) 2025 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions.stylus

import android.util.Log
import java.io.File

/**
 * Manager for reading Xiaomi Stylus wireless charging status.
 * Reads directly from sysfs nodes exposed by the idtp9418 driver.
 */
object PenChargingManager {

    private const val TAG = "PenChargingManager"
    private const val IDT_PATH = "/sys/class/power_supply/idt"

    /**
     * Debug flag to enable verbose logging.
     * Set to true for development/debugging, false for production.
     */
    var DEBUG = false

    /**
     * Data class representing pen charging status
     */
    data class PenChargingStatus(
        val isConnected: Boolean,
        val isCharging: Boolean,
        val batteryLevel: Int // 0-100, or -1 if unknown
    )

    /**
     * Check if the IDT wireless charging hardware is available
     */
    fun isAvailable(): Boolean {
        val available = File(IDT_PATH).exists()
        if (DEBUG) {
            Log.d(TAG, "isAvailable() = $available, path = $IDT_PATH")
        }
        return available
    }

    /**
     * Get current pen charging status
     * @return PenChargingStatus or null if unavailable
     *
     * Uses reverse_iout (charging current in mA) as the reliable real-time
     * indicator - it's only > 0 when a pen is actively charging.
     */
    fun getStatus(): PenChargingStatus? {
        if (!isAvailable()) {
            if (DEBUG) Log.d(TAG, "getStatus() returning null - hardware not available")
            return null
        }

        return try {
            // Primary indicator: actual charging current (mA)
            // This is 0 when no pen is connected, > 0 when actively charging
            val iout = readSysfsInt("$IDT_PATH/reverse_iout") ?: 0

            // Battery level from pen (only valid when connected)
            val soc = readSysfsInt("$IDT_PATH/reverse_pen_soc") ?: -1

            // Pen is connected and charging if current is flowing
            // Typical charging current is 100-500mA
            val isCharging = iout > 0
            val isConnected = isCharging // If current flows, pen must be connected

            if (DEBUG) {
                Log.d(TAG, "getStatus(): iout=${iout}mA, soc=$soc%, " +
                        "isConnected=$isConnected, isCharging=$isCharging")
            }

            PenChargingStatus(
                isConnected = isConnected,
                isCharging = isCharging,
                batteryLevel = soc
            )
        } catch (e: Exception) {
            Log.e(TAG, "Failed to read pen charging status", e)
            null
        }
    }

    private fun readSysfsInt(path: String): Int? {
        return try {
            val value = File(path).readText().trim().toIntOrNull()
            if (DEBUG) Log.v(TAG, "readSysfsInt($path) = $value")
            value
        } catch (e: Exception) {
            if (DEBUG) Log.w(TAG, "readSysfsInt($path) failed: ${e.message}")
            null
        }
    }
}

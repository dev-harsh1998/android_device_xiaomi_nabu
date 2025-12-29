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
    var DEBUG = true

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
     */
    fun getStatus(): PenChargingStatus? {
        if (!isAvailable()) {
            if (DEBUG) Log.d(TAG, "getStatus() returning null - hardware not available")
            return null
        }

        return try {
            val hall3 = readSysfsInt("$IDT_PATH/reverse_chg_hall3") ?: 0
            val hall4 = readSysfsInt("$IDT_PATH/reverse_chg_hall4") ?: 0
            val chgMode = readSysfsInt("$IDT_PATH/reverse_chg_mode") ?: 0
            val soc = readSysfsInt("$IDT_PATH/reverse_pen_soc") ?: -1

            val isConnected = hall3 == 1 || hall4 == 1
            val isCharging = chgMode == 1 && isConnected

            if (DEBUG) {
                Log.d(TAG, "getStatus() raw values: hall3=$hall3, hall4=$hall4, " +
                        "chgMode=$chgMode, soc=$soc")
                Log.d(TAG, "getStatus() computed: isConnected=$isConnected, " +
                        "isCharging=$isCharging, batteryLevel=$soc")
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

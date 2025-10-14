/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions

import android.os.ServiceManager
import android.util.Log
import custom.hardware.hwcontrol.IHwControl

/**
 * Singleton manager for hardware control AIDL service.
 * Provides thread-safe access to IHwControl interface with lazy initialization.
 */
object HwStateManager {
    
    private val TAG = HwStateManager::class.java.simpleName
    private const val AIDL_INTERFACE = "custom.hardware.hwcontrol.IHwControl/default"
    
    @Volatile
    private var hwControl: IHwControl? = null
    private val lock = Any()
    
    /**
     * Initialize the AIDL interface connection.
     * Should be called during boot to establish connection early.
     */
    fun initialize() {
        getHwControl()
    }
    
    /**
     * Set hardware control state.
     * @param hwType Hardware type from HwType enum
     * @param state State to set (typically 0 = disabled, 1 = enabled)
     */
    fun setHwState(hwType: Int, state: Int) {
        val service = getHwControl() ?: run {
            Log.e(TAG, "Hardware control interface unavailable")
            return
        }
        
        try {
            service.setHwState(hwType, state)
            Log.d(TAG, "Hardware type $hwType set to state $state")
        } catch (e: Exception) {
            Log.e(TAG, "Failed to set hardware type $hwType to state $state", e)
        }
    }
    
    /**
     * Lazy initialization of IHwControl interface with double-checked locking.
     */
    private fun getHwControl(): IHwControl? {
        if (hwControl == null) {
            synchronized(lock) {
                if (hwControl == null) {
                    try {
                        val binder = ServiceManager.waitForDeclaredService(AIDL_INTERFACE)
                        hwControl = IHwControl.Stub.asInterface(binder)
                        Log.i(TAG, "Connected to hardware control service")
                    } catch (e: Exception) {
                        Log.e(TAG, "Failed to connect to hardware control service", e)
                    }
                }
            }
        }
        return hwControl
    }
}


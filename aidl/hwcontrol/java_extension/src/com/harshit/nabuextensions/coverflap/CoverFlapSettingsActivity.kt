/*
 * Copyright (C) 2023 Harshit Jain <dev-harsh1998@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.harshit.nabuextensions.coverflap

import android.os.Bundle
import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity

class CoverFlapSettingsActivity : CollapsingToolbarBaseActivity() {
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        supportFragmentManager.beginTransaction()
            .replace(
                com.android.settingslib.collapsingtoolbar.R.id.content_frame,
                CoverFlapSettingsFragment(),
                TAG_COVERFLAP
            )
            .commit()
    }
    
    companion object {
        private const val TAG_COVERFLAP = "CoverFlap"
    }
}

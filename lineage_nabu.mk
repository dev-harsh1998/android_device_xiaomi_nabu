# Copyright (C) 2023 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_tablet_wifionly.mk)

# Inherit some common AOSP stuff.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# Signing keys (Private).
ifneq ("$(wildcard vendor/harshit/product.mk)", "")
$(call inherit-product, vendor/harshit/product.mk)
endif

# Inherit device configuration for nabu.
$(call inherit-product, device/xiaomi/nabu/device.mk)

PRODUCT_NAME := lineage_nabu
PRODUCT_DEVICE := nabu
PRODUCT_BRAND := Xiaomi
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_MODEL := Xiaomi Pad 5

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="nabu_global-user 13 RKQ1.200826.002 V816.0.1.0.TKXMIXM release-keys"

BUILD_FINGERPRINT := Xiaomi/nabu_global/nabu:13/RKQ1.200826.002/V816.0.1.0.TKXMIXM:user/release-keys
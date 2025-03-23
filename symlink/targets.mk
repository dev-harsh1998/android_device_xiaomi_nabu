# RFS Symlink inherited from hardware/qcom-caf/common/common.mk
$(warning "RFS Symlink inherited from hardware/qcom-caf/common/common.mk")
$(call inherit-product, hardware/qcom-caf/common/common.mk)

$(warning "Will create wlan symlinks")
# Wlan symlinks
PRODUCT_PACKAGES += \
    firmware_WCNSS_qcom_cfg.ini_symlink \
    firmware_wlan_mac.bin_symlink
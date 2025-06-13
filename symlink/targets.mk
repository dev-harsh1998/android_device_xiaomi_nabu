# RFS Symlink inherited from hardware/qcom-caf/common/common.mk
$(warning "RFS Symlink inherited from hardware/qcom-caf/common/common.mk")
$(call inherit-product, hardware/qcom-caf/common/common.mk)

$(warning "Will create symlinks")
# symlinks
PRODUCT_PACKAGES += \
    firmware_WCNSS_qcom_cfg.ini_symlink \
    firmware_wlan_mac.bin_symlink \
    vendor_bt_firmware_mountpoint \
    vendor_dsp_mountpoint \
    vendor_firmware_mnt_mountpoint
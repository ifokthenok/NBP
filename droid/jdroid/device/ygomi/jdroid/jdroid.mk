
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
# If you're using 4.2/Jelly Bean, use full_base.mk instead of full.mk
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

PRODUCT_NAME := jdroid
PRODUCT_DEVICE := jdroid
PRODUCT_MODEL := Jailed droid from Connexis
PRODUCT_MANUFACTURER := Connexis
PRODUCT_BRAND := jdroid

DEVICE_PACKAGE_OVERLAYS := device/ygomi/jdroid/overlay

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    net.dns1=8.8.8.8 \
    net.dns2=8.8.4.4 \
    persist.sys.strictmode.disable=true \
    ro.lockscreen.disable.default=true \
    config.disable_bluetooth=true \
    config.disable_telephony=true \
    config.disable_systemui=true \
    config.disable_noncore=true 


# Ygomi apps
PRODUCT_PACKAGES += \
    HomeDemo \
    StoreDemo \
    SyncDemo

PRODUCT_COPY_FILES += \
    tools/script/oip2droid.sh:system/bin/wl2droid.sh \
    tools/script/dnsproxy2:system/bin/dnsproxy2 \
    tools/script/dnsproxy.sh:system/bin/dnsproxy.sh \
    device/ygomi/jdroid/init.jdroid.oip.rc:root/init.freescalei.mx6quad.rc \
    device/ygomi/jdroid/init.jdroid.j6.rc:root/init.jacinto6evmboard.rc \
    device/generic/goldfish/camera/media_profiles.xml:system/etc/media_profiles.xml \
    device/generic/goldfish/camera/media_codecs.xml:system/etc/media_codecs.xml




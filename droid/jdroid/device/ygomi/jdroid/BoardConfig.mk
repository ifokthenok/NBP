
# config.mk
#
# Product-specific compile-time definitions.
#

# use mini-emulator-armv7-a-neon as a start point
include device/generic/mini-emulator-armv7-a-neon/BoardConfig.mk

BOARD_SYSTEMIMAGE_PARTITION_SIZE := 346712960
BOARD_USERDATAIMAGE_PARTITION_SIZE := 7000000
BOARD_CACHEIMAGE_PARTITION_SIZE := 5000000

# Guess working
BOARD_HAVE_BLUETOOTH := false
TARGET_NO_RADIOIMAGE := true
HAVE_HTC_AUDIO_DRIVER := true


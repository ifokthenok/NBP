#!/bin/sh
#
# This is the script to launch droid from Arago Linux
#
# Version 0.4
#
# Revisions
#
#   0.4 2014-10-18  Revised to self detecting runtime folder
#   0.3 2014-10-16  Adjusted for OIP baseline with droid support
#   0.2 2014-09-16  Adjusted for iMX6Q 
#   0.1 2014-05-22  By Yanfeng
#
#

_prog=$0
if [ -z "$D_DIR" ]; then
    D_DIR=`dirname $_prog`
    D_DIR=`dirname $D_DIR`
fi
R_DIR=$D_DIR/root

if (echo $D_DIR|grep -v "^/">/dev/null); then
    echo
    echo please specify the full path of this script or 
    echo define D_DIR environment variable.
    echo 
    exit 10
fi

# mount point for the jail of droid
#R_DIR=/droid/root
#R_DIR=/mnt/oip_app_data/droid/root

# physical folder holding droid image and data folders
#D_DIR=/droid
RAM_I=$D_DIR/img/ramdisk.img
SYS_I=$D_DIR/img/system.img

echo "Checking pre-conditions..."
[ ! -d $D_DIR ] && echo "$D_DIR is missing" && exit 1
[ ! -d $D_DIR/data ] && echo "$D_DIR/data is missing" && exit 1
[ ! -r $SYS_I ] && echo "$SYS_I is missing" && exit 1
[ ! -r $RAM_I ] && echo "$RAM_I is missing" && exit 1

[ ! -d $R_DIR ] && echo "Make root folder $R_DIR..." && mkdir -p $R_DIR
[ ! -d $R_DIR ] && echo "$R_DIR doesn't exist!" && exit 2

# avoid reentrance
df -h | fgrep $R_DIR && echo && echo " --- dirty env? have to exit." && exit 2

# for debugging purpose, skip using tmpfs and use a real folder for jail

if true; then
    echo "Mounting root volume $R_DIR..."
    mkdir -p $R_DIR 2>/dev/null
    mount -t tmpfs tmpfs $R_DIR || exit $?
fi

_CWD=`pwd`

cd $R_DIR

# only use ramdisk.img for once
if [ ! -f init ]; then
    echo populating root folder...
    (zcat $RAM_I | cpio -id >/dev/null) || exit $?
fi

echo "Mounting droid system volume..."
modprobe loop
mkdir -p data system
mount -o ro $SYS_I system || exit $?
mount --bind $D_DIR/data data || exit $?


mkdir -p mnt/asec mnt/obb cache mnt/sdcard
mount -o mode=2777,uid=1000 -t tmpfs tmpfs mnt/sdcard
mount -t tmpfs tmpfs cache

# make the wayland socket path visible in jail 
mkdir -p var/run var/lib tmp
mount --bind /run var/run
mount --bind /tmp tmp

echo "Done." && df -h | fgrep $R_DIR

echo ""
echo ""
echo "After making sure Internet connection is ready, please run"
echo ""
echo "   # droid.start.sh"
echo ""
echo "to start droid on this device"

cd $_CWD

exit 0

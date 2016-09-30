#!/bin/sh
# a simple script to start droid in a Wayland environment
#

#R_DIR=/droid/root
#R_DIR=/mnt/oip_app_data/droid/root

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

# checking droid folders...
[ ! -x $R_DIR/init ] && echo doing droid.prep.sh first! && exit 1

# checking wayland sockets...
[ ! -d $XDG_RUNTIME_DIR ] && echo is Wayland installed? && exit 2
chmod a+x $XDG_RUNTIME_DIR

_WAYLAND=$XDG_RUNTIME_DIR/wayland-0
[ ! -S $_WAYLAND ] && echo is Wayland running? && exit 3
chmod a+rw $_WAYLAND


chroot $R_DIR /init &
_ptree_root=$!

echo droid process tree root is $_ptree_root
#pstree -p `pgrep init|tail -1`


#!/bin/bash
#
# a simple script that drops unwanted apks from the given system.img
# on a linux host machine
# 
# version: 0.1
#
# history:
#
#       2014-11-13  0.1 initial version
#
# usage:   $0  inImageFile
# 
# 

_version=0.1
_prog=`basename $0`

function help()
{
    echo "$_prog version $_version"
    echo "usage: $_prog inImage"
}

# function to judge if an input argument is a needed file
function isWanted()
{
    _W_APPS="Browser LatinIME PackageInstaller MediaProvider FusedLocation
             SettingsProvider DefaultContainerService DownloadProvider 
             HomeDemo StoreDemo SyncDemo"

    for a in $_W_APPS; do
        if (echo $1 | fgrep -q $a.apk); then return $?; fi
    done

    # not found
    return 2
}

# Check input file
_IMG_I=$1
[ -z "$_IMG_I" ] && help && exit 1
[ ! -f "$_IMG_I" ] && echo "$_IMG_I doesn't exist!" && exit 1
file $_IMG_I | fgrep -q "filesystem data"
[ $? -ne 0 ] && echo "Is $_IMG_I a filesystem image?" && exit 1

# mounting images
_D_I=`mktemp -d`
trap "df | grep -e $_D_I'$' && sudo umount $_D_I >/dev/null && rm -rf $_D_I" EXIT

sudo mount $_IMG_I $_D_I 
[ $? -ne 0 ] && echo "Failed to mount $_IMG_I" && exit 1

# Purging unwanted files
for f in $_D_I/app/* $_D_I/priv-app/* ; do
    if ( isWanted "$f" ); then
        echo keep $f
    else
        sudo rm -f $f
    fi
done

# cleaning up
sudo umount $_D_I
exit 0


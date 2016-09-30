#!/bin/sh
# the function to clean up droid runtime env, note system images and
# data folder is not cleaned.
# TODO: stop droid process tree before cleaning up

# mount point for runtime droid folder, i.e. the jail
#R_DIR=/tmp/jail
#R_DIR=/droid/root
#R_DIR=/mnt/oip_app_data/droid/root

_prog=$0
if [ -z "$R_DIR" ]; then
    D_DIR=`dirname $_prog`
    D_DIR=`dirname $D_DIR`
    R_DIR=$D_DIR/root
else
    D_DIR=`dirname $R_DIR`
fi

echo In case of failure, please clean up droid process tree manually.

echo Umounting data and system folders...
_folders=`mount|awk '{print $3}'|grep $R_DIR|sort -r`
while [ "" != "$_folders" ]; do
   umount $_folders
   _folders=`mount|awk '{print $3}'|grep $R_DIR|sort -r`
done

#echo Please manually clean up root volume stuffs
umount $R_DIR 2>/dev/null

exit 0

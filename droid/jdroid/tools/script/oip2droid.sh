#!/system/bin/sh
# a wrapper script to run wl2droid service together with input injection
#
# the input command will start only after sys.boot_completed is "1"

# check if 2nd display is needed
N=`/system/bin/getprop persist.sys.wayland.num`
if [ "$N" -gt 1 ]; then
   /system/bin/wl2droid | /system/bin/input pipe &
   /system/bin/wl2droid -e | /system/bin/input pipe
else
   /system/bin/wl2droid | /system/bin/input pipe 
fi


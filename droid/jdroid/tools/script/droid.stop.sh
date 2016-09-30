#!/bin/sh
# a simple script to stop droid 
#

# searching for the droid root process
for p in `pidof init`; do
   if [ $p != "1" ] && pstree $p|fgrep -q -e zygote -e adbd; then
       P_ROOT=$p;
       break;
   fi
done

[ -z "$P_ROOT" ] && echo "Is droid running?" && exit 1 

# echo trying to find droid init process tree
echo Found droid process tree ... 
pstree -p $P_ROOT
_PIDS=`pstree -p $P_ROOT|sed 's/(/\n(/g' | grep '(' | sed 's/(\(.*\)).*/\1/' | tr "\n" " "`
echo About to kill the process tree...
[ ! -z "$_PIDS" ] && kill -9 $_PIDS 
echo done
exit 0

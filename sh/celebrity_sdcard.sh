#!/bin/bash

[ $# -lt 1 ] && echo "Please specify img dir as the first argument." && exit -1; 
[ ! -d $1 ] && echo "$1 is not a dir" && exit -1;

set -x

IMG_DIR=${1%/}
TMP_DIR=`mktemp -d`


mkdir -p $TMP_DIR/system $TMP_DIR/data

cp -r $IMG_DIR/system/*	$TMP_DIR/system/
cp -r $IMG_DIR/data/*	$TMP_DIR/data/	

sudo rm -rf /media/system/* 
sudo rm -rf /media/data/*

sudo cp -r $TMP_DIR/system/* /media/system/
sudo cp -r $TMP_DIR/data/* /media/data/

rm -rf $TMP_DIR

sync

set +x

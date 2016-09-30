#!/bin/bash
#cd ./protocol/
#wayland-scanner client-header < wayland.xml > wayland-client-protocol.h
#wayland-scanner server-header < wayland.xml > wayland-server-protocol.h
#wayland-scanner code < wayland.xml > wayland-protocol.c

cd ./protocol/
for file in *.xml
do

	myvar="${file%.*}"
	echo "Parse file: ${file}"
	wayland-scanner client-header < ${file} > ./${myvar}-client-protocol.h
	wayland-scanner server-header < ${file} > ./${myvar}-server-protocol.h
	wayland-scanner code < ${file} > ./${myvar}-protocol.c
done

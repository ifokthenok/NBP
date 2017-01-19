obj-m := hello.o memory.o parlelport.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD ?= $(shell pwd) 
defalult: 
	make -C ${KERNELDIR} M=${PWD} modules 
install: 
	insmod hello.ko memory.ko parlelport.ko
uninstall: 
	rmmod hello.ko memory.ko parlelport.ko
clear: 
	make -C ${KERNELDIR} M=${PWD} clean

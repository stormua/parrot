#TARGET = pp_dma_driver

pp_dma_driver-objs := pp_dma_driver.o device_ioctl.o

#CFLAGS_MODULE=-Werror

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	+= pp_dma_driver.o


else

#KERNELDIR ?= /usr/src/linux
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

default: 
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

endif

clean:
	rm -rf *.o *.ko *~ core .depend *.mod.c .*.cmd .tmp_versions .*.o.d

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ins: default rem
	insmod $(TARGET).ko debug=1

rem:
	@if [ -n "`lsmod | grep -s $(TARGET)`" ]; then rmmod $(TARGET); echo "rmmod $(TARGET)"; fi

ifeq (.depend,$(wildcard .depend))
include .depend
endif


test1:
	gcc -o test1 test_parrot.c


test2:
	gcc -o test2 test2_parrot.c


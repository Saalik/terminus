ifneq ($(KERNELRELEASE),)
obj-m += terminusmod.o
else
#KERNELDIR ?=/lib/modules/$(shell uname -r)/build
KERNELDIR ?=../../linux-4.2.3
#KERNELDIR ?=~/Cave/linux
PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

.PHONY: all
endif

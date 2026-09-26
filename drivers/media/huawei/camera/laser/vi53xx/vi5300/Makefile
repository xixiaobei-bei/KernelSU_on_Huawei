ifneq ($(KERNELRELEASE),)
include Kbuild

else
KDIR ?= /lib/modules/`uname -r`/build

default:
	CONFIG_VI5300=m $(MAKE) -C $(KDIR) M=$$PWD
clean:
	CONFIG_VI5300=m $(MAKE) -C $(KDIR) M=$$PWD clean

endif
obj-$(CONFIG_DKMD_CMDLIST_ENABLE) += dkmd_cmdlist.o

dkmd_cmdlist-objs := \
	cmdlist_node.o \
	disp_cmdlist.o \
	cmdlist_dev.o \
	cmdlist_mem.o \
	cmdlist_drv.o

EXTRA_CFLAGS += \
	-Idrivers/video/fbdev/dkmd/cmdlist \
	-Idrivers/video/fbdev/dkmd/cmdlist/include \
	-Idrivers/video/fbdev/dkmd/dksm/fence \
	-Iinclude/media/dkmd \
	-Iarch/arm64/include/ \
	-Iinclude/

# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_CMDLIST_ENABLE),m)

# obj-y += test/

KERNEL_DIR = $(ANDROID_BUILD_TOP)/kernel/linux-4.14
KERNEL_OUT = $(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ
PWD := $(shell pwd)
ARCH := arm64
CROSS_COMPILE := aarch64-linux-android-
CFLAGS_MODULE := -fno-pic

default:
	$(MAKE) -C $(KERNEL_OUT) ARCH=$(ARCH) CFLAGS=$(EXTRA_CFLAGS) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
	$(MAKE) -C $(KERNEL_OUT) M=$(PWD) INSTALL_MOD_PATH=$(PWD)/signed INSTALL_MOD_STRIP=1 modules_install

endif

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d
	$(MAKE) -C test clean

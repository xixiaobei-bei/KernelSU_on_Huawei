obj-$(CONFIG_DKMD_DPU_CONNECTOR) += mipi/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += dp/
obj-$(CONFIG_DKMD_DPU_CONNECTOR) += panel/

# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DPU_CONNECTOR_TEST),m)

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
	$(MAKE) -C mipi clean
	$(MAKE) -C panel clean
	$(MAKE) -C dp clean
	$(MAKE) -C test clean



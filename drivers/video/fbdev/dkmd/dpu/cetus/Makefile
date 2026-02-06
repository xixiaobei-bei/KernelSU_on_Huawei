obj-$(CONFIG_DKMD_DPU_ENABLE) += composer/
obj-$(CONFIG_DKMD_DPU_ENABLE) += device/
obj-$(CONFIG_DKMD_DPU_ENABLE) += res_mgr/
obj-$(CONFIG_DKMD_DPU_ENABLE) += common/

# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DPU_ENABLE_TEST),m)

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
	$(MAKE) -C composer clean
	$(MAKE) -C device clean
	$(MAKE) -C res_mgr clean
	$(MAKE) -C common clean

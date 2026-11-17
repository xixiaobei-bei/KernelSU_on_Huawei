obj-$(CONFIG_DKMD_DKSM) += dkmd_dksm.o

dkmd_dksm-objs = \
	./fence/dkmd_fence_utils.o \
	./fence/dkmd_release_fence.o \
	./fence/dkmd_acquire_fence.o \
	./timeline/dkmd_timeline.o \
	./buf_sync/dkmd_buf_sync.o \
	./dkmd_utils.o

EXTRA_CFLAGS += \
	-Idrivers/video/fbdev/dkmd/dksm/buf_sync \
	-Idrivers/video/fbdev/dkmd/dksm/fence \
	-Idrivers/video/fbdev/dkmd/dksm/smmu \
	-Idrivers/video/fbdev/dkmd/dksm/timeline \
	-Idrivers/video/fbdev/dkmd/dksm/

# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile
ifeq ($(CONFIG_DKMD_DKSM),m)

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
	find . -name "*.o" | xargs rm -f
	find . -name ".*.cmd" | xargs rm -f
	find . -name ".*.d" | xargs rm -f

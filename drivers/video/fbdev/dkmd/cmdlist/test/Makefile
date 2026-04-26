obj-m += cmdlist_test.o

EXTRA_CFLAGS += \
	-Idrivers/video/fbdev/dkmd/kunit \
	-Idrivers/video/fbdev/dkmd/cmdlist \
	-Idrivers/video/fbdev/dkmd/cmdlist/include \
	-Iinclude/media/dkmd \
	-Iarch/arm64/include \
	-Iinclude

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d


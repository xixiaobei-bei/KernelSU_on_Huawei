obj-m += connector_test.o

EXTRA_CFLAGS += \
	-Idrivers/video/fbdev/dkmd/kunit \
	-Idrivers/video/fbdev/dkmd/connector \
	-Idrivers/video/fbdev/dkmd/connector/mipi \
	-Idrivers/video/fbdev/dkmd/connector/panel \
	-Iarch/arm64/include \
	-Iinclude

clean:
	rm -rf *.o .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.symversions signed .*.d


ifneq ($(KERNELRELEASE),)

ccflags-y += -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror

obj-$(CONFIG_D2D_PROTOCOL) := d2dp.o

d2dp-y := dynamic_list.o	\
	  headers.o		\
	  kobject.o		\
	  options.o		\
	  protocol.o		\
	  rx_buffer.o		\
	  timers.o		\
	  trace.o		\
	  transport.o		\
	  tx_buffer.o		\
	  main.o

# enable include directory for correct trace generation
CFLAGS_trace.o += -I$(src)

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	rm -f *.a *.ko *.o .*.o.d .*.cmd .*.flags *.mod.c *.order
	rm -f .*.*.cmd *.symvers
	rm -rf .tmp_versions/

endif

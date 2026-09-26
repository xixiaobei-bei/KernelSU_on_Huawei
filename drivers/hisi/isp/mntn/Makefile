##ifneq ($(TARGET_BUILD_VARIANT),user)
#ifneq ($(kcov),true)
ifeq ($(CONFIG_HISP_DEBUG), y)
#obj-$(CONFIG_HISP_RDR) += ispperf.o ispmonitor.o ispcoresight.o
subdir-ccflags-y += -DDEBUG_HISP
#endif
endif

obj-$(CONFIG_HISP_RDR) += isprdr.o isplog.o

subdir-ccflags-y += -I$(srctree)/drivers/hisi/mntn/blackbox/

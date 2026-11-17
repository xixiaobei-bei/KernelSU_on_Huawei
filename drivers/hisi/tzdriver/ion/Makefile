KERNEL_DIR :=$(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEF_ENG
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/staging/android

obj-$(CONFIG_STATIC_ION) += static_ion_mem.o
obj-$(CONFIG_DYNAMIC_ION) += dynamic_ion_mem.o
ifneq ($(CONFIG_MTK_PLATFORM), )
obj-$(CONFIG_STATIC_ION) += mtk/declare_static_ion.o
else
obj-$(CONFIG_STATIC_ION) += hisi/declare_static_ion.o
endif

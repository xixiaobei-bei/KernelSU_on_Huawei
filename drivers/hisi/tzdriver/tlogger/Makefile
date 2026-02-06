ifeq ($(strip $(TARGET_PRODUCT)), mdc)
	ccflags-y += -DCONFIG_MDC_PLATFORM
endif

ifeq ($(strip $(TARGET_PRODUCT)), mini)
	ccflags-y += -DCONFIG_MINI_PLATFORM
endif

KERNEL_DIR :=$(srctree)

ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDEF_ENG
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/core

obj-$(CONFIG_TEELOG) += tlogger.o

ifeq ($(CONFIG_RDR_MEM),y)
	ifeq ($(CONFIG_MNTN),y)
		obj-$(CONFIG_MNTN) += log_bbox_cfg.o
	else
		obj-$(CONFIG_HISI_BB) += log_rdr_cfg.o
	endif
else
	obj-y += log_pages_cfg.o
endif

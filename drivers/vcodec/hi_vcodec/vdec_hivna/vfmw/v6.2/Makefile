# local variables
#===============================================================================
VFMW_DIR := drivers/vcodec/hi_vcodec/vdec_hivna/vfmw/v6.2

################################################################################################
#    PRODUCT
################################################################################################
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/v2.0
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/common
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/include
include $(VFMW_DIR)/product/hi36a0/linux_cfg/vfmw_make.cfg

EXTRA_CFLAGS += $(VFMW_CFLAGS)
EXTRA_CFLAGS += -fno-pic

MOD_NAME := hi_vfmw
#obj-m += $(MOD_NAME).o
obj-y += $(MOD_NAME).o

VFMW_OBJ := $(VFMW_SRC:%.c=%.o)
$(MOD_NAME)-y := $(VFMW_OBJ)

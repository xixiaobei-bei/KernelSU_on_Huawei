# local variables
#===============================================================================
VFMW_DIR := drivers/vcodec/hi_vcodec/vdec_hivna/vfmw/vfmw_v6.2

################################################################################################
#    PRODUCT
################################################################################################
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/vdec/drv_v2.0
EXTRA_CFLAGS += -Idrivers/vcodec/hi_vcodec/vdec_hivna/include
include $(VFMW_DIR)/product/HiVCodecV600/linux_cfg/vfmw_make.cfg

EXTRA_CFLAGS += $(VFMW_CFLAGS)
EXTRA_CFLAGS += -fno-pic

MOD_NAME := hi_vfmw
#obj-m += $(MOD_NAME).o
obj-y += $(MOD_NAME).o

VFMW_OBJ := $(VFMW_SRC:%.c=%.o)
$(MOD_NAME)-y := $(VFMW_OBJ)

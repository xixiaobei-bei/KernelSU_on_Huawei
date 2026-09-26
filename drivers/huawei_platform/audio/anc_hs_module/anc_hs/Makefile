ifneq ($(TARGET_BUILD_VARIANT), user)
ccflags-y += -DANC_HS_DEBUG
endif
obj-$(CONFIG_ANC_HS) := anc_hs.o
ccflags-$(CONFIG_ANC_HS) += -Idrivers/devkit/audiokit/

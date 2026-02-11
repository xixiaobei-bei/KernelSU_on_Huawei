KCOV_INSTRUMENT := n
CONFIG_TEE_SMP := y

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DDEBUG_TUI
ccflags-y += -DDEF_ENG
endif

ccflags-y +=-DDYNAMIC_ION
ifeq ($(CONFIG_TEE_SMP), y)
ccflags-y += -DCONFIG_TEE_SMP
obj-$(CONFIG_TZDRIVER) += smc_smp.o
else
obj-$(CONFIG_TZDRIVER) += smc.o
endif

obj-$(CONFIG_TZDRIVER) += tc_client_driver.o tc_client_sub_driver.o teek_client_api.o mailbox_mempool.o
obj-$(CONFIG_TZDRIVER) += security_auth_enhance.o agent.o gp_ops.o mem.o cmdmonitor.o tzdebug.o tz_spi_notify.o  dynamic_mem.o
obj-$(CONFIG_TEELOG) += tlogger.o tee_rdr_register.o
ifeq ($(CONFIG_TZDRIVER), y)
obj-$(CONFIG_HISI_RPMB) += agent_rpmb.o
endif

obj-$(CONFIG_TEE_TUI) += tui.o


ccflags-y += 	-Idrivers/video/hisi/dss \
		-Idrivers/video/fbdev/hisi/dss \
		-Idrivers/staging/android \

ifdef CONFIG_IMAGINATION_D5500_DRM_VCODEC
ifeq ($(wildcard drivers/staging/android/ion/ion.h),)
  ccflags-y += -DIMG_KERNEL_ION_HEADER="<linux/ion.h>"
else
  ccflags-y += -DIMG_KERNEL_ION_HEADER="<$(srctree)/drivers/staging/android/ion/ion.h>"
endif
  ccflags-y += -Idrivers/vcodec/imagination/D5500_DRM/imgvideo/platform/include
endif

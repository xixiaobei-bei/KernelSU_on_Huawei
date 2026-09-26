ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/hisi/ap/platform/$(TARGET_BOARD_PLATFORM)
ccflags-y += -Idrivers/hisi/tzdriver

obj-$(CONFIG_ION_HISI)+= of_mm_ion.o mm_ion_dump.o sec_alloc.o
obj-$(CONFIG_ION_HISI_SECSG) += ion_tee_op.o
obj-$(CONFIG_ION_HISI_DMA_POOL) += ion_tee_op.o
obj-$(CONFIG_HISI_VLTMM) += vltmm.o vltmm_agent.o

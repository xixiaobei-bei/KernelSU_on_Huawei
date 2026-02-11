ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/hisi/ap/platform/$(TARGET_BOARD_PLATFORM)
ccflags-y += -Idrivers/hisi/tzdriver

obj-$(CONFIG_ION_HISI)+= of_hisi_ion.o hisi_ion_dump.o sec_alloc.o
obj-$(CONFIG_ION_HISI_SECSG) += ion_tee_op.o


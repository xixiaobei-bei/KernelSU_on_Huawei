# SPDX-License-Identifier: GPL-2.0
#
# Makefile for ARPP drivers.
#

obj-$(CONFIG_HISI_ARPP) += arpp.o

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver/libhwsecurec

ccflags-$(CONFIG_HISI_ARPP_V100) += -Idrivers/hisi/arpp/platform/v100

arpp-objs := \
	arpp_smmu.o \
	arpp_isr.o \
	arpp_hwacc.o \
	arpp_core.o

obj-$(CONFIG_HISI_ARPP_V100) += /platform/v100/

clean:
	rm *.o .*cmd

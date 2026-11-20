#
# Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
# Description: Makefile for the Huawei root scanner.
# Author: likun <quentin.lee@huawei.com>
# Create: 2016-06-18
#

obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_utils.o
obj-$(CONFIG_HW_ROOT_SCAN) += kcode.o
obj-$(CONFIG_HW_ROOT_SCAN) += rproc.o
obj-$(CONFIG_HW_ROOT_SCAN) += sescan.o
obj-$(CONFIG_HW_ROOT_SCAN) += setids.o
obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_scanner.o
obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_module.o
ifdef CONFIG_MTK_PLATFORM
ccflags-$(CONFIG_HW_ROOT_SCAN) += -Idrivers/ca_antiroot
else
ccflags-$(CONFIG_HW_ROOT_SCAN) += -Idrivers/hisi/ca_antiroot
endif
ccflags-$(CONFIG_HW_ROOT_SCAN) += -Ivendor/huawei/chipset_common/modules/libc_sec/include
ccflags-$(CONFIG_HW_ROOT_SCAN) += -Isecurity/selinux/include

flags += -Wdate-time -Wfloat-equal

# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the kernel block layer
#

obj-$(CONFIG_BLOCK) := bio.o elevator.o blk-core.o blk-tag.o blk-sysfs.o \
			blk-flush.o blk-settings.o blk-ioc.o blk-map.o \
			blk-exec.o blk-merge.o blk-softirq.o blk-timeout.o \
			blk-lib.o blk-mq.o blk-mq-tag.o blk-stat.o \
			blk-mq-sysfs.o blk-mq-cpumap.o blk-mq-sched.o ioctl.o \
			genhd.o partition-generic.o ioprio.o \
			badblocks.o partitions/
obj-$(CONFIG_HISI_BLK) += hisi-blk-core.o hisi-blk-dump.o hisi-blk-latency.o hisi-blk-busy-idle-notify.o \
					hisi-blk-flush.o hisi-blk-iosched-ufs-mq.o hisi-blk-iosched-interface.o \
					hisi-blk-busy-idle-interface.o hisi-blk-core-interface.o hisi-blk-dump-interface.o \
					hisi-blk-flush-interface.o hisi-blk-latency-interface.o
obj-$(CONFIG_HISI_BLK_DEBUG) += hisi-blk-ft.o
obj-$(CONFIG_HISI_BKOPS) += hisi-bkops-core.o hisi-bkops-sysfs.o hisi-bkops-core-interface.o \
					hisi-bkops-sysfs-interface.o
obj-$(CONFIG_BOUNCE)		+= bounce.o
obj-$(CONFIG_BLK_SCSI_REQUEST)	+= scsi_ioctl.o
obj-$(CONFIG_BLK_DEV_BSG)	+= bsg.o
obj-$(CONFIG_BLK_DEV_BSGLIB)	+= bsg-lib.o
obj-$(CONFIG_BLK_CGROUP)	+= blk-cgroup.o
obj-$(CONFIG_BLK_DEV_THROTTLING)	+= blk-throttle.o
obj-$(CONFIG_IOSCHED_NOOP)	+= noop-iosched.o
obj-$(CONFIG_IOSCHED_DEADLINE)	+= deadline-iosched.o
obj-$(CONFIG_IOSCHED_ROW)   += row-iosched.o
obj-$(CONFIG_IOSCHED_CFQ)	+= cfq-iosched.o
obj-$(CONFIG_MQ_IOSCHED_DEADLINE)	+= mq-deadline.o
obj-$(CONFIG_MQ_IOSCHED_KYBER)	+= kyber-iosched.o
bfq-y				:= bfq-iosched.o bfq-wf2q.o bfq-cgroup.o
obj-$(CONFIG_IOSCHED_BFQ)	+= bfq.o

obj-$(CONFIG_BLOCK_COMPAT)	+= compat_ioctl.o
obj-$(CONFIG_BLK_CMDLINE_PARSER)	+= cmdline-parser.o
obj-$(CONFIG_BLK_DEV_INTEGRITY) += bio-integrity.o blk-integrity.o t10-pi.o
obj-$(CONFIG_HISI_BLOCK_FREQUENCE_CONTROL) += hisi_freq_ctl.o
obj-$(CONFIG_BLK_MQ_PCI)	+= blk-mq-pci.o
obj-$(CONFIG_HISI_PARTITION) += kirin_partition.o

obj-$(CONFIG_BLK_MQ_VIRTIO)	+= blk-mq-virtio.o
obj-$(CONFIG_BLK_MQ_RDMA)	+= blk-mq-rdma.o
obj-$(CONFIG_BLK_DEV_ZONED)	+= blk-zoned.o
obj-$(CONFIG_BLK_WBT)		+= blk-wbt.o
obj-$(CONFIG_BLK_DEBUG_FS)	+= blk-mq-debugfs.o
obj-$(CONFIG_BLK_SED_OPAL)	+= sed-opal.o

obj-$(CONFIG_HISI_RPMB_UFS)       += hisi_blk_scsi_kern.o

ifeq ($(TARGET_VERSION_MODE),factory)
	EXTRA_CFLAGS += -DBYPASS_AUTHORITY_VERIFY
endif
obj-$(CONFIG_HW_SYSTEM_WR_PROTECT) += software_system_wp.o

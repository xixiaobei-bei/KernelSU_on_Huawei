# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_F2FS_FS) += f2fs.o

f2fs-y		:= dir.o file.o inode.o namei.o hash.o super.o inline.o
f2fs-y		+= checkpoint.o gc.o data.o node.o segment.o recovery.o
f2fs-y		+= shrinker.o extent_cache.o sysfs.o
f2fs-y		+= f2fs_dump_info.o
f2fs-$(CONFIG_F2FS_STAT_FS) += debug.o
f2fs-$(CONFIG_F2FS_FS_XATTR) += xattr.o
f2fs-$(CONFIG_F2FS_FS_POSIX_ACL) += acl.o
f2fs-$(CONFIG_F2FS_IO_TRACE) += trace.o
f2fs-y		+= sdp_context.o policy.o keyinfo.o ecdh.o
f2fs-$(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3) += sdp_metadata.o fbe3_trace.o
f2fs-$(CONFIG_F2FS_TURBO_ZONE) += turbo_zone.o
f2fs-$(CONFIG_HWDPS) += hwdps_context.o
f2fs-y		+= hp_preallocate.o

# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_HMFS_FS) += hmfs.o

hmfs-y		:= dir.o file.o inode.o namei.o hash.o super.o inline.o
hmfs-y		+= checkpoint.o gc.o data.o node.o segment.o recovery.o
hmfs-y		+= shrinker.o extent_cache.o sysfs.o
hmfs-y		+= f2fs_dump_info.o
hmfs-y		+= fscrypt_metadata.o
hmfs-$(CONFIG_HMFS_STAT_FS) += debug.o
hmfs-$(CONFIG_HMFS_FS_XATTR) += xattr.o
hmfs-$(CONFIG_HMFS_FS_POSIX_ACL) += acl.o
hmfs-$(CONFIG_HMFS_IO_TRACE) += trace.o
hmfs-y		+= sdp_context.o policy.o keyinfo.o ecdh.o
hmfs-$(CONFIG_HWDPS) += hwdps_context.o
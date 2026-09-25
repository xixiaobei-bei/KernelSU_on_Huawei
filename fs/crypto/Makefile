obj-$(CONFIG_FS_ENCRYPTION)	+= fscrypto.o

fscrypto-y := crypto.o fname.o hooks.o keyinfo.o policy.o
fscrypto-$(CONFIG_BLOCK) += bio.o
EXTRA_CFLAGS := -I$(srctree)/lib/libc_sec/securec_v2/include

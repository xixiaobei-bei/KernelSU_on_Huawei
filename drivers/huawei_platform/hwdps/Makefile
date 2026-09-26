
obj-$(CONFIG_HWDPS) += src/base/hwdps_utils.o \
		       src/base/hwdps_list.o

obj-$(CONFIG_HWDPS) += src/fek/hwdps_fs_callbacks.o

obj-$(CONFIG_HWDPS) += src/ioctl/hwdps_ioctl.o \
		       src/ioctl/hwdps_ioctl_impl.o \
		       src/ioctl/hwdps_user_key.o

obj-$(CONFIG_HWDPS) += src/policy/hwdps_policy.o

obj-$(CONFIG_HWDPS) += src/tee/hwdps_keyinfo.o \
		       src/tee/base_alg.o \
		       src/tee/hwdps_alg.o \
		       src/tee/hwdps_adapter.o \
		       src/tee/hwdps_tee.o

obj-$(CONFIG_HWDPS) += src/data/hwdps_packages.o \
		       src/data/hwdps_data.o

EXTRA_CFLAGS := -I../../include/ \
		-Iinc/base \
		-Iinc/data \
		-Iinc/fek \
		-Iinc/ioctl \
		-Iinc/policy \
		-Iinc/tee \
		-Idrivers/hisi/tzdriver \
		-I$(srctree)/lib/libc_sec/securec_v2/include

flags += -pipe -Wdate-time -Wfloat-equal -Wformat=2

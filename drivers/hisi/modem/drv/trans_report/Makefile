# balong trans report module
subdir-ccflags-y := -Werror

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/hds

obj-y += trans_report_pkt.o trans_report_cnt.o

obj-$(CONFIG_BALONG_WETH) += trans_report_rc.o

#
#NoC Makefile
#

subdir-ccflags-$(CONFIG_HISI_BB)               += -Idrivers/hisi/mntn/blackbox/
subdir-ccflags-$(CONFIG_HISI_NOC)              += -Idrivers/hisi/mntn/ -Wall -Werror

obj-$(CONFIG_HISI_NOC) +=  dfx_noc.o             \
                           dfx_noc_err_probe.o       \
                           dfx_noc_packet.o      \
                           dfx_noc_transcation.o \
                           dfx_noc_info.o        \
                           dfx_noc_dump.o        \
                           dfx_noc_info_mouse.o \
                           dfx_noc_info_cows.o \
                           dfx_noc_info_capricorn.o \
                           dfx_noc_info_tiger_es.o \
                           dfx_noc_info_tiger.o \
                           dfx_noc_info_aquarius.o \
                           dfx_noc_info_rabbit_es.o \
                           dfx_noc_info_rabbit.o \
                           dfx_noc_info_pisces.o \
                           dfx_noc_get_data_from_dts.o \
                           dfx_noc_info_dragon_cs2.o \

obj-$(CONFIG_HISI_NOC_DBG) += dfx_noc_dbg.o
obj-$(CONFIG_HISI_QIC) +=  dfx_qic.o             \
                           dfx_qic_dump.o        \

# soundwire driver
subdir-ccflags-y += -Wall -Werror

ccflags-y += -Isound/soc/codecs/hisi/
ccflags-y += -Isound/soc/codecs/hisi/fpga_test/
ccflags-y += -Iinclude/linux/hisi/
ccflags-y += -Isound/soc/codecs/hisi/adapter/
ccflags-y += -Isound/soc/codecs/hisi/fpga_test/hi_audio_test

obj-y += soundwire.o
obj-y += soundwire_loop_test.o
obj-y += soundwire_ip_test.o
obj-y += soundwire_extend_test.o
obj-y += soundwire_enum.o
obj-y += soundwire_utils.o
#obj-y += dsp_img.o
obj-y += soundwire_3m_dpcfg.o
obj-y += soundwire_12m_dpcfg.o

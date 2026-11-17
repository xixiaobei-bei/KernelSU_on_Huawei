obj-$(CONFIG_HW_WAUDIO_MODULE) += wifi_audio.o wifi_audio_utils.o wifi_audio_ptp.o
ifeq (v818, $(findstring v818,$(TARGET_DEVICE)))
KBUILD_CFLAGS += -DHONGHU_WIFI_AUDIO_CAST_PCM_DATA
endif
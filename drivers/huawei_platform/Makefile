#=============================================================================
#=============================================================================

obj-$(CONFIG_HW_ANTENNA_BOARD_MATCH)  += antenna_board_match/
obj-$(CONFIG_HUAWEI_AUDIO) += audio/
obj-$(CONFIG_RIL_NETLINK_MODULE)   += riltcp/
obj-$(CONFIG_RIL_SIM_NETLINK_MODULE)   += rilsim/
obj-y   += connectivity/
obj-$(CONFIG_HUAWEI_HW_DEV_DCT)  += devdetect/
#obj-${CONFIG_HUAWEI_DSM} += dsm/
ifneq ($(strip $(OBB_SEPARATE)),true)
obj-$(CONFIG_VIACBP82D_3RD_MODEM) += ext_modem/
endif
obj-y   += inputhub/

obj-$(CONFIG_MAXQ616_POWER_SWITCH) += irda/
#obj-$(CONFIG_LOGGER_EXTEND) += log/
obj-$(CONFIG_HW_STAT_MM) += mm/
obj-$(CONFIG_NFC_DEVICES)   += nfc/
obj-$(CONFIG_HW_WIFI)   += net/
obj-y   += power/
obj-y += hwpower/
obj-$(CONFIG_HUAWEI_DSM) += recovery/
obj-$(CONFIG_HUAWEI_DSM_KIRIN_CHIPSETS) += kirin_chipsets/
obj-$(CONFIG_HUAWEI_SENSORS_INPUT_INFO)   += sensor/
obj-y   += storage/
obj-$(CONFIG_HUAWEI_TS)   += touthscreen/
obj-y   += touch_key/
obj-y   += usb/
obj-$(CONFIG_ANTENNA_BOARDID_GPIO_DETECT) += antenna_board_gpio_match/
obj-$(CONFIG_HUAWEI_VIBRATOR) += vibrator/
obj-$(CONFIG_MADAPT) += madapt/
obj-$(CONFIG_HUAWEI_IO_TRACING) += iotrace/
obj-$(CONFIG_HW_HANDPRESS_SWITCH) += handpress_switch/
obj-$(CONFIG_HW_LED_ALWAYSON) += led_alwayson/
obj-$(CONFIG_HUAWEI_BOOT_TIME) += boottime/
obj-y   += hw_led/
obj-$(CONFIG_DP_AUX_SWITCH) += dp_aux_switch/
obj-$(CONFIG_HW_DP_SOURCE) += dp_source/
obj-$(CONFIG_HUAWEI_VENDOR_EXCEPTION) += vendor_exception/
obj-$(CONFIG_HWDPS) += hwdps/
obj-$(CONFIG_CARD_TRAY_GPIO_DETECT) += card_tray/
obj-$(CONFIG_MEM_INSPECT) += rmeminspect/
obj-$(CONFIG_HUAWEI_MEMINFO_DUMP) += pmeminfo/
obj-y   += hw_peripheral/
obj-y += comb_key/
obj-$(CONFIG_WAKEUP) += wakeup/
obj-$(CONFIG_MULTICHIP_SKYTONE) += multichip_skytone/
obj-$(CONFIG_HISI_EXT_NTC) += thermal/
obj-$(CONFIG_HWSDP) += hwsdp/
obj-$(CONFIG_HUAWEI_BLPWM) += blpwm/

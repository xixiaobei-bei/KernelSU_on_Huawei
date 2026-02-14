mkfilepath := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir :=$(dir $(mkfilepath))

 include $(current_dir)/balong_product_config_drv.mk
 include $(current_dir)/balong_product_config_pam.mk
 include $(current_dir)/balong_product_config_gucas.mk
 include $(current_dir)/balong_product_config_gucnas.mk
 include $(current_dir)/balong_product_config_tlps.mk
 include $(current_dir)/balong_product_config_audio.mk
 include $(current_dir)/balong_product_config_tool.mk
 include $(current_dir)/balong_product_config_version.mk
#ifneq ($(TARGET_BUILD_VARIANT),user)
ifneq ($(choose_variant_modem),user)
 include $(current_dir)/balong_product_config_drv_eng.mk
 include $(current_dir)/balong_product_config_pam_eng.mk
 include $(current_dir)/balong_product_config_gucas_eng.mk
 include $(current_dir)/balong_product_config_gucnas_eng.mk
 include $(current_dir)/balong_product_config_tlps_eng.mk
 include $(current_dir)/balong_product_config_audio_eng.mk
 include $(current_dir)/balong_product_config_tool_eng.mk
 include $(current_dir)/balong_product_config_version_eng.mk
endif
#endif

#Copyright Huawei Technologies Co., Ltd. 1998-2011. All rights reserved.
#This file is Auto Generated 

dtb-y += hi6250/hi6250_udp_noRF_config.dtb
dtb-y += hi6250/hi6250_udp_6555codec_noRF_config.dtb
dtb-y += hi6250/hi6250_udp_DSDS_config.dtb
dtb-y += hi6250/hi6250_udp_6555codec_config.dtb
dtb-y += hi6250/hi6250_hi6250_fpga_config.dtb
dtb-y += hi6250/hi6250_udp_SRLTE_config.dtb
dtb-y += hi6250/hi6250_emulator_config.dtb
dtb-y += hi6250/hi6250_udp_default_config.dtb

targets += hi6250_dtb
targets += $(dtb-y)

# *.dtb used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
hi6250_dtb: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtb

clean-files := *.dtb

#end of file

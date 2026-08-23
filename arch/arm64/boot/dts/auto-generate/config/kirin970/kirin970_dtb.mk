#Copyright Huawei Technologies Co., Ltd. 1998-2011. All rights reserved.
#This file is Auto Generated 

dtb-y += kirin970/kirin970_udp_cs_EVB3_PILOT_config.dtb
dtb-y += kirin970/kirin970_udp_cs_config.dtb
dtb-y += kirin970/kirin970_udp_cs_EVB1_VB_config.dtb
dtb-y += kirin970/kirin970_udp_cs_EVB3_config.dtb
dtb-y += kirin970/kirin970_udp_cs_EVB7_config.dtb

targets += kirin970_dtb
targets += $(dtb-y)

# *.dtb used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
kirin970_dtb: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtb

clean-files := *.dtb

#end of file

#Copyright Huawei Technologies Co., Ltd. 1998-2011. All rights reserved.
#This file is Auto Generated 

dtb-y += kirin970/kirin970_udp_cs_EVB3_PILOT_overlay.dtbo
dtb-y += kirin970/kirin970_udp_cs_overlay.dtbo
dtb-y += kirin970/kirin970_udp_ET_overlay.dtbo
dtb-y += kirin970/kirin970_udp_cs_EVB1_VB_overlay.dtbo
dtb-y += kirin970/kirin970_udp_cs_EVB3_overlay.dtbo
dtb-y += kirin970/kirin970_udp_cs_EVB7_overlay.dtbo

targets += kirin970_dtbo
targets += $(dtb-y)

# *.dtbo used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
kirin970_dtbo: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtbo

clean-files := *.dtbo

#end of file

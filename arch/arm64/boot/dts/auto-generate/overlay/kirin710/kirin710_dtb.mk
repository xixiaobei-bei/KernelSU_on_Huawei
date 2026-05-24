#Copyright Huawei Technologies Co., Ltd. 1998-2011. All rights reserved.
#This file is Auto Generated 

dtb-y += kirin710/kirin710_udp_pop_codec_out_hi6363_14_overlay.dtbo
dtb-y += kirin710/kirin710_udp_pop_codec_in_hi6353_01_overlay.dtbo
dtb-y += kirin710/kirin710_udp_pop_codec_in_hi6363_04_overlay.dtbo
dtb-y += kirin710/kirin710_udp_default_overlay.dtbo
dtb-y += kirin710/kirin710_udp_pop_codec_out_default_10_overlay.dtbo
dtb-y += kirin710/kirin710_udp_pop_codec_out_hi6353_11_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_out_default_30_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_out_hi6363_34_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_out_hi6353_31_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_in_default_20_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_in_hi6353_21_overlay.dtbo
dtb-y += kirin710/kirin710_udp_pop_codec_in_hi6353_02_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_in_hi6363_24_overlay.dtbo
dtb-y += kirin710/kirin710_udp_fccsp_codec_in_hi6353_22_overlay.dtbo

targets += kirin710_dtbo
targets += $(dtb-y)

# *.dtbo used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
kirin710_dtbo: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtbo

clean-files := *.dtbo

#end of file

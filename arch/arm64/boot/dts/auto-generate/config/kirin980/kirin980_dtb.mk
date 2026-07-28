#Copyright Huawei Technologies Co., Ltd. 1998-2011. All rights reserved.
#This file is Auto Generated 

dtb-y += kirin980/kirin980_emulator_cs_config.dtb
dtb-y += kirin980/kirin980_chip_es_config.dtb
dtb-y += kirin980/kirin980_fpga_cs_config.dtb
dtb-y += kirin980/kirin980_chip_cs_config.dtb

targets += kirin980_dtb
targets += $(dtb-y)

# *.dtb used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
kirin980_dtb: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtb

clean-files := *.dtb

#end of file

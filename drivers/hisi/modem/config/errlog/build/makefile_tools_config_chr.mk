# Created by Jeffery.zhai/199916 2012.03.21

#***********************************************************#
# include the define at the top
#***********************************************************#
include $(BALONG_TOPDIR)/build/scripts/make_base.mk

# process
.PHONY: $(TOOLS_CONFIG_CHR_MODULES)
tools_config_chr_tlas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/tl/TOOLS_CONFIG/makefile_tools_config_chr_tlas.mk
tools_config_chr_tlnas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/tl/TOOLS_CONFIG/makefile_tools_config_chr_tlnas.mk
tools_config_chr_gas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_gas.mk
tools_config_chr_gucnas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_gucnas.mk
tools_config_chr_cttf:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/cdma/tools_config/makefile_tools_config_chr_cttf.mk
tools_config_chr_cas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/cdma/tools_config/makefile_tools_config_chr_cas.mk
tools_config_chr_was:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_was.mk

# process
.PHONY: $(TOOLS_CONFIG_CHR_MODULES_CLEAN)
clean-tools_config_chr_tlas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/tl/TOOLS_CONFIG/makefile_tools_config_chr_tlas.mk clean
clean-tools_config_chr_tlnas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/tl/TOOLS_CONFIG/makefile_tools_config_chr_tlnas.mk clean
clean-tools_config_chr_gas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_gas.mk clean
clean-tools_config_chr_gucnas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_gucnas.mk clean
clean-tools_config_chr_cttf:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/cdma/tools_config/makefile_tools_config_chr_cttf.mk clean
clean-tools_config_chr_cas:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/cdma/tools_config/makefile_tools_config_chr_cas.mk clean
clean-tools_config_chr_was:
	$(MAKE)  -f $(BALONG_TOPDIR)/modem/ps/build/gu/TOOLS_CONFIG/makefile_tools_config_chr_was.mk clean

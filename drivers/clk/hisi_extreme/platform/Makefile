ifeq ($(CONFIG_BALT_IP_PLATFORM),y)
obj-$(CONFIG_HISI_CLK)                  += clk-balt.o
obj-$(CONFIG_HISI_CLK)                  += clk-balt-cs.o
endif

ifeq ($(CONFIG_PHOE_IP_PLATFORM),y)
ifeq ($(product_type),cdc)
obj-$(CONFIG_HISI_CLK)                  += clk-phoe-cs.o
endif
endif

ifeq ($(CONFIG_BURB_IP_PLATFORM),y)
obj-$(CONFIG_HISI_CLK)                  += clk-bur.o
endif

ifeq ($(CONFIG_CHAR_IP_PLATFORM),y)
obj-$(CONFIG_HISI_CLK)                  += clk-char.o
endif

ifeq ($(CONFIG_LEX_IP_PLATFORM),y)
obj-$(CONFIG_HISI_CLK)                  += clk-lex.o
endif

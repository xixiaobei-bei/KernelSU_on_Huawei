ccflags-$(CONFIG_LPCPU_MAX_POWER)  += -I$(srctree)/drivers/hisi/max_power
obj-$(CONFIG_LPCPU_MAX_POWER)		+= max_power_debug.o
obj-$(CONFIG_LPCPU_MAX_POWER)		+= max_power_stub.o

ccflags-$(CONFIG_LPCPU_MAX_POWER) += -I$(srctree)/drivers/hisi/max_power/$(shell echo $(TARGET_BOARD_PLATFORM) | rev)
obj-$(CONFIG_LPCPU_MAX_POWER) += \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/ananke_max_power.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/ananke_max_power_wfe.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/max_power.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/hera_max_power.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/hera_max_power_wfe.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/hercules_max_power.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/hercules_max_power_wfe.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/deimos_max_power.o \
	$(shell echo $(TARGET_BOARD_PLATFORM) | rev)/deimos_max_power_wfe.o

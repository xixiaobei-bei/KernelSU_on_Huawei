#
# Makefile for the vl53L0 drivers.
#

# Each configuration option enables a list of files.

ccflags-y += -I$(src)/vi5300

obj-y += vi5300.o

# module
vi5300-objs := laser_vi5300_module.o

# vi5300
vi5300-objs += vi5300/vi5300_module.o vi5300/vi5300_platform.o vi5300/vi5300_api.o vi5300/vi5300_firmware.o



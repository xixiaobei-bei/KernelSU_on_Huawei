# Makefile for the key driver.
#
obj-$(CONFIG_STEP_MOTOR) += drv8846.o
obj-$(CONFIG_STEP_MOTOR) += aw8646.o

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DSTEP_MOTOR_DEBUG
endif

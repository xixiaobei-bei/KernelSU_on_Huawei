ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

subdir-ccflags-y := -Werror -I$(srctree)/drivers/hisi/modem/include/adrv/
obj-y += ringbuffer.o console.o virtshell.o cshell_port.o uart_dev.o con_platform.o cshell_logger.o

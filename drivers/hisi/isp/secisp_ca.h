/*
 * TEEC for secisp
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _LOAD_SECISP_H_
#define _LOAD_SECISP_H_

#include <linux/hisi/hisi_ion.h>
#include <dynamic_mem.h>
#include <linux/device.h>
#include <teek_client_id.h>

#ifdef __cplusplus
extern "C"
{
#endif
/* info */
#define secisp_print_err(fmt, ...)     (printk(KERN_ERR "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define secisp_print_info(fmt, ...)    (printk(KERN_INFO "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define secisp_print_debug(fmt, ...)   (printk(KERN_DEBUG "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
/* CA info */
#define TEE_SERVICE_SECISP_ID           TEE_SERVICE_SECISP/* Tzdriver UUID for secisp */
//#define TEE_SECISP_PACKAGE_NAME         "/system/bin/isptest"
//#define TEE_SECISP_UID                  0
#define TEE_SECISP_PACKAGE_NAME         "/vendor/bin/CameraDaemon"
#define TEE_SECISP_UID                  1013

/* CA cmd */
enum SVC_SECISP_CMD_ID {
	TEE_SECISP_CMD_IMG_DISRESET  = 0,
	TEE_SECISP_CMD_RESET         = 1,
	TEE_SECISP_MEM_CFG_AND_MAP   = 2,
	TEE_SECISP_MEM_CFG_AND_UNMAP = 3,
	TEE_SECISP_STATIC_MEM_MAP    = 4,
	TEE_SECISP_STATIC_MEM_UNMAP  = 5,
	TEE_SECISP_DYNAMIC_MEM_MAP   = 6,
	TEE_SECISP_DYNAMIC_MEM_UNMAP = 7,
	TEE_SECISP_CMD_MAX
};
/* cfg info */
#define SECISP_SEC      1
#define SECISP_NSEC     0

enum secisp_mem_type {
	SECISP_SEC_PAGE  = 1,
	SECISP_TEXT,
	SECISP_DATA,
	SECISP_ISPSEC_POOL,
	SECISP_SEC_POOL,
	SECISP_DYNAMIC,
	SECORB_SEC_MEM,
	SECISP_SHARE,
	SECISP_RDR,
	SECISP_VQ,
	SECISP_VR0,
	SECISP_VR1,
	SECISP_MEM_SET_SEC,
	SECISP_MAX_TYPE
};

typedef struct isp_ion_mem_type {
	unsigned int type;
	unsigned int da;
	unsigned int size;
	unsigned int prot;
	unsigned int sec_flag;/* SEC or NESC*/
	int sharefd;
	u64 pa;
}secisp_mem_info;

/* secisp CA API */
extern int TEEK_secisp_close(void);
extern int TEEK_secisp_open(void);
extern int TEEK_secisp_disreset(int sharefd, unsigned int size);
extern int TEEK_secisp_reset(void);
extern int TEEK_secisp_mem_map(secisp_mem_info *buffer, unsigned int *iova);
extern int TEEK_secisp_mem_unmap(secisp_mem_info *buffer);
extern int TEEK_secisp_static_mem_map(secisp_mem_info *buffer, unsigned int *iova);
extern int TEEK_secisp_static_mem_unmap(secisp_mem_info *buffer);
extern int TEEK_secisp_dynamic_mem_map(secisp_mem_info *buffer, unsigned int *iova, struct device * dev);
extern int TEEK_secisp_dynamic_mem_unmap(secisp_mem_info *buffer, struct device * dev);
extern void TEEK_secisp_ca_probe(void);
extern void TEEK_secisp_ca_remove(void);

#ifdef __cplusplus
}
#endif
#endif

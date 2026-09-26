/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef HISI_AP_DRV_H
#define HISI_AP_DRV_H
#include <linux/module.h>
#include <linux/device.h>
/*************************************************************************
*
*   启动/加载/复位/校验
*
*************************************************************************/

#define PARTITION_MODEM_IMAGE_NAME       "modem"
#define PARTITION_MODEM_DSP_NAME         "modem_dsp"
#define PARTITION_MODEM_NVDLOAD_NAME     "modemnvm_update"
#define PARTITION_MODEM_NVDLOAD_CUST_NAME "modemnvm_cust"
#define PARTITION_MODEM_NVDEFAULT_NAME   "modemnvm_factory"
#define PARTITION_MODEM_NVBACKUP_NAME    "modemnvm_backup"
#define PARTITION_MODEM_NVSYS_NAME       "modemnvm_system"
#define PARTITION_MODEM_NVIMG_NAME       "modemnvm_img"
#define PARTITION_MODEM_NVPATCH_NAME     "modem_patch_nv"
#define PARTITION_MODEM_LOG_NAME         "modem_om"
#define PARTITION_MODEM_DTB_NAME	 "modem_dtb"
#define PARTITION_PTN_VRL_P_NAME 	 "vrl"
#define PARTITION_PTN_VRL_M_NAME	 "vrl_backup"

#define BSP_RESET_NOTIFY_REPLY_OK 		   0
#define BSP_RESET_NOTIFY_SEND_FAILED      -1
#define BSP_RESET_NOTIFY_TIMEOUT          -2

typedef enum
{
	BSP_START_MODE_CHARGING = 0,
	BSP_START_MODE_NORMAL,
	BSP_START_MODE_UPDATE,
	BSP_START_MODE_BUILT
} BSP_START_MODE_E;

typedef enum {
	BSP_CCORE = 0,
	BSP_HIFI,
	BSP_BBE,
	BSP_CDSP,
	BSP_CCORE_TAS,
	BSP_CCORE_WAS,
	BSP_CCORE_CAS,
	BSP_BUTT
} BSP_CORE_TYPE_E;

/*****************************************************************************
* 函 数 名  : flash_find_ptn_s
*
* 功能描述  : 此接口根据分区名返回分区信息, 是新安全接口，强烈建议用新接口flash_find_ptn_s
*
* 输入参数  : ptn_name 分区名 ，pblkname_length 调用者给pblkname分配内存的长度
*
* 输出参数  : pblkname 返回分区路径信息，如/dev/block/by-name/xxx
*
* 返 回 值  : 0 成功 -1 失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int flash_find_ptn_s(const char *ptn_name, char *pblkname,
						unsigned int pblkname_length);

/*****************************************************************************
* 函 数 名  : flash_find_ptn
*
* 功能描述  : 根据分区名返回分区信息, 此老接口有溢出风险，强烈建议用新接口flash_find_ptn_s
*
* 输入参数  : ptn_name 分区名
*
* 输出参数  : pblkname 返回分区路径信息，如/dev/block/by-name/xxx
*
* 返 回 值  : 0 成功 -1 失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int flash_find_ptn(const char *ptn_name, char *pblkname);

/*****************************************************************************
* 函 数 名  : bsp_need_loadmodem
*
* 功能描述  : 获取是否需要启动modem
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 1 需要启动
			  0 不需要启动
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_need_loadmodem(void);

/*****************************************************************************
* 函 数 名  : bsp_reset_loadimage
*
* 功能描述  : Modem和HIFI 镜像加载
*
* 输入参数  : ecoretype 见BSP_CORE_TYPE_E
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_reset_loadimage (BSP_CORE_TYPE_E ecoretype);

/*
 * Function name:bsp_reset_core_notify.
 * Description:notify the remote processor MODEM is going to reset.
 * Parameters:
 *      @ ecoretype: the core to be notified.
 *      @ cmdtype: to send to remote processor.
 *      @ timeout_ms: max time to wait, ms.
 *      @ retval: the ack's value get from the remote processor.
 *Called in:
 *      @ modem is going to reset. <reset_balong.c>
 *Return value:
 *      @ BSP_RESET_NOTIFY_REPLY_OK-->the remote processor give response in time.
 *      @ BSP_RESET_NOTIFY_SEND_FAILED-->the parameter is wrong or other rproc_sync_send's self error.
 *      @ BSP_RESET_NOTIFY_TIMEOUT-->after wait timeout_ms's time, the remote processor give no response.
*/
int bsp_reset_core_notify(BSP_CORE_TYPE_E ecoretype, unsigned int cmdtype, unsigned int timeout_ms, unsigned int *retval);



/*************************************************************************
*
*   RDR/Coresight
*
*************************************************************************/

#define STR_EXCEPTIONDESC_MAXLEN	48

/*与AP 约定定义成如下格式，如果变更，有AP变更*/
typedef enum
{
    CP_S_MODEMDMSS     = 0x70,
    CP_S_MODEMNOC      = 0x71,
    CP_S_MODEMAP       = 0x72,
    CP_S_EXCEPTION     = 0x73,
    CP_S_RESETFAIL     = 0x74,
    CP_S_NORMALRESET   = 0x75,
    CP_S_RILD_EXCEPTION= 0x76,
    CP_S_3RD_EXCEPTION = 0x77,
    CP_S_DRV_EXC       = 0x78,
    CP_S_PAM_EXC       = 0x79,
    CP_S_GUAS_EXC      = 0x7a,
    CP_S_CTTF_EXC      = 0x7b,
    CP_S_CAS_CPROC_EXC = 0x7c,
    CP_S_GUDSP_EXC     = 0x7d,
    CP_S_TLPS_EXC      = 0x7e,
    CP_S_TLDSP_EXC     = 0x7f,
    CP_S_CPHY_EXC      = 0x80,
    CP_S_GUCNAS_EXC    = 0x81,
} EXCH_SOURCE;

enum RDR_SAVE_LOG_FLAG {
    RDR_SAVE_DMESG = (0x1 << 0),
    RDR_SAVE_CONSOLE_MSG = (0x1 << 1),
    RDR_SAVE_BL31_LOG = (0x1 << 2),
    RDR_SAVE_LOGBUF = (0x1 << 3),
};

struct list_head_rdr {
	struct list_head_rdr *next, *prev;
};
typedef void (*rdr_e_callback)(unsigned int, void*);

/*
 *   struct list_head_rdr   e_list;
 *   u32 modid,			    exception id;
 *		if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end,		    can register modid region. [modid~modid_end];
		need modid_end >= modid,
 *		if modid_end equal 0, will be register modid only,
		but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,	exception process priority
 *   u32 reboot_priority,	exception reboot priority
 *   u64 save_log_mask,		need save log mask
 *   u64 notify_core_mask,	need notify other core mask
 *   u64 reset_core_mask,	need reset other core mask
 *   u64 from_core,		    the core of happen exception
 *   u32 reentrant,		    whether to allow exception reentrant
 *   u32 exce_type,		    the type of exception
 *   char* from_module,		    the module of happen excption
 *   char* desc,		        the desc of happen excption
 *   u32 save_log_flags,		set bit 1 to save the log(dmsg, console, bl31log)
 *   rdr_e_callback callback,	will be called when excption has processed.
 *   u32 reserve_u32;		reserve u32
 *   void* reserve_p		    reserve void *
 */
struct rdr_exception_info_s {
	struct list_head_rdr e_list;	/*list_head_rdr instead of list_head  to solve the confliction between <linux/types.h>*/
	unsigned int	e_modid;
	unsigned int	e_modid_end;
	unsigned int	e_process_priority;
	unsigned int	e_reboot_priority;
	unsigned long long	e_notify_core_mask;
	unsigned long long	e_reset_core_mask;
	unsigned long long	e_from_core;
	unsigned int	e_reentrant;
	unsigned int    e_exce_type;
	unsigned int    e_exce_subtype;
	unsigned int    e_upload_flag;
	unsigned char	e_from_module[MODULE_NAME_LEN];
	unsigned char	e_desc[STR_EXCEPTIONDESC_MAXLEN];
	unsigned int	e_save_log_flags;
	unsigned int	e_reserve_u32;
	void*	e_reserve_p;
	rdr_e_callback e_callback;

};

enum UPLOAD_FLAG {
    RDR_UPLOAD_YES = 0xff00fa00,
    RDR_UPLOAD_NO
};
enum REENTRANT {
    RDR_REENTRANT_ALLOW = 0xff00da00,
    RDR_REENTRANT_DISALLOW
};

enum REBOOT_PRI {
    RDR_REBOOT_NOW      = 0x01,
    RDR_REBOOT_WAIT,
    RDR_REBOOT_NO,
    RDR_REBOOT_MAX
};
enum CORE_LIST {
    RDR_AP      = 0x1,
    RDR_CP      = 0x2,
    RDR_HIFI    = 0x8,
    RDR_LPM3    = 0x10,
    RDR_MODEMAP = 0x200,
};
enum PROCESS_PRI {
    RDR_ERR      = 0x01,
    RDR_WARN,
    RDR_OTHER,
    RDR_PPRI_MAX
};


typedef void (*pfn_cb_dump_done)(unsigned int modid, unsigned long long coreid);
typedef void (*pfn_dump)(unsigned int modid, unsigned int etype, unsigned long long coreid, char* logpath, pfn_cb_dump_done fndone);
typedef void (*pfn_reset)(unsigned int modid, unsigned int etype, unsigned long long coreid);

struct rdr_module_ops_pub {
    pfn_dump    ops_dump;
    pfn_reset   ops_reset;
};

struct rdr_register_module_result {
    unsigned long long   log_addr;
    unsigned int         log_len;
    unsigned long long   nve;
};


#ifdef CONFIG_HISI_BB
/*****************************************************************************
* 函 数 名  : rdr_register_exception
*
* 功能描述  : RDR异常注册接口
*
* 输入参数  : rdr_exception_info_s
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理失败
			  >0 e_modid_end
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int rdr_register_exception (struct rdr_exception_info_s* e);

/*****************************************************************************
* 函 数 名  : rdr_register_module_ops
*
* 功能描述  : 注册异常处理函数
*
* 输入参数  : coreid  异常核ID
			  ops     异常处理函数与reset函数
*             retinfo 注册返回信息
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub* ops, struct rdr_register_module_result* retinfo);


/*****************************************************************************
* 函 数 名  : rdr_system_error
*
* 功能描述  : 软件异常记录接口，完成异常文件记录之后主动复位单板
*
* 输入参数  :
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/
void rdr_system_error(unsigned int modid, unsigned int arg1, unsigned int arg2);
#else
static inline u32 rdr_register_exception(struct rdr_exception_info_s* e){ return 0;}
static inline int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub* ops,
				struct rdr_register_module_result* retinfo){ return -1; }
static inline void rdr_system_error(unsigned int modid, unsigned int arg1, unsigned int arg2) {}
#endif

/*****************************************************************************
* 函 数 名  : rdr_syserr_process_for_ap
*
* 功能描述  : 当已注册到rdr上的异常发生时，需要调用此接口触发保存log和reset流程。此接口供AP使用。
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/
void rdr_syserr_process_for_ap(unsigned int modid, unsigned long long arg1, unsigned long long arg2);
/*****************************************************************************
* 函 数 名  : is_reboot_reason_from_modem
*
* 功能描述  : 判断上次整机重启是否是由Modem引起
*
* 输入参数  :
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/

unsigned int is_reboot_reason_from_modem(void);
/*****************************************************************************
* 函 数 名  : rdr_get_reboot_logpath
*
* 功能描述  : 整机重启后如果modem需要保存log，获取保存log的路径
* 输入参数  :
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/

char *rdr_get_reboot_logpath(void);


/*
 * eFuse
 */

/*
 * FUNC DESCRIPTION: get_efuse_dieid_value
 *
 * INPUT PARAMETERS:
 *   unsigned char *buf;     store the dieid read value
 *   unsigned int  size;     defined in bytes, and requires a minimum of 20 byte
 *   unsigned int  timeout;  overrun time, usually 1000 ms, 0 means waiting unless
 *                           the func finish your request successfully.
 *
 * OUTPUT PARAMETERS:
 *   buf            is updated
 *
 * RETURN VALUE:
 *    The func returns 0 if it finish your request successfully,
 *    otherwise, it returns less than 0 which means an error has occurred.
 */
int get_efuse_dieid_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/*
 * FUNC DESCRIPTION: get_efuse_chipid_value
 *
 * INPUT PARAMETERS:
 *   unsigned char *buf;     store the chipid read value
 *   unsigned int  size;     defined in bytes, and requires a minimum of 8 byte
 *   unsigned int  timeout;  overrun time, usually 1000 ms, 0 means waiting unless
 *                           the func finish your request successfully.
 *
 * OUTPUT PARAMETERS:
 *    buf            is updated
 *
 * RETURN VALUE:
 *    The func returns 0 if it finish your request successfully,
 *    otherwise, it returns less than 0 which means an error has occurred.
 */
int get_efuse_chipid_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/*
 * FUNC DESCRIPTION: get_efuse_kce_value
 *
 * INPUT PARAMETERS:
 *   unsigned char *buf;     store the kce efuse read value
 *   unsigned int  size;     defined in bytes, and requires a minimum of 16 byte
 *   unsigned int  timeout;  overrun time, usually 1000 ms, 0 means waiting unless
 *                           the func finish your request successfully.
 *
 * OUTPUT PARAMETERS:
 *    buf            is updated
 *
 * RETURN VALUE:
 *    The func returns 0 if it finish your request successfully,
 *    otherwise, it returns less than 0 which means an error has occurred.
 */
int get_efuse_kce_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/*
 * FUNC DESCRIPTION: set_efuse_kce_value
 *
 * INPUT PARAMETERS:
 *   unsigned char *buf;     store the kce efuse you want to write
 *   unsigned int  size;     defined in bytes, and requires a minimum of 16 byte
 *   unsigned int  timeout;  overrun time, usually 1000 ms, 0 means waiting unless
 *                           the func finish your request successfully.
 *
 * RETURN VALUE:
 *    The func returns 0 if it finish your request successfully,
 *    otherwise, it returns less than 0 which means an error has occurred.
 */
int set_efuse_kce_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/*
 * FUNC DESCRIPTION: get_efuse_avs_value
 *
 * INPUT PARAMETERS:
 *   unsigned char *buf;     store the avs efuse read value
 *   unsigned int  size;     defined in bytes, and requires a minimum of 1 byte
 *   unsigned int  timeout;  overrun time, usually 1000 ms, 0 means waiting unless
 *                           the func finish your request successfully.
 *
 * OUTPUT PARAMETERS:
 *    buf            is updated
 *
 * RETURN VALUE:
 *    The func returns 0 if it finish your request successfully,
 *    otherwise, it returns less than 0 which means an error has occurred.
 */
int get_efuse_avs_value(unsigned char *buf, unsigned int size, unsigned int timeout);

/*************************************************************************
*
*   USB
*
*************************************************************************/
typedef void (*USB_ENABLE_CB_T)(void);
typedef void (*USB_DISABLE_CB_T)(void);

/*****************************************************************************
* 函 数 名  : bsp_acm_open
*
* 功能描述  : 打开设备
*
* 输入参数  : dev_id 要打开的设备id
*
* 输出参数  : 无
*
* 返 回 值  : 非0 返回的handle
			  NULL 错误
*
* 其它说明  : kernel
*
*****************************************************************************/
void *bsp_acm_open(unsigned int dev_id);

/*****************************************************************************
* 函 数 名  : bsp_acm_close
*
* 功能描述  : 关闭设备
*
* 输入参数  : 要关闭的设备handle
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_close(void *handle);

/*****************************************************************************
* 函 数 名  : bsp_acm_write
*
* 功能描述  : 数据写
*
* 输入参数  : handle 设备handle
			  buf  待写入的数据
			  size 代写入的数据大小
*
* 输出参数  : 无
*
* 返 回 值  : >0  处理成功的数据大小
			  <=0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_write(void *handle, void *buf, unsigned int size);

/*****************************************************************************
* 函 数 名  : bsp_acm_read
*
* 功能描述  : 数据写
*
* 输入参数  : handle 设备handle
			  size 代写入的数据大小
*
* 输出参数  : buf  待返回的数据
*
* 返 回 值  : >0  处理成功的数据大小
			  <=0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_read(void *handle, void *buf, unsigned int size);

/*****************************************************************************
* 函 数 名  : bsp_acm_ioctl
*
* 功能描述  : 根据命令码进行设置
*
* 输入参数  : handle 设备handle
			  cmd 命令码
			  para 命令参数
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_ioctl(void *handle, unsigned int cmd, void *para);

/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiEnableCB
*
* 功能描述  : 注册usb枚举完成回调
*
* 输入参数  : pFunc 回调指针
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiEnableCB(USB_ENABLE_CB_T pFunc);

/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiDisableCB
*
* 功能描述  : 注册usb拔出完成回调
*
* 输入参数  : pFunc 回调指针
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiDisableCB(USB_DISABLE_CB_T pFunc);

/*************************************************************************
*
*   PMU
*
*************************************************************************/


typedef  void   (*PMU_OCP_CALLBACK)( const char * name);
/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiDisableCB
*
* 功能描述  : 注册usb拔出完成回调
*
* 输入参数  : name OCP产生的电源名称，见pmu_mntn DTS定义
			  pFunc
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int pmu_ocp_event_reg (const char * name, PMU_OCP_CALLBACK pFunc);

enum EDITION_KIND{
    EDITION_USER            = 1,
    EDITION_INTERNAL_BETA   = 2,
    EDITION_OVERSEA_BETA    = 3,
    EDITION_MAX
};
#ifdef CONFIG_HISI_BB
/*
 * func name: bbox_check_edition
 * func args:
 *   void
 * return:
 *   unsigned int:	return edition information
 *				0x01		User
 *				0x02		Internal BETA
 *				0x03		Oversea BETA
 */
unsigned int bbox_check_edition(void);
#else
static inline unsigned int bbox_check_edition(void){return EDITION_USER;}
#endif

/* hisi_pmic_special_ocp_register需要加入接口管控  */


#ifdef CONFIG_DRM_DRIVER
int atfd_hisi_service_access_register_smc(unsigned long long main_fun_id,
                                          unsigned long long buff_addr_phy,
                                          unsigned long long data_len,
                                          unsigned long long sub_fun_id);
#else
static inline int atfd_hisi_service_access_register_smc(unsigned long long main_fun_id,
                                          unsigned long long buff_addr_phy,
                                          unsigned long long data_len,
                                          unsigned long long sub_fun_id){return 0;}
#endif

void ipf_get_waking_pkt(void* data, unsigned int len);

/*****************************************************************************
* function     : get_kernel_build_time
* description  : get build date and build time of kernel
* input        : builddate: buffer for get build date
*                dtlen    : length of builddate buffer
*                buildtime: buffer for get build time
*                tmlen    : length of buildtime buffer
* output       : builddate : kernel build date string
*                buildtime : kernel build time string
* ret value  : 0  successfull
*               <0  failed to get date&time
*****************************************************************************/
int get_kernel_build_time(char* builddate, int dtlen, char* buildtime, int tmlen);


/********sim hardware service start************/
/*
status=1 means plug out;
status=0 means plug in;
*/
#define STATUS_PLUG_IN 0
#define STATUS_PLUG_OUT 1

#define MODULE_SD  0
#define MODULE_SIM  1

/*****************************************************************************
* function     : get_sd2jtag_status
* description  : get sd2jtag status
* input        : void
* output       : void
* ret value  : true -- sd2jtag enabled
*                  false -- sd2jtag disabled
*****************************************************************************/
int get_sd2jtag_status(void);

/*****************************************************************************
* function     : get_card1_type
* description  : get card1 type
* input        : void
* output       : void
* ret value  : STATUS_SIM, STATUS_SD, STATUS_NO_CARD, STATUS_SD2JTAG
*****************************************************************************/
u8 get_card1_type(void);

/*****************************************************************************
* function     : sd_sim_detect_run
* description  : run sd sim detect action
* input        : dw_mci_host      mci_host
                    status                in or out
                    current_module  sim or sd
                    need_sleep        true or false
* output       : void
* ret value  : STATUS_PLUG_IN, STATUS_PLUG_OUT
*****************************************************************************/
int sd_sim_detect_run(void *dw_mci_host, int status, int current_module, int need_sleep);

/*****************************************************************************
* function     : hisi_pmic_reg_read
* description  : read pmic register
* input        : addr      pmic register addr
* output       : void
* ret value  : value of addr
*****************************************************************************/
unsigned int hisi_pmic_reg_read(int addr);

/*****************************************************************************
* function     : hisi_pmic_reg_write
* description  : pmic reg write
* input        : addr      pmic register addr
                    val        value to write
* ret value  : void
*****************************************************************************/
void hisi_pmic_reg_write(int addr, int val);

/*****************************************************************************
* function     : hisi_sub_pmic_reg_read
* description  : read sub pmic register
* input        : addr      pmic register addr
* output       : void
* ret value  : value of addr
*****************************************************************************/
unsigned int hisi_sub_pmic_reg_read(int addr);

/*****************************************************************************
* function     : hisi_sub_pmic_reg_write
* description  : sub pmic reg write
* input        : addr      pmic register addr
                    val        value to write
* ret value  : void
*****************************************************************************/
void hisi_sub_pmic_reg_write(int addr, int val);

/**
 * @brief sim卡硬件处理类型
 */
typedef enum {
    SIM_HW_CARD_TYPE_INQUIRE=0x11, /**<卡类型查询*/
    SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE =0x12, /**<ESIM时实卡插入查询*/
    SIM_HW_POWER_ON_CLASS_C_REQUEST = 0x21, /**<请求设置1.8V电压*/
    SIM_HW_POWER_ON_CLASS_B_REQUEST = 0x22, /**<请求设置3.0V电压*/
    SIM_HW_POWER_OFF_REQUEST = 0x23, /**<请求关闭电源*/
    SIM_HW_DEACTIVE_NOTIFIER= 0x31, /**<去激活通知*/
    SIM_HW_SET_ESIM_SWITCH=0x41, /**<切换ESIM*/
    SIM_HW_SET_USIM_SWITCH=0x42, /**<切换USIM*/
    SIM_HW_OP_TYPE_BUTT
} sci_hw_op_type_e;

/**
 * @brief SIM卡硬件处理结果
 */
typedef enum {
    SIM_HW_OP_RESULT_SUCC=0x0, /**<操作成功*/
    SIM_HW_HOTPLUG_DTS_ERR,    /**<热插拔DTS读取失败*/
    SIM_HW_HOTPLUG_DET_ERR,    /**<热插拔DET管脚设置失败*/
    SIM_HW_HOTPLUG_HPD_ERR,    /**<热插拔HPD配置失败*/
    SIM_HW_HOTPLUG_STATE_ERR,  /**<热插拔状态初始化失败*/
    SIM_HW_HOTPLUG_HPD_IRQ_FAIL,  /**<热插拔HPD中断注册失败*/
    SIM_HW_INVALID_INQUIRE_MSG=0x100, /**<非法查询命令*/
    SIM_HW_CARD_TYPE_SIM=0x101,  /**<卡类型为SIM*/
    SIM_HW_CARD_TYPE_MUXSIM,  /**<卡类型为共卡槽SIM*/
    SIM_HW_CARD_TYPE_SD,  /**<卡类型为SD*/
    SIM_HW_CARD_TYPE_JTAG,  /**<卡类型为JTAG*/
    SIM_HW_CARD_TYPE_FAIL,  /**<卡类型查询失败*/
    SIM_HW_CARD_EXIST_WHILE_ESIM, /**<ESIM时存在实体卡*/
    SIM_HW_CARD_NO_EXIST_WHILE_ESIM, /**<ESIM时不存在实体卡*/
    SIM_HW_SET_GPIO_FAIL,  /**<设置GPIO失败 */
    SIM_HW_INVALID_POWER_MSG=0x200, /**<非法上电消息*/
    SIM_HW_POWER_ON_SUCC=0x201,  /**<打开电源成功*/
    SIM_HW_POWER_ON_FAIL,  /**<打开电源失败*/
    SIM_HW_POWER_OFF_SUCC,  /**<关闭电源成功*/
    SIM_HW_POWER_OFF_FAIL,  /**<关闭电源失败*/
    SIM_HW_REQUESR_PMU_FAIL, /**<请求PMU失败*/
    SIM_HW_SET_VOLTAGE_FAIL, /**<设置电压失败*/
    SIM_HW_INVALID_NOTIFIER_MSG=0x300, /**<非法通知消息*/
    SIM_HW_DEACTIVE_SUCC = 0x301,  /**<去激活成功*/
    SIM_HW_DEACTIVE_FAIL,  /**<去激活失败*/
    SIM_HW_INVALID_SWITCH_MSG=0x400, /**<非法切换消息*/
    SIM_HW_SET_ESIM_SUCC=0x401,  /**<切换ESIM状态成功*/
    SIM_HW_SET_ESIM_FAIL,  /**<切换ESIM状态失败*/
    SIM_HW_SET_USIM_SUCC,  /**<切换USIM状态成功*/
    SIM_HW_SET_USIM_FAIL,  /**<切换USIM状态失败*/
    SIM_HW_REQUEST_RESULT_BUTT /**<BUTT*/
} sci_hw_op_result_e;


/**
 * @brief 本接口用于SIM卡硬件适配初始化
 *
 * @par 描述:
 * 本接口用于SIM卡硬件适配初始化
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[in]  dev 当前使用的设备
 *
 * @retval 0:  初始化成功。
 * @retval 非0：初始化失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int sim_hwadp_init(u32 sim_id, struct device *dev);

/**
 * @brief 本接口用于SIM卡硬件适配消息处理
 *
 * @par 描述:
 * 本接口用于SIM卡硬件适配消息处理
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[in]  msg_id 消息类型，sci_hw_op_type_e
 *
 * @retval 消息处理结果，sci_hw_op_result_e
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see sci_hw_op_type_e, sci_hw_op_result_e
 */
sci_hw_op_result_e sim_hwadp_handler(u32 sim_id, sci_hw_op_type_e msg_id);

/**
 * @brief sim卡硬件调试信息
 */
struct sim_hw_debug_info {
    u32 iomux_status[3]; /**<当前iomux状态 */
    u32 power_status; /**<当前电源状态 */
    u32 event_len; /**<事件队列深度 */
    u32 data[]; /**<事件队列 */
};

/**
 * @brief 本接口用于获取SIM卡硬件调试信息
 *
 * @par 描述:
 * 本接口用于获取SIM卡硬件调试信息
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[out]  dbg_info sim卡硬件调试信息
 *
 * @retval 0:  获取成功。
 * @retval 非0：获取失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int sim_get_hw_debug_info(u32 sim_id, struct sim_hw_debug_info *dbg_info);
/********sim hardware service end************/

/********kirin pcie start************/
enum kirin_pcie_trigger {
	KIRIN_PCIE_TRIGGER_CALLBACK,
	KIRIN_PCIE_TRIGGER_COMPLETION,
};

enum kirin_pcie_event {
	KIRIN_PCIE_EVENT_MIN_INVALID = 0x0,		/*min invalid value*/
	KIRIN_PCIE_EVENT_LINKUP = 0x1,		/* linkup event  */
	KIRIN_PCIE_EVENT_LINKDOWN = 0x2,		/* linkdown event */
	KIRIN_PCIE_EVENT_WAKE = 0x4,	/* wake event*/
	KIRIN_PCIE_EVENT_L1SS = 0x8,	/* l1ss event*/
	KIRIN_PCIE_EVENT_CPL_TIMEOUT = 0x10,	/* completion timeout event */
	KIRIN_PCIE_EVENT_MAX_INVALID = 0x1F,	/* max invalid value*/
};

struct kirin_pcie_notify {
	enum kirin_pcie_event event;
	void *user;
	void *data;
	u32 options;
};

struct kirin_pcie_register_event {
	u32 events;
	void *user;
	enum kirin_pcie_trigger mode;
	void (*callback)(struct kirin_pcie_notify *notify);
	struct kirin_pcie_notify notify;
	struct completion *completion;
	u32 options;
};

/*****************************************************************************
* 函 数 名  : kirin_pcie_register_event
*
* 功能描述  : EP register hook fun for link event notification
*
* 输入参数  : reg register_event
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg);

/*****************************************************************************
* 函 数 名  : kirin_pcie_deregister_event
*
* 功能描述  : EP deregister hook fun for link event notification
*
* 输入参数  : reg register_event
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg);

/*****************************************************************************
* 函 数 名  : kirin_pcie_pm_control
*
* 功能描述  : ep控制rc上电接口
*
* 输入参数  : power_ops 0---PowerOff normally
*                       1---Poweron normally
*                       2---PowerOFF without PME
*                       3---Poweron without LINK
*             rc_idx ep对接的rc端口号
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_pm_control(int power_ops, u32 rc_idx);

/*****************************************************************************
* 函 数 名  : kirin_pcie_ep_off
*
* 功能描述  : 获取ep设备下电状态接口
*
* 输入参数  : rc_idx ep对接的rc端口号
*
* 输出参数  : 无
*
* 返 回 值  : 0  未下电
*             1  已下电
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_ep_off(u32 rc_idx);

/*****************************************************************************
* 函 数 名  : kirin_pcie_lp_ctrl
*
* 功能描述  : 打开关闭低功耗接口
*
* 输入参数  : rc_idx ep对接的rc端口号
*             enable 0---关闭
*                    1---打开
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable);

/*****************************************************************************
* 函 数 名  : kirin_pcie_enumerate
*
* 功能描述  : pcie枚举接口
*
* 输入参数  : rc_idx ep对接的rc端口号
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_enumerate(u32 rc_idx);

/*****************************************************************************
* 函 数 名  : kirin_pcie_remove_ep
*
* 功能描述  : pcie删除ep设备接口
*
* 输入参数  : rc_idx ep对接的rc端口号
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_remove_ep(u32 rc_idx);

/*****************************************************************************
* 函 数 名  : kirin_pcie_rescan_ep
*
* 功能描述  : pcie扫描ep设备接口
*
* 输入参数  : rc_idx ep对接的rc端口号
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_rescan_ep(u32 rc_idx);

/*****************************************************************************
* 函 数 名  : pcie_ep_link_ltssm_notify
*
* 功能描述  : 设置ep设备link状态接口
*
* 输入参数  : rc_id ep对接的rc端口号
*             link_status ep设备link状态
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status);

/*****************************************************************************
* 函 数 名  : kirin_pcie_power_notifiy_register
*
* 功能描述  : 注册pcie上电提供时钟后ep回调
*
* 输入参数  : rc_id ep对接的rc端口号
*             poweron ep上电回调指针
*             poweroff ep下电回调指针
*             data ep回调函数入参
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
*             非0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int kirin_pcie_power_notifiy_register(u32 rc_id, int (*poweron)(void* data),
				int (*poweroff)(void* data), void* data);

/********kirin pcie end************/

/*****************************************************************************
* 函 数 名  : get_cmd_product_id
*
* 功能描述  : 从kernel的cmdline中获取product id
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 32bit无符号值，非0xFFFFFFFF  处理成功
*             0xFFFFFFFF 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int get_cmd_product_id(void);

/******************* modem temp start *************/
struct mdm_adc_s {
    unsigned int channel;
    unsigned int mode;
};
#ifdef CONFIG_HISI_HKADC_MODEM
extern int hisi_mdm_adc_get_value_register(int (*func)(struct mdm_adc_s *mdm_adc_para));
#else
static inline int hisi_mdm_adc_get_value_register(int (*func)(struct mdm_adc_s *mdm_adc_para))
{
	return 0;
}
#endif
/******************* modem temp end ****************/


/******************* hisi cpufreq req start *************/
struct cpufreq_req {
	struct notifier_block nb;
	int cpu;
	unsigned int freq;
};

#ifdef CONFIG_HISI_CPUFREQ_DT
/*****************************************************************************
 Description: initlize struct cpufreq, register cpufreq call chain
	      notifier block
 Input:       req: a struct cpufreq_req instance
              cpu: the cpu for which we request frequency
 Return:      zero for SUCCESS, non-zero for FAIL
*****************************************************************************/
extern int hisi_cpufreq_init_req(struct cpufreq_req *req, int cpu);

/*****************************************************************************
 Description: update request frequency and trigger a cpufreq scaling
 Input:       req: a struct cpufreq_req instance
              cpu: new request frequency
 Return:      NA
*****************************************************************************/
extern void hisi_cpufreq_update_req(struct cpufreq_req *req,
				    unsigned int freq);
#else
static inline int hisi_cpufreq_init_req(struct cpufreq_req *req, int cpu)
{
	return 0;
}

static inline void hisi_cpufreq_update_req(struct cpufreq_req *req,
					   unsigned int freq)
{
}
#endif
/******************* hisi cpufreq req end *************/

/******************* hisi core ctl req start *************/
#ifdef CONFIG_HISI_CORE_CTRL
/*****************************************************************************
 Description: disable big cluster isolation for a while
 Input:       timeout: the duration we expect for disabling big
		       cluster isolation
 Return:      NA
*****************************************************************************/
extern void core_ctl_set_boost(unsigned int timeout);
#else
static inline void core_ctl_set_boost(unsigned int timeout) {}
#endif
/******************* hisi core ctl req end *************/

/******************* A+B modem connect with hub by ap ctrl *************/
struct hw_ap_cp_route_t {
	unsigned int channel_id;
	int (*icc_open)(unsigned int channel_id, void *read_cb);
	int (*icc_read)(unsigned int channel_id, unsigned char *pdata, int size);
	int (*icc_write)(unsigned int channel_id, unsigned char *pdata, int size);
};
#ifdef CONFIG_INPUTHUB_20
void connect_pcie_icc_sensorhub_route(struct hw_ap_cp_route_t *ap_cp_route);
#else
static inline void connect_pcie_icc_sensorhub_route
	(struct hw_ap_cp_route_t *ap_cp_route)
{
}
#endif
/******************* A+B modem connect with hub by ap ctrl *************/

/* hifi reset notify modem */
enum DRV_RESET_CALLCBFUN_MOMENT {
	DRV_RESET_CALLCBFUN_RESET_BEFORE,
	DRV_RESET_CALLCBFUN_RESET_AFTER,
	DRV_RESET_CALLCBFUN_RESETING,
	DRV_RESET_CALLCBFUN_MOEMENT_INVALID
};
typedef int (*hifi_reset_cbfunc)(enum DRV_RESET_CALLCBFUN_MOMENT eparam, int userdata);

#ifdef CONFIG_HISI_HIFI_BB
int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel);
#else
static inline int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel)
{
	return 0;
}
#endif

/******************* SOCP kernel rsv mem ctrl *************/
typedef struct {
    unsigned long base_addr;
    unsigned int  buffer_size;
    unsigned int  rsv_mem_usable;
    unsigned int  bbp_mem_flag;
    unsigned int  threshold_timeout;
}socp_mem_info_s;

typedef struct {
    unsigned int version;
    socp_mem_info_s socp_mem_info;
}socp_attr_s;

socp_attr_s *bsp_socp_get_mem_info(void);
unsigned int bsp_socp_ds_mem_enable(void);
/******************* SOCP kernel rsv mem ctrl end*************/
enum noc_error_bus {
    NOC_ERRBUS_SYS_CONFIG,
    NOC_ERRBUS_VCODEC,
    NOC_ERRBUS_VIVO,
    NOC_ERRBUS_NPU,
    NOC_ERRBUS_FCM,
    NOC_ERRBUS_BOTTOM,
};

typedef int (*noc_e_callback)(int);
struct noc_err_para_s {
    unsigned int masterid;
    unsigned int initflow;
    unsigned int targetflow;
    enum noc_error_bus bus;
    noc_e_callback noc_callback;
};

/*
 * func name: noc_modid_register
 * func args:
 *   struct noc_err_para_s noc_err_info
 *   unsigned int modid
 * return:
    void
 */

void noc_modid_register(struct noc_err_para_s noc_err_info,unsigned int modid);

/******************* modem dpm start *************/
#define MDM_NAME_SIZE	64
#define DPM_MODULE_MAX	20 /* max value of modem ip_num */
#pragma pack(1)
struct mdm_energy {
	char name[MDM_NAME_SIZE];
	unsigned long long energy;
};

struct mdm_transmit_t {
	unsigned int length;
	char data[0];
};
#pragma pack()

int dpm_modem_register(unsigned int (*pf1)(void),
		       int (*pf2)(struct mdm_transmit_t *data, unsigned int len));
/******************* modem dpm end ****************/

#endif /* HISI_AP_DRV_H */




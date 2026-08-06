
#ifndef __DUMP_MODEM_RDR_
#define __DUMP_MODEM_RDR_
#include "dump_modem.h"

/*modem cp异常的rdr id*/
#define RDR_MODEM_CP_DRV_MOD_ID_START           0x00000000
#define RDR_MODEM_CP_DRV_MOD_ID_END             0x0fffffff
#define RDR_MODEM_CP_OSA_MOD_ID_START           0x10000000
#define RDR_MODEM_CP_OSA_MOD_ID_END             0x1fffffff
#define RDR_MODEM_CP_OAM_MOD_ID_START           0x20000000
#define RDR_MODEM_CP_OAM_MOD_ID_END             0x2fffffff
#define RDR_MODEM_CP_GUL2_MOD_ID_START          0x30000000
#define RDR_MODEM_CP_GUL2_MOD_ID_END            0x3effffff
#define RDR_MODEM_CP_CTTF_MOD_ID_START          0x3f000000
#define RDR_MODEM_CP_CTTF_MOD_ID_END            0x3fffffff
#define RDR_MODEM_CP_GUWAS_MOD_ID_START         0x40000000
#define RDR_MODEM_CP_GUWAS_MOD_ID_END           0x4dffffff
#define RDR_MODEM_CP_CAS_MOD_ID_START           0x4e000000
#define RDR_MODEM_CP_CAS_MOD_ID_END             0x4effffff
#define RDR_MODEM_CP_CPROC_MOD_ID_START         0x4f000000
#define RDR_MODEM_CP_CPROC_MOD_ID_END           0x4fffffff
#define RDR_MODEM_CP_GUGAS_MOD_ID_START         0x50000000
#define RDR_MODEM_CP_GUGAS_MOD_ID_END           0x5fffffff
#define RDR_MODEM_CP_GUCNAS_MOD_ID_START        0x60000000
#define RDR_MODEM_CP_GUCNAS_MOD_ID_END          0x6fffffff
#define RDR_MODEM_CP_GUDSP_MOD_ID_START         0x70000000
#define RDR_MODEM_CP_GUDSP_MOD_ID_END           0x7fffffff
#define RDR_MODEM_CP_LPS_MOD_ID_START           0xa0000000
#define RDR_MODEM_CP_LPS_MOD_ID_END             0xafffffff
#define RDR_MODEM_CP_LMSP_MOD_ID_START          0xb0000000
#define RDR_MODEM_CP_LMSP_MOD_ID_END            0xbfffffff
#define RDR_MODEM_CP_TLDSP_MOD_ID_START         0xc0000000
#define RDR_MODEM_CP_TLDSP_MOD_ID_END           0xcfffffff
#define RDR_MODEM_CP_CPHY_MOD_ID_START          0xd0000000
#define RDR_MODEM_CP_CPHY_MOD_ID_END            0xdfffffff
#define RDR_MODEM_CP_IMS_MOD_ID_START           0xe0000000
#define RDR_MODEM_CP_IMS_MOD_ID_END             0xefffffff


 

/* rdr为CP分配的ID范围为0x82000000~0x82ffffff，定义在rdr_pub.h中*/
#define RDR_MODEM_AP_MOD_ID                     0x82000000  /* AP异常，保存log，整机复位，phone和mbb都有 */
#define RDR_MODEM_CP_MOD_ID                     0x82000001  /* CP异常，保存log，phone和mbb都有 */
#define RDR_MODEM_CP_WDT_MOD_ID                 0x82000003  /* CP看门狗异常，保存log，phone和mbb都有 */
#define RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID    0x82000005  /* 切卡引起的CP单独复位，不保存log，phone独有 */
#define RDR_MODEM_CP_RESET_FAIL_MOD_ID          0x82000006  /* CP单独复位失败，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_RILD_MOD_ID          0x82000007  /* RILD引起的CP单独复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_3RD_MOD_ID           0x82000008  /* 3rd modem引起的CP单独复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID    0x82000009  /* CP单独复位为桩时，整机复位，保存log，phone独有*/
#define RDR_MODEM_CP_RESET_USER_RESET_MOD_ID    0x82000010  /* CP单独复位,用户发起，不保存log,PHONE 独有*/
#define RDR_MODEM_CP_RESET_DLOCK_MOD_ID         0x82000011  /* modem总线挂死*/
#define RDR_MODEM_CP_NOC_MOD_ID                 0x82000012  /* modem内部触发noc */
#define RDR_MODEM_AP_NOC_MOD_ID                 0x82000013  /* modem访问AP触发noc */
#define RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID    0x82000014  /* c 核频繁单独复位，整机重启*/

#define RDR_MODEM_NOC_MOD_ID                    0x82000030  /* NOC异常, 保存log，phone独有, */
                                                            /* 如果要修改者定义一定要知会AP侧同步修改，否则会导致NOC异常log存储不下来 */
#define RDR_MODEM_DMSS_MOD_ID                   0x82000031  /* DMSS异常, 保存log，phone独有, */
                                                            /* 如果要修改者定义一定要知会AP侧同步修改，否则会导致DMSS异常log存储不下来 */


#define RDR_MODEM_CP_DRV_MOD_ID                 0x82000041
#define RDR_MODEM_CP_OSA_MOD_ID                 0x82000042
#define RDR_MODEM_CP_OAM_MOD_ID                 0x82000043
#define RDR_MODEM_CP_GUL2_MOD_ID                0x82000044
#define RDR_MODEM_CP_CTTF_MOD_ID                0x82000045
#define RDR_MODEM_CP_GUWAS_MOD_ID               0x82000046
#define RDR_MODEM_CP_CAS_MOD_ID                 0x82000047
#define RDR_MODEM_CP_CPROC_MOD_ID               0x82000048
#define RDR_MODEM_CP_GUGAS_MOD_ID               0x82000049
#define RDR_MODEM_CP_GUCNAS_MOD_ID              0x8200004a
#define RDR_MODEM_CP_GUDSP_MOD_ID               0x8200004b
#define RDR_MODEM_CP_LPS_MOD_ID                 0x8200004c
#define RDR_MODEM_CP_LMSP_MOD_ID                0x8200004d
#define RDR_MODEM_CP_TLDSP_MOD_ID               0x8200004e
#define RDR_MODEM_CP_CPHY_MOD_ID                0x8200004f
#define RDR_MODEM_CP_IMS_MOD_ID                 0x82000050
#define RDR_MODEM_AP_DRV_MOD_ID                 0x82000051


#define RDR_DUMP_FILE_PATH_LEN                128
#define RDR_DUMP_FILE_CP_PATH                 "cp_log/"
#define RDR_MODEM_CP_RESET_TIMEOUT            (30000)


typedef struct
{
    u32     modid;
    u32     product_type;
    u64     coreid;
    char    log_path[RDR_DUMP_FILE_PATH_LEN];
    pfn_cb_dump_done    dump_done;
    struct rdr_register_module_result soc_rst;
}rdr_exc_info_s;

typedef struct modid
{
    u32 mdm_id_start;
    u32 mdm_id_end;
    u32 rdr_id;
}DUMP_MOD_ID;


rdr_exc_info_s* dump_get_rdr_exc_info(void);
u32 dump_match_rdr_mod_id(u32 drv_mod_id);
void dump_save_balong_rdr_info(u32 mod_id);
s32 dump_register_rdr_exc(void);
u32 dump_convert_id_mdmcp2rdr(u32 mdmcp_mod_id);

#endif

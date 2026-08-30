#ifndef __SOC_VCODEC_BUS_INTERFACE_H__
#define __SOC_VCODEC_BUS_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_VCODEC_BUS_ERR_ID_COREID_ADDR(base) ((base) + (0x1C00))
#define SOC_VCODEC_BUS_ID_REVISIONID_ADDR(base) ((base) + (0x1C04))
#define SOC_VCODEC_BUS_FAULTEN_ADDR(base) ((base) + (0x1C08))
#define SOC_VCODEC_BUS_ERRVLD_ADDR(base) ((base) + (0x1C0C))
#define SOC_VCODEC_BUS_ERRCLR_ADDR(base) ((base) + (0x1C10))
#define SOC_VCODEC_BUS_ERRLOG0_ADDR(base) ((base) + (0x1C14))
#define SOC_VCODEC_BUS_ERRLOG1_ADDR(base) ((base) + (0x1C18))
#define SOC_VCODEC_BUS_ERRLOG3_ADDR(base) ((base) + (0x1C20))
#define SOC_VCODEC_BUS_ERRLOG4_ADDR(base) ((base) + (0x1C24))
#define SOC_VCODEC_BUS_ERRLOG5_ADDR(base) ((base) + (0x1C28))
#define SOC_VCODEC_BUS_ERRLOG7_ADDR(base) ((base) + (0x1C30))
#define SOC_VCODEC_BUS_STALLEN_ADDR(base) ((base) + (0x1C38))
#define SOC_VCODEC_BUS_VENC_QOS_ID_COREID_ADDR(base) ((base) + (0x0000))
#define SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_ADDR(base) ((base) + (0x0004))
#define SOC_VCODEC_BUS_VENC_QOS_PRIORITY_ADDR(base) ((base) + (0x0008))
#define SOC_VCODEC_BUS_VENC_QOS_MODE_ADDR(base) ((base) + (0x000C))
#define SOC_VCODEC_BUS_VENC_QOS_BANDWIDTH_ADDR(base) ((base) + (0x0010))
#define SOC_VCODEC_BUS_VENC_QOS_SATURATION_ADDR(base) ((base) + (0x0014))
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_ADDR(base) ((base) + (0x0018))
#define SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_ADDR(base) ((base) + (0x0000))
#define SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_ADDR(base) ((base) + (0x0004))
#define SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_ADDR(base) ((base) + (0x0008))
#define SOC_VCODEC_BUS_VDEC_QOS_MODE_ADDR(base) ((base) + (0x000C))
#define SOC_VCODEC_BUS_VDEC_QOS_BANDWIDTH_ADDR(base) ((base) + (0x0010))
#define SOC_VCODEC_BUS_VDEC_QOS_SATURATION_ADDR(base) ((base) + (0x0014))
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_ADDR(base) ((base) + (0x0018))
#define SOC_VCODEC_BUS_CNN_QOS_ID_COREID_ADDR(base) ((base) + (0x0000))
#define SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_ADDR(base) ((base) + (0x0004))
#define SOC_VCODEC_BUS_CNN_QOS_PRIORITY_ADDR(base) ((base) + (0x0008))
#define SOC_VCODEC_BUS_CNN_QOS_MODE_ADDR(base) ((base) + (0x000C))
#define SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_ADDR(base) ((base) + (0x0010))
#define SOC_VCODEC_BUS_CNN_QOS_SATURATION_ADDR(base) ((base) + (0x0014))
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_ADDR(base) ((base) + (0x0018))
#define SOC_VCODEC_BUS_ICS2DDRC_ADAPTER_ID_COREID_ADDR(base) ((base) + (0x1000))
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTER_ID_REVISIONID_ADDR(base) ((base) + (0x1004))
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTERRATE_ADDR(base) ((base) + (0x1008))
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAP_BYPASS_ADDR(base) ((base) + (0x100C))
#define SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_ADDR(base) ((base) + (0x0000))
#define SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_ADDR(base) ((base) + (0x0004))
#define SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_ADDR(base) ((base) + (0x0008))
#define SOC_VCODEC_BUS_ICS2_QOS_MODE_ADDR(base) ((base) + (0x000C))
#define SOC_VCODEC_BUS_ICS2_QOS_BANDWIDTH_ADDR(base) ((base) + (0x0010))
#define SOC_VCODEC_BUS_ICS2_QOS_SATURATION_ADDR(base) ((base) + (0x0014))
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_ADDR(base) ((base) + (0x0018))
#define SOC_VCODEC_BUS_ICS22DDRC_ADAPTER_ID_COREID_ADDR(base) ((base) + (0x1000))
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTER_ID_REVISIONID_ADDR(base) ((base) + (0x1004))
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTERRATE_ADDR(base) ((base) + (0x1008))
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAP_BYPASS_ADDR(base) ((base) + (0x100C))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_coreid : 32;
    } reg;
} SOC_VCODEC_BUS_ERR_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_ERR_ID_COREID_id_coreid_START (0)
#define SOC_VCODEC_BUS_ERR_ID_COREID_id_coreid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_revisionid : 32;
    } reg;
} SOC_VCODEC_BUS_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_ID_REVISIONID_id_revisionid_START (0)
#define SOC_VCODEC_BUS_ID_REVISIONID_id_revisionid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int faulten : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_FAULTEN_UNION;
#endif
#define SOC_VCODEC_BUS_FAULTEN_faulten_START (0)
#define SOC_VCODEC_BUS_FAULTEN_faulten_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errvld : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_ERRVLD_UNION;
#endif
#define SOC_VCODEC_BUS_ERRVLD_errvld_START (0)
#define SOC_VCODEC_BUS_ERRVLD_errvld_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errclr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_ERRCLR_UNION;
#endif
#define SOC_VCODEC_BUS_ERRCLR_errclr_START (0)
#define SOC_VCODEC_BUS_ERRCLR_errclr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lock : 1;
        unsigned int opc : 4;
        unsigned int reserved_0: 3;
        unsigned int errcode : 3;
        unsigned int reserved_1: 5;
        unsigned int len1 : 8;
        unsigned int reserved_2: 7;
        unsigned int format : 1;
    } reg;
} SOC_VCODEC_BUS_ERRLOG0_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG0_lock_START (0)
#define SOC_VCODEC_BUS_ERRLOG0_lock_END (0)
#define SOC_VCODEC_BUS_ERRLOG0_opc_START (1)
#define SOC_VCODEC_BUS_ERRLOG0_opc_END (4)
#define SOC_VCODEC_BUS_ERRLOG0_errcode_START (8)
#define SOC_VCODEC_BUS_ERRLOG0_errcode_END (10)
#define SOC_VCODEC_BUS_ERRLOG0_len1_START (16)
#define SOC_VCODEC_BUS_ERRLOG0_len1_END (23)
#define SOC_VCODEC_BUS_ERRLOG0_format_START (31)
#define SOC_VCODEC_BUS_ERRLOG0_format_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errlog1 : 15;
        unsigned int reserved : 17;
    } reg;
} SOC_VCODEC_BUS_ERRLOG1_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG1_errlog1_START (0)
#define SOC_VCODEC_BUS_ERRLOG1_errlog1_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errlog3 : 32;
    } reg;
} SOC_VCODEC_BUS_ERRLOG3_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG3_errlog3_START (0)
#define SOC_VCODEC_BUS_ERRLOG3_errlog3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errlog4 : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_VCODEC_BUS_ERRLOG4_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG4_errlog4_START (0)
#define SOC_VCODEC_BUS_ERRLOG4_errlog4_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errlog5 : 11;
        unsigned int reserved : 21;
    } reg;
} SOC_VCODEC_BUS_ERRLOG5_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG5_errlog5_START (0)
#define SOC_VCODEC_BUS_ERRLOG5_errlog5_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errlog7 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_VCODEC_BUS_ERRLOG7_UNION;
#endif
#define SOC_VCODEC_BUS_ERRLOG7_errlog7_START (0)
#define SOC_VCODEC_BUS_ERRLOG7_errlog7_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stallen : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_STALLEN_UNION;
#endif
#define SOC_VCODEC_BUS_STALLEN_stallen_START (0)
#define SOC_VCODEC_BUS_STALLEN_stallen_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coretypeid : 8;
        unsigned int corechecksum : 24;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_ID_COREID_coretypeid_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_ID_COREID_coretypeid_END (7)
#define SOC_VCODEC_BUS_VENC_QOS_ID_COREID_corechecksum_START (8)
#define SOC_VCODEC_BUS_VENC_QOS_ID_COREID_corechecksum_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int userid : 8;
        unsigned int flexnocid : 24;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_userid_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_userid_END (7)
#define SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_flexnocid_START (8)
#define SOC_VCODEC_BUS_VENC_QOS_ID_REVISIONID_flexnocid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p0 : 2;
        unsigned int reserved_0: 6;
        unsigned int p1 : 2;
        unsigned int reserved_1: 22;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_PRIORITY_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_PRIORITY_p0_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_PRIORITY_p0_END (1)
#define SOC_VCODEC_BUS_VENC_QOS_PRIORITY_p1_START (8)
#define SOC_VCODEC_BUS_VENC_QOS_PRIORITY_p1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_MODE_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_MODE_mode_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_MODE_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bandwidth : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_BANDWIDTH_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_BANDWIDTH_bandwidth_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_BANDWIDTH_bandwidth_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int saturation : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_SATURATION_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_SATURATION_saturation_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_SATURATION_saturation_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int socketqosen : 1;
        unsigned int extthren : 1;
        unsigned int intclken : 1;
        unsigned int extlimiten : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_UNION;
#endif
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_socketqosen_START (0)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_socketqosen_END (0)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_extthren_START (1)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_extthren_END (1)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_intclken_START (2)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_intclken_END (2)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_extlimiten_START (3)
#define SOC_VCODEC_BUS_VENC_QOS_EXTCONTROL_extlimiten_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coretypeid : 8;
        unsigned int corechecksum : 24;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_coretypeid_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_coretypeid_END (7)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_corechecksum_START (8)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_COREID_corechecksum_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int userid : 8;
        unsigned int flexnocid : 24;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_userid_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_userid_END (7)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_flexnocid_START (8)
#define SOC_VCODEC_BUS_VDEC_QOS_ID_REVISIONID_flexnocid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p0 : 2;
        unsigned int reserved_0: 6;
        unsigned int p1 : 2;
        unsigned int reserved_1: 22;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_p0_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_p0_END (1)
#define SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_p1_START (8)
#define SOC_VCODEC_BUS_VDEC_QOS_PRIORITY_p1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_MODE_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_MODE_mode_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_MODE_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bandwidth : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_BANDWIDTH_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_BANDWIDTH_bandwidth_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_BANDWIDTH_bandwidth_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int saturation : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_SATURATION_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_SATURATION_saturation_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_SATURATION_saturation_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int socketqosen : 1;
        unsigned int extthren : 1;
        unsigned int intclken : 1;
        unsigned int extlimiten : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_UNION;
#endif
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_socketqosen_START (0)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_socketqosen_END (0)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_extthren_START (1)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_extthren_END (1)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_intclken_START (2)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_intclken_END (2)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_extlimiten_START (3)
#define SOC_VCODEC_BUS_VDEC_QOS_EXTCONTROL_extlimiten_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coretypeid : 8;
        unsigned int corechecksum : 24;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_ID_COREID_coretypeid_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_ID_COREID_coretypeid_END (7)
#define SOC_VCODEC_BUS_CNN_QOS_ID_COREID_corechecksum_START (8)
#define SOC_VCODEC_BUS_CNN_QOS_ID_COREID_corechecksum_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int userid : 8;
        unsigned int flexnocid : 24;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_userid_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_userid_END (7)
#define SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_flexnocid_START (8)
#define SOC_VCODEC_BUS_CNN_QOS_ID_REVISIONID_flexnocid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p0 : 2;
        unsigned int reserved_0: 6;
        unsigned int p1 : 2;
        unsigned int reserved_1: 22;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_PRIORITY_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_PRIORITY_p0_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_PRIORITY_p0_END (1)
#define SOC_VCODEC_BUS_CNN_QOS_PRIORITY_p1_START (8)
#define SOC_VCODEC_BUS_CNN_QOS_PRIORITY_p1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_MODE_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_MODE_mode_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_MODE_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bandwidth : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_bandwidth_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_bandwidth_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int saturation : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_SATURATION_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_SATURATION_saturation_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_SATURATION_saturation_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int socketqosen : 1;
        unsigned int extthren : 1;
        unsigned int intclken : 1;
        unsigned int extlimiten : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_UNION;
#endif
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_socketqosen_START (0)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_socketqosen_END (0)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_extthren_START (1)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_extthren_END (1)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_intclken_START (2)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_intclken_END (2)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_extlimiten_START (3)
#define SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_extlimiten_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_coreid : 32;
    } reg;
} SOC_VCODEC_BUS_ICS2DDRC_ADAPTER_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2DDRC_ADAPTER_ID_COREID_id_coreid_START (0)
#define SOC_VCODEC_BUS_ICS2DDRC_ADAPTER_ID_COREID_id_coreid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_revisionid : 32;
    } reg;
} SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTER_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTER_ID_REVISIONID_id_revisionid_START (0)
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTER_ID_REVISIONID_id_revisionid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rate : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTERRATE_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTERRATE_rate_START (0)
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAPTERRATE_rate_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_ICS2DDRC_117_ADAP_BYPASS_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAP_BYPASS_bypass_START (0)
#define SOC_VCODEC_BUS_ICS2DDRC_117_ADAP_BYPASS_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coretypeid : 8;
        unsigned int corechecksum : 24;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_coretypeid_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_coretypeid_END (7)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_corechecksum_START (8)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_COREID_corechecksum_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int userid : 8;
        unsigned int flexnocid : 24;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_userid_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_userid_END (7)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_flexnocid_START (8)
#define SOC_VCODEC_BUS_ICS2_QOS_ID_REVISIONID_flexnocid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p0 : 2;
        unsigned int reserved_0: 6;
        unsigned int p1 : 2;
        unsigned int reserved_1: 22;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_p0_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_p0_END (1)
#define SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_p1_START (8)
#define SOC_VCODEC_BUS_ICS2_QOS_PRIORITY_p1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_MODE_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_MODE_mode_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_MODE_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bandwidth : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_BANDWIDTH_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_BANDWIDTH_bandwidth_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_BANDWIDTH_bandwidth_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int saturation : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_SATURATION_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_SATURATION_saturation_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_SATURATION_saturation_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int socketqosen : 1;
        unsigned int extthren : 1;
        unsigned int intclken : 1;
        unsigned int extlimiten : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_UNION;
#endif
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_socketqosen_START (0)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_socketqosen_END (0)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_extthren_START (1)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_extthren_END (1)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_intclken_START (2)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_intclken_END (2)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_extlimiten_START (3)
#define SOC_VCODEC_BUS_ICS2_QOS_EXTCONTROL_extlimiten_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_coreid : 32;
    } reg;
} SOC_VCODEC_BUS_ICS22DDRC_ADAPTER_ID_COREID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS22DDRC_ADAPTER_ID_COREID_id_coreid_START (0)
#define SOC_VCODEC_BUS_ICS22DDRC_ADAPTER_ID_COREID_id_coreid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id_revisionid : 32;
    } reg;
} SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTER_ID_REVISIONID_UNION;
#endif
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTER_ID_REVISIONID_id_revisionid_START (0)
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTER_ID_REVISIONID_id_revisionid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rate : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTERRATE_UNION;
#endif
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTERRATE_rate_START (0)
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAPTERRATE_rate_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_VCODEC_BUS_ICS22DDRC_117_ADAP_BYPASS_UNION;
#endif
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAP_BYPASS_bypass_START (0)
#define SOC_VCODEC_BUS_ICS22DDRC_117_ADAP_BYPASS_bypass_END (0)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif

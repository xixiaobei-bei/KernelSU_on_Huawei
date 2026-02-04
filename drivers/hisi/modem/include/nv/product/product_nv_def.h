/******************************************************************************

    Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       :   product_nv_def.h
  Description     :   产线NV 结构体定义
  History         :
******************************************************************************/

#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_SINGLE_GUC_BAND_CHECK_NUM       28
#define MAX_SINGLE_TL_BAND_CHECK_NUM        12
#define MAX_NV_GUC_CHECK_ITEM_NUM           32
#define MAX_NV_TL_CHECK_ITEM_NUM            64

/*请参考结构体样例*/
typedef struct
{
    int reserved;  /*note */
}PRODUCT_MODULE_STRU;

typedef struct
{
    u32 uwGucCrcResult;                     		/* GUc主卡NV的校验结果 */
    u32 uwTlCrcResult;                      		/* TL主卡NV的校验结果 */
    u32 uwGucM2CrcResult;                   		/* GUC副卡NV的校验结果 */
}NV_CRC_CHECK_RESULT_STRU;

typedef struct
{
    u16 uhwTransmitMode;                       		/* 指示该组的发送模式 */
    u16 uhwBand;                              		/* 指示该组的BAND号 */
    u16 uhwEnable;                              	/* 指示是否有效 */
    u16 uhwValidCount;                              	/* 指示有效数据的个数 */
    u16 auhwNeedCheckID[MAX_SINGLE_GUC_BAND_CHECK_NUM]; /* 保存需要检查的NV_ID值 */
}SINGLE_GUC_BAND_NV_ID_STRU;

typedef struct
{
    u16 uhwTransmitMode;                       		/* 指示该组的发送模式 */
    u16 uhwBand;                              		/* 指示该组的BAND号 */
    u16 uhwEnable;                              	/* 指示是否有效 */
    u16 uhwValidCount;                              	/* 指示有效数据的个数 */
    u16 auhwNeedCheckID[MAX_SINGLE_TL_BAND_CHECK_NUM]; 	/* 保存需要检查的NV_ID值 */
}SINGLE_TL_BAND_NV_ID_STRU;

typedef struct
{
    SINGLE_GUC_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_GUC_CHECK_ITEM_NUM];  	/* 保存需要检查的NV_ID值 */
}NV_GUC_CHECK_ITEM_STRU;

typedef struct
{
    SINGLE_TL_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_TL_CHECK_ITEM_NUM];   	/* 保存需要检查的NV_ID值 */
}NV_TL_CHECK_ITEM_STRU;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif



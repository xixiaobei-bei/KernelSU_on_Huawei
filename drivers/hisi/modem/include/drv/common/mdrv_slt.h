#ifndef __MDRV_SLT_H__
#define __MDRV_SLT_H__
#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
 函 数 名  : mdrv_slt_set_bsn
 功能描述  : mdrv_slt_set_bsn
 输入参数  : bsn --- 待设置bsn号
            
 输出参数  : 无
 返 回 值  : 0    - successful
             -1   - unsuccessful
*****************************************************************************/
int mdrv_slt_set_bsn(char * bsn);

/*****************************************************************************
 函 数 名  : mdrv_slt_get_bsn
 功能描述  : mdrv_slt_get_bsn
 输入参数  : 无
 输出参数  : *bsn --- 返回bsn号
 返 回 值  : 0    - successful
             -1 - unsuccessful
*****************************************************************************/
int mdrv_slt_get_bsn(char * bsn);

int mdrv_slt_pcie_test(void);

int mdrv_slt_usb_test(int usb_mode);

int mdrv_slt_SGMII_test(void);

int mdrv_slt_RGMII_test(void);

int mdrv_slt_spi_test(void);

int mdrv_slt_sdcard_test(void);

int mdrv_slt_acore_l2cache_test(void);

int mdrv_slt_hifi_test(void);

#ifdef __cplusplus
}
#endif
#endif
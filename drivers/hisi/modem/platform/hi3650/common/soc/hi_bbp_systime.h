#ifndef __HI_BBP_SYSTIME_H__
#define __HI_BBP_SYSTIME_H__ 
#include "product_config.h"
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#ifdef BSP_CONFIG_K3V3_SFT
#define HI_BBP_SYSTIME_ABS_TIMER_L_OFFSET 0x1008
#define HI_BBP_SYSTIME_ABS_TIMER_H_OFFSET 0x100c
#else
#define HI_BBP_SYSTIME_ABS_TIMER_L_OFFSET 0x534
#define HI_BBP_SYSTIME_ABS_TIMER_H_OFFSET 0x538
#endif
#ifndef __ASSEMBLY__
typedef union
{
    struct
    {
        unsigned int fpga_version : 32;
    } bits;
    unsigned int u32;
}HI_BBP_SYSTIME_FPGA_VERSION_T;
typedef union
{
    struct
    {
        unsigned int abs_timer_l : 32;
    } bits;
    unsigned int u32;
}HI_BBP_SYSTIME_ABS_TIMER_L_T;
typedef union
{
    struct
    {
        unsigned int abs_timer_h : 32;
    } bits;
    unsigned int u32;
}HI_BBP_SYSTIME_ABS_TIMER_H_T;
typedef union
{
    struct
    {
        unsigned int drp_data : 32;
    } bits;
    unsigned int u32;
}HI_BBP_SYSTIME_DRP_DATA_T;
typedef union
{
    struct
    {
        unsigned int abb_rst : 32;
    } bits;
    unsigned int u32;
}HI_BBP_SYSTIME_ABB_RST_T;
#if 0
HI_SET_GET(hi_bbp_systime_abs_timer_l_abs_timer_l,abs_timer_l,HI_BBP_SYSTIME_ABS_TIMER_L_T,HI_BBP_SYSTIME_BASE_ADDR, HI_BBP_SYSTIME_ABS_TIMER_L_OFFSET)
HI_SET_GET(hi_bbp_systime_abs_timer_h_abs_timer_h,abs_timer_h,HI_BBP_SYSTIME_ABS_TIMER_H_T,HI_BBP_SYSTIME_BASE_ADDR, HI_BBP_SYSTIME_ABS_TIMER_H_OFFSET)
#endif
#endif
#endif

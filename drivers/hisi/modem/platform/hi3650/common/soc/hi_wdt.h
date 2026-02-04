#ifndef __HI_WDT_H__
#define __HI_WDT_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_WDG_LOAD_OFFSET (0x0)
#define HI_WDG_VALUE_OFFSET (0x4)
#define HI_WDG_CONTROL_OFFSET (0x8)
#define HI_WDG_INTCLR_OFFSET (0xC)
#define HI_WDG_RIS_OFFSET (0x10)
#define HI_WDG_MIS_OFFSET (0x14)
#define HI_WDG_LOCK_OFFSET (0xC00)
#define HI_WDG_SYSCTRL_ENABLE_OFFSET (0x33C)
typedef union
{
    struct
    {
        unsigned int wdg_load : 32;
    } bits;
    unsigned int u32;
}HI_WDG_LOAD_T;
typedef union
{
    struct
    {
        unsigned int wdogvalue : 32;
    } bits;
    unsigned int u32;
}HI_WDG_VALUE_T;
typedef union
{
    struct
    {
        unsigned int inten : 1;
        unsigned int resen : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
}HI_WDG_CONTROL_T;
typedef union
{
    struct
    {
        unsigned int wdg_intclr : 32;
    } bits;
    unsigned int u32;
}HI_WDG_INTCLR_T;
typedef union
{
    struct
    {
        unsigned int wdogris : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_WDG_RIS_T;
typedef union
{
    struct
    {
        unsigned int wdogmis : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_WDG_MIS_T;
typedef union
{
    struct
    {
        unsigned int wdg_lock : 32;
    } bits;
    unsigned int u32;
}HI_WDG_LOCK_T;
HI_SET_GET(hi_wdg_load_wdg_load,wdg_load,HI_WDG_LOAD_T,HI_WDT_BASE_ADDR, HI_WDG_LOAD_OFFSET)
HI_SET_GET(hi_wdg_value_wdogvalue,wdogvalue,HI_WDG_VALUE_T,HI_WDT_BASE_ADDR, HI_WDG_VALUE_OFFSET)
HI_SET_GET(hi_wdg_control_inten,inten,HI_WDG_CONTROL_T,HI_WDT_BASE_ADDR, HI_WDG_CONTROL_OFFSET)
HI_SET_GET(hi_wdg_control_resen,resen,HI_WDG_CONTROL_T,HI_WDT_BASE_ADDR, HI_WDG_CONTROL_OFFSET)
HI_SET_GET(hi_wdg_control_reserved,reserved,HI_WDG_CONTROL_T,HI_WDT_BASE_ADDR, HI_WDG_CONTROL_OFFSET)
HI_SET_GET(hi_wdg_intclr_wdg_intclr,wdg_intclr,HI_WDG_INTCLR_T,HI_WDT_BASE_ADDR, HI_WDG_INTCLR_OFFSET)
HI_SET_GET(hi_wdg_ris_wdogris,wdogris,HI_WDG_RIS_T,HI_WDT_BASE_ADDR, HI_WDG_RIS_OFFSET)
HI_SET_GET(hi_wdg_ris_reserved,reserved,HI_WDG_RIS_T,HI_WDT_BASE_ADDR, HI_WDG_RIS_OFFSET)
HI_SET_GET(hi_wdg_mis_wdogmis,wdogmis,HI_WDG_MIS_T,HI_WDT_BASE_ADDR, HI_WDG_MIS_OFFSET)
HI_SET_GET(hi_wdg_mis_reserved,reserved,HI_WDG_MIS_T,HI_WDT_BASE_ADDR, HI_WDG_MIS_OFFSET)
HI_SET_GET(hi_wdg_lock_wdg_lock,wdg_lock,HI_WDG_LOCK_T,HI_WDT_BASE_ADDR, HI_WDG_LOCK_OFFSET)
#endif

#ifndef __HI_TIMER_H__
#define __HI_TIMER_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_TIMER_LOADCOUNT_OFFSET (0x0)
#define HI_TIMER_LOADCOUNT_H_OFFSET (0xB0)
#define HI_TIMER_CURRENTVALUE_OFFSET (0x4)
#define HI_TIMER_CURRENTVALUE_H_OFFSET (0xB4)
#define HI_TIMER_CONTROLREG_OFFSET (0x8)
#define HI_TIMER_EOI_OFFSET (0xC)
#define HI_TIMER_INTSTATUS_OFFSET (0x10)
#define HI_TIMERS_INTSTATUS_OFFSET (0xA0)
#define HI_TIMERS_EOI_OFFSET (0xA4)
#define HI_TIMERS_RAWINTSTATUS_OFFSET (0xA8)
#ifndef __ASSEMBLY__
typedef union
{
    struct
    {
        unsigned int timern_loadcount_l : 32;
    } bits;
    unsigned int u32;
}HI_TIMER_LOADCOUNT_T;
typedef union
{
    struct
    {
        unsigned int timern_loadcount_h : 32;
    } bits;
    unsigned int u32;
}HI_TIMER_LOADCOUNT_H_T;
typedef union
{
    struct
    {
        unsigned int timern_currentvalue_l : 32;
    } bits;
    unsigned int u32;
}HI_TIMER_CURRENTVALUE_T;
typedef union
{
    struct
    {
        unsigned int timern_currentvalue_h : 32;
    } bits;
    unsigned int u32;
}HI_TIMER_CURRENTVALUE_H_T;
typedef union
{
    struct
    {
        unsigned int timern_en : 1;
        unsigned int timern_mode : 1;
        unsigned int timern_int_mask : 1;
        unsigned int reserved_1 : 1;
        unsigned int timern_en_ack : 1;
        unsigned int reserved_0 : 27;
    } bits;
    unsigned int u32;
}HI_TIMER_CONTROLREG_T;
typedef union
{
    struct
    {
        unsigned int timern_eoi : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_TIMER_EOI_T;
typedef union
{
    struct
    {
        unsigned int timern_int_status : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_TIMER_INTSTATUS_T;
typedef union
{
    struct
    {
        unsigned int timer1_int_status : 1;
        unsigned int timer2_int_status : 1;
        unsigned int timer3_int_status : 1;
        unsigned int timer4_int_status : 1;
        unsigned int timer5_int_status : 1;
        unsigned int timer6_int_status : 1;
        unsigned int timer7_int_status : 1;
        unsigned int timer8_int_status : 1;
        unsigned int reserved : 24;
    } bits;
    unsigned int u32;
}HI_TIMERS_INTSTATUS_T;
typedef union
{
    struct
    {
        unsigned int reserved : 32;
    } bits;
    unsigned int u32;
}HI_TIMERS_EOI_T;
typedef union
{
    struct
    {
        unsigned int timer1_raw_int_status : 1;
        unsigned int timer2_raw_int_status : 1;
        unsigned int timer3_raw_int_status : 1;
        unsigned int timer4_raw_int_status : 1;
        unsigned int timer5_raw_int_status : 1;
        unsigned int timer6_raw_int_status : 1;
        unsigned int timer7_raw_int_status : 1;
        unsigned int timer8_raw_int_status : 1;
        unsigned int reserved : 24;
    } bits;
    unsigned int u32;
}HI_TIMERS_RAWINTSTATUS_T;
#endif
#endif

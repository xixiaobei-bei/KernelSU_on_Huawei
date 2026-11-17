#ifndef __BSP_LOADPS_H__
#define __BSP_LOADPS_H__

typedef struct _loadps_msg
{
    unsigned addr;
    unsigned size;
    unsigned rat_mode;
    unsigned seq;
} loadps_msg;

typedef struct _loadps_status
{
    int result;
    unsigned seq;
} loadps_status;

/*镜像的头格式*/
struct loadps_header
{
   unsigned long magic;
   unsigned long image_size;
   unsigned long text_start;
   unsigned long text_size;
   unsigned long data_start;
   unsigned long data_size;
   unsigned long bss_start;
   unsigned long bss_size;
   unsigned long unwind_start;
   unsigned long unwind_size;
   unsigned long symbol_start;
   unsigned long symbol_size;
};

#if 1
#define BSP_LOADPS_LOG_LEVEL  BSP_LOG_LEVEL_INFO

#else
#define BSP_LOADPS_LOG_LEVEL  BSP_LOG_LEVEL_ERROR
#endif
#define LOADPS_ICC_IFC_ID    (ICC_CHN_IFC << 16 | IFC_RECV_FUNC_LOADPS)

void bsp_loadps_debug(void);

#endif



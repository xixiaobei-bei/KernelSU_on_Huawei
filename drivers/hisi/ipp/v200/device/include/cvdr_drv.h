

#ifndef CVDR_DRV_H
#define CVDR_DRV_H

#include "ipp.h"
#include "adapter_common.h"
#include "config_table_cvdr.h"

#define CVDR_TOTAL_BYTES_ALIGN  0x8000
#define DEFAULT_LINE_WRAP       (0x3FFF)

struct _cvdr_ops_t;

typedef struct _cvdr_dev_t
{
    unsigned int base_addr;
    struct _cvdr_ops_t* ops;
    struct _cmd_buf_t* cmd_buf;
} cvdr_dev_t;

typedef struct _cvdr_ops_t
{
    int (*set_debug_enable)(cvdr_dev_t* dev, unsigned char wr_peak_en, unsigned char rd_peak_en);
    int (*get_debug_info)(cvdr_dev_t* dev, unsigned char* wr_peak, unsigned char* rd_peak);
    int (*set_vp_wr_ready)(cvdr_dev_t* dev, unsigned char port, cvdr_wr_fmt_desc_t* desc, cvdr_bw_cfg_t* bw);
    int (*set_vp_rd_ready)(cvdr_dev_t* dev, unsigned char port, cvdr_rd_fmt_desc_t* desc, cvdr_bw_cfg_t* bw);

    int  (*do_config)(cvdr_dev_t* dev, cfg_tab_cvdr_t* table);
    int (*prepare_cmd)(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
    int (*prepare_vprd_cmd)(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
    int (*prepare_vpwr_cmd)(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
} cvdr_ops_t;

int cvdr_prepare_cmd(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
int cvdr_prepare_vprd_cmd(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
int cvdr_prepare_vpwr_cmd(cvdr_dev_t* dev, cmd_buf_t* cmd_buf, cfg_tab_cvdr_t* table);
extern cvdr_dev_t g_cvdr_devs[];

#endif /* CVDR_DRV_H */

/********************************** END **********************************/


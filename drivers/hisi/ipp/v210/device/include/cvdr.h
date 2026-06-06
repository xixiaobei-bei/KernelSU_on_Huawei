#ifndef __CVDR__H__
#define __CVDR__H__

typedef enum _vp_wr_id_e
{
    WR0_UV,
    WR1_Y,
    WR2_C,
    WR_CMDLST,
    WR0_SLAM,
    WR1_SLAM,
    WR2_SLAM,
    WR_MAX,
} vp_wr_id_e;

typedef enum _vp_rd_id_e
{
    RD_CMDLST,
    RD0_L0,
    RD1_L1,
    RD2_L2,
    RD3_MONO_DS4,
    RD4_MONO,
    RD5_COL_Y_DS4,
    RD0_SLAM,
    RD_MAX,
} vp_rd_id_e;

extern unsigned int g_cvdr_wr_id[];
extern unsigned int g_cvdr_rd_id[];

int ippdev_lock(void);
int ippdev_unlock(void);

#define get_cvdr_wr_port_num(x) g_cvdr_wr_id[x]
#define get_cvdr_rd_port_num(x) g_cvdr_rd_id[x]
#endif/*__CVDR__H__ */

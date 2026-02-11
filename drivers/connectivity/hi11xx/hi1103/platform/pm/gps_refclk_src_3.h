

#ifndef __GPS_REFCLK_SRC_3_H__
#define __GPS_REFCLK_SRC_3_H__

#include "oal_util.h"
#include "oneimage.h"
#include "hisi_oneimage.h"

typedef struct {
    bool enable;
    gps_modem_id_enum modem_id;
    gps_rat_mode_enum rat;
} gps_refclk_param;

/* EXTERN FUNCTION */
extern int hi_gps_plat_init_etc(void);
extern void hi_gps_plat_exit_etc(void);
extern int set_gps_ref_clk_enable_hi110x_etc(bool enable, gps_modem_id_enum modem_id, gps_rat_mode_enum rat_mode);
#endif

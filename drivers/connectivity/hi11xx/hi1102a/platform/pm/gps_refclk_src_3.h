

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
extern void plat_gnss_clk_enable(void);
extern void plat_gnss_clk_disable(void);
extern int set_gps_ref_clk_enable_hi110x(bool enable, gps_modem_id_enum modem_id, gps_rat_mode_enum rat_mode);
#endif

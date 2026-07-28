/*
 * cvdr_config.h
 *
 * provide interface for config cvdr.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#ifndef _INCLUDE_CVDR_CFG_H
#define _INCLUDE_CVDR_CFG_H

#include "hjpeg_common.h"
#include <media/huawei/hjpeg_cfg.h>

extern void hjpeg_config_cvdr(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t* config);
extern int is_hjpeg_qos_update(void);
extern int is_hjpeg_wr_port_addr_update(void);
extern int is_hjpeg_iova_update(void);

#define EXP_PIX                              1
#define ACCESS_LIMITER                       7
#define ALLOCATED_DU                         8

#define ACCESS_LIMITER_VP_WR_0                      0x2
#define ACCESS_LIMITER_NR_RD_0                      0x4
#define ALLOCATED_DU_NR_RD_0                        0x7
#define ALLOCATED_DU_NR_RD_0_UPDATE                 0x6

#define CVDR_AXI_JPEG_CVDR_CFG                      0x0
#define AXI_JPEG_CVDR_CFG_max_axiwrite_id_OFFSET    24
#define AXI_JPEG_CVDR_CFG_max_axiwrite_id_LEN       5
#define AXI_JPEG_CVDR_CFG_max_axiread_id_OFFSET     16
#define AXI_JPEG_CVDR_CFG_max_axiread_id_LEN        5

#define CVDR_AXI_JPEG_CVDR_WR_QOS_CFG               0xC
#define CVDR_AXI_JPEG_CVDR_RD_QOS_CFG               0x10
#define CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET            0x1C
#define CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET         0x20
#define CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET       0x24
#define CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET         0x28
#define CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET        0x400
#define CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET       0x894
#define CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET            0xA00
#define CVDR_AXI_JPEG_NR_RD_DEBUG_0_OFFSET          0xA04
#define CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET        0xA08
#define CVDR_AXI_JPEG_DEBUG_0_OFFSET                0xB00
#define CVDR_AXI_JPEG_DEBUG_1_OFFSET                0xB04
#define CVDR_AXI_JPEG_DEBUG_2_OFFSET                0xB08
#define CVDR_AXI_JPEG_DEBUG_3_OFFSET                0xB0C
#define CVDR_AXI_JPEG_VP_WR_DEBUG_0_OFFSET          0xF00

#define CVDR_SRT_VP_WR_CFG_25_OFFSET                0x1AC
#define CVDR_SRT_VP_WR_AXI_FS_25_OFFSET             0x1B0
#define CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET           0x1B4
#define CVDR_SRT_VP_WR_IF_CFG_25_OFFSET             0x1B8

#define CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE         0x1C0
#define CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE      0x1C4
#define CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE    0x1C8
#define CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE      0x1CC

#define CVDR_SRT_NR_RD_CFG_1_OFFSET                 0xA10
#define CVDR_SRT_LIMITER_NR_RD_1_OFFSET             0xA18
#define CVDR_SRT_LIMITER_VP_WR_25_OFFSET            0x464
#define CVDR_AXI_JPEG_NR_RD_CFG_4                   0x1570
#define CVDR_AXI_JPEG_LIMITER_NR_RD_4               0x1578

typedef enum _cvdr_pix_fmt_e
{
    DF_1PF8 = 0x0,
    DF_1PF10 = 0x1,
    DF_1PF12 = 0x2,
    DF_1PF14 = 0x3,
    DF_2PF8 = 0x4,
    DF_2PF10 = 0x5,
    DF_2PF12 = 0x6,
    DF_2PF14 = 0x7,
    DF_3PF8 = 0x8,
    DF_3PF10 = 0x9,
    DF_3PF12 = 0xA,
    DF_3PF14 = 0xB,
    DF_D32 = 0xC,
    DF_D48 = 0xD,
    DF_D64 = 0xE,
    DF_FMT_INVALID,
} cvdr_pix_fmt_e;

typedef enum _cvdr_rd_port
{
    RD_PORT_0  = 0,
    RD_PORT_1  = 1,
    RD_PORT_4  = 4,
}cvdr_rd_port;

typedef enum _cvdr_wr_port
{
    WR_PORT_0 = 0,
    WR_PORT_25 = 25,
}cvdr_wr_port;

/* Define the union U_CVDR_SRT_LIMITER_NR_RD_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t vpwr_access_limiter_0_0 : 4 ; /* [3..0] */
        uint32_t vpwr_access_limiter_1_0 : 4 ; /* [7..4] */
        uint32_t vpwr_access_limiter_2_0 : 4 ; /* [11..8] */
        uint32_t vpwr_access_limiter_3_0 : 4 ; /* [15..12] */
        uint32_t reserved_0              : 8 ; /* [23..16] */
        uint32_t vpwr_access_limiter_reload_0 : 4 ; /* [27..24] */
        uint32_t reserved_1              : 4 ; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_CVDR_SRT_LIMITER_VP_WR_0;


/* Define the union U_CVDR_WR_QOS_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t wr_qos_threshold_01_stop  : 4 ; /* [3..0] */
        uint32_t wr_qos_threshold_01_start : 4 ; /* [7..4] */
        uint32_t wr_qos_threshold_10_stop  : 4 ; /* [11..8] */
        uint32_t wr_qos_threshold_10_start : 4 ; /* [15..12] */
        uint32_t wr_qos_threshold_11_stop  : 4 ; /* [19..16] */
        uint32_t wr_qos_threshold_11_start : 4 ; /* [23..20] */
        uint32_t reserved                  : 2 ; /* [25..24] */
        uint32_t wr_qos_min                : 2 ; /* [27..26] */
        uint32_t wr_qos_max                : 2 ; /* [29..28] */
        uint32_t wr_qos_sr                 : 2 ; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_CVDR_WR_QOS_CFG;

/* Define the union U_CVDR_RD_QOS_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t rd_qos_threshold_01_stop  : 4 ; /* [3..0] */
        uint32_t rd_qos_threshold_01_start : 4 ; /* [7..4] */
        uint32_t rd_qos_threshold_10_stop  : 4 ; /* [11..8] */
        uint32_t rd_qos_threshold_10_start : 4 ; /* [15..12] */
        uint32_t rd_qos_threshold_11_stop  : 4 ; /* [19..16] */
        uint32_t rd_qos_threshold_11_start : 4 ; /* [23..20] */
        uint32_t reserved                  : 2 ; /* [25..24] */
        uint32_t rd_qos_min                : 2 ; /* [27..26] */
        uint32_t rd_qos_max                : 2 ; /* [29..28] */
        uint32_t rd_qos_sr                 : 2 ; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;
} U_CVDR_RD_QOS_CFG;


typedef struct _cvdr_wr_fmt_desc_t
{
    cvdr_pix_fmt_e      pix_fmt;
    unsigned char       pix_expan;
    unsigned char       access_limiter;
    uint32_t            last_page;
    unsigned short      line_stride;
    unsigned short      line_wrap;
} cvdr_wr_fmt_desc_t;

typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t    vpwr_pixel_format     : 4   ; /* [3..0]  */
        uint32_t    vpwr_pixel_expansion  : 1   ; /* [4]  */
        uint32_t    reserved_0            : 4   ; /* [8..5]  */
        uint32_t    vpwr_access_limiter   : 4   ; /* [12..9]  */
        uint32_t    reserved_1            : 1   ; /* [13]  */
        uint32_t    reserved_2            : 1   ; /* [14]  */
        uint32_t    vpwr_last_page        : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    uint32_t    reg32;

} U_VP_WR_CFG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t    vpwr_pixel_format     : 4   ; /* [3..0]  */
        uint32_t    vpwr_pixel_expansion  : 1   ; /* [4]  */
        uint32_t    reserved_0            : 4   ; /* [8..5]  */
        uint32_t    vpwr_access_limiter   : 4   ; /* [12..9]  */
        uint32_t    vpwr_last_page        : 17  ; /* [29..13]  */
        uint32_t    reserved_1            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    uint32_t    reg32;

} U_VP_WR_CFG_UPDATE;

typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t    vpwr_line_stride      : 10  ; /* [9..0]  */
        uint32_t    reserved_0            : 5   ; /* [14..10]  */
        uint32_t    vpwr_line_wrap        : 14  ; /* [28..15]  */
        uint32_t    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    uint32_t    reg32;

} U_VP_WR_AXI_LINE;

typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t    reserved_0            : 4   ; /* [3..0]  */
        uint32_t    vpwr_address_frame_start : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    uint32_t    reg32;

} U_VP_WR_AXI_FS;

/* Define the union U_CVDR_SRT_NR_RD_CFG_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t reserved_0 : 5 ; /* [4..0] */
        uint32_t nrrd_allocated_du_1 : 5 ; /* [9..5] */
        uint32_t reserved_1 : 6 ; /* [15..10] */
        uint32_t nr_rd_stop_enable_du_threshold_reached_1 : 1 ; /* [16] */
        uint32_t nr_rd_stop_enable_flux_ctrl_1 : 1 ; /* [17] */
        uint32_t nr_rd_stop_enable_pressure_1 : 1 ; /* [18] */
        uint32_t reserved_2 : 5 ; /* [23..19] */
        uint32_t nr_rd_stop_ok_1 : 1 ; /* [24] */
        uint32_t nr_rd_stop_1 : 1 ; /* [25] */
        uint32_t reserved_3 : 5 ; /* [30..26] */
        uint32_t nrrd_enable_1 : 1 ; /* [31] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_CVDR_SRT_NR_RD_CFG_1;

/* Define the union U_CVDR_SRT_LIMITER_NR_RD_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t nrrd_access_limiter_0_1 : 4 ; /* [3..0] */
        uint32_t nrrd_access_limiter_1_1 : 4 ; /* [7..4] */
        uint32_t nrrd_access_limiter_2_1 : 4 ; /* [11..8] */
        uint32_t nrrd_access_limiter_3_1 : 4 ; /* [15..12] */
        uint32_t reserved_0 : 8 ; /* [23..16] */
        uint32_t nrrd_access_limiter_reload_1 : 4 ; /* [27..24] */
        uint32_t reserved_1 : 4 ; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_CVDR_SRT_LIMITER_NR_RD_1;


#endif  // _INCLUDE_CVDR_CFG_H

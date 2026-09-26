/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_dmd.h.
 * Create: 2019/11/05
 */
#ifndef __IOMCU_DMD_H
#define __IOMCU_DMD_H

/*
 * Function    : dmd_init
 * Description : dmd resource init
 * Input       : none
 * Output      : none
 * Return      : none
 */
void dmd_init(void);

/*
 * Function    : get_shb_dclient
 * Description : get dsm_client for sensorhub
 * Input       : none
 * Output      : none
 * Return      : none
 */
struct dsm_client *get_shb_dclient(void);

/*
 * Function    : get_shb_dsm_dev
 * Description : get dsm_dev for sensorhub
 * Input       : none
 * Output      : none
 * Return      : none
 */
struct dsm_dev *get_shb_dsm_dev(void);
#endif /* __IOMCU_DMD_H */

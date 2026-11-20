/*
 * ipc_rproc_id_mgr.h
 *
 * IPC mailbox driver: business channel id and business channel name map.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _IPC_RPROC_ID_MGR_H_
#define _IPC_RPROC_ID_MGR_H_

/*
 * Find IPC mailbox rproc_id by rproc name.
 * @pram rproc_name   the business channel name of mailbox
 *
 * @return  enum rproc_id_t
 */
int ipc_find_rproc_id(const char *rproc_name);

/*
 * Get rproc_name by rproc_id.
 * @param rproc_id   the business channel id
 *
 * @return the business channel name
 */
char *ipc_get_rproc_name(int rproc_id);

/*
 * Check the  the business channel id and name map config.
 *
 * @return  0:success   other:config failed
 */
int ipc_check_rproc_id_cfg(void);

#endif

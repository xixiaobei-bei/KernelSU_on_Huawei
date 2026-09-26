/*
 * head file of mntn_filesys
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __MNTN_DUMP_H__
#define __MNTN_DUMP_H__

#ifdef CONFIG_HISI_MNTN_PC
	#define KDUMP_DONE_FILE "/var/log/hisi/bbox/memorydump/done"
#else
	#define KDUMP_DONE_FILE "/data/hisi_logs/memorydump/done"
#endif

void create_hisi_kdump_gzip_log_file(void);
unsigned int skp_skp_flag(void);

#endif

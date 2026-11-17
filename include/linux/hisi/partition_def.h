/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: partition table
 */
#ifndef _PARTITION_DEF_H_
#define _PARTITION_DEF_H_

struct partition {
	char name[PART_NAMELEN];
	unsigned long long start;
	unsigned long long length;
	unsigned int flags;
};

#endif


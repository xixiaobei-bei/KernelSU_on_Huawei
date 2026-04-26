/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for init_user and
 *              package management.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_IOCTL_IMPL_H
#define _HWDPS_IOCTL_IMPL_H

#include <huawei_platform/hwdps/hwdps_ioctl.h>

void hwdps_sync_installed_packages(
	struct hwdps_sync_packages_t *sync_packages);

/*
 * This function insert package information when install app.
 * Input: install_package: package information
 */
void hwdps_install_package(struct hwdps_install_package_t *install_package);

/*
 * This function delete package information when uninstall app.
 * Input: uninstall_package: package information
 */
void hwdps_uninstall_package(
	struct hwdps_uninstall_package_t *uninstall_package);

void hwdps_init_user(struct hwdps_init_user_t *iusr);

#endif

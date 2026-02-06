/*
 * EFI Test function for Runtime Services
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is used for test EFI runtime service interface.
 *
 */
#include <linux/module.h>
#include <linux/efi.h>

MODULE_LICENSE("GPL");

static long efi_status_assert(efi_status_t status, efi_status_t expect, const char* func)
{
	if (status == expect) {
		pr_err("[%s] pass!", func);
		return 0;
	}
	pr_err("[%s] failed, status: 0x%lx!", func, status);
	return -1;
}

static long efi_runtime_set_variable_test(void)
{
	efi_status_t status;
	efi_guid_t vendor_guid = NULL_GUID;

	status = efi.set_variable((efi_char16_t*)"Lang", &vendor_guid, (u32)0x0, (unsigned long)0x0, NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_get_time_test(void)
{
	efi_status_t status;

	status = efi.get_time(NULL, NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_set_time_test(void)
{
	efi_status_t status;

	status = efi.set_time(NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_get_waketime_test(void)
{
	efi_status_t status;

	status = efi.get_wakeup_time(NULL, NULL, NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_set_waketime_test(void)
{
	efi_status_t status;

	status = efi.set_wakeup_time((efi_bool_t)0, NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_get_nexthighmonocount_test(void)
{
	efi_status_t status;

	status = efi.get_next_high_mono_count(NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_query_variableinfo_test(void)
{
	efi_status_t status;

	status = efi.query_variable_info((u32)0x0, NULL, NULL, NULL);
	return efi_status_assert(status, EFI_INVALID_PARAMETER, __func__);
}

static long efi_runtime_query_capsulecaps_test(void)
{
	efi_status_t status;

	status = efi.query_capsule_caps(NULL, (unsigned long)0x0, NULL, NULL);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

static long efi_runtime_update_capsule_test(void)
{
	efi_status_t status;

	status = efi.update_capsule(NULL, (unsigned long)0x0, (unsigned long)0x0);
	return efi_status_assert(status, EFI_UNSUPPORTED, __func__);
}

long efi_runtime_test(void)
{
	long res = 0;
	pr_err("%s start!\n", __func__);
	res += efi_runtime_set_variable_test();
	res += efi_runtime_get_time_test();
	res += efi_runtime_set_time_test();
	res += efi_runtime_get_waketime_test();
	res += efi_runtime_set_waketime_test();
	res += efi_runtime_get_nexthighmonocount_test();
	res += efi_runtime_query_variableinfo_test();
	res += efi_runtime_query_capsulecaps_test();
	res += efi_runtime_update_capsule_test();
	if (res == 0) {
		pr_err("%s success!\n", __func__);
		return 0;
	}
	pr_err("%s failed!\n", __func__);
	return -1;
}

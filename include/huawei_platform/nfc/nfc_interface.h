/*
 * nfc_interface.h
 *
 * nfc interface
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

enum NFC_PROBE_RESULT {
	NFC_PN547_PROBE_SUCCESS = 0,
	NFC_PN547_PROBE_FAIL,
	NFC_ST21NFC_PROBE_SUCCESS,
	NFC_ST21NFC_PROBE_FAIL,
};

int set_nfc_dmd_probe_flag(unsigned char probe_result);
unsigned char get_nfc_dmd_probe_flag(void);

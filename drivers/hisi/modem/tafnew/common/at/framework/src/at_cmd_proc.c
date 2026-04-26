/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "at_cmd_proc.h"
#include "securec.h"
#include "at_check_func.h"
#include "ppp_interface.h"
#include "taf_mmi_str_parse.h"
#include "app_vc_api.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "taf_std_lib.h"
#include "mn_comm_api.h"
#include "at_event_report.h"

#include "at_common.h"

#include "at_mdrv_interface.h" /* AT_SysbootShutdown AT_GetTimerAccuracyTimestamp */

#include "at_ltev_taf_rslt_proc.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CMDPROC_C

#define AT_VALID_COMMA_NUM 2
#define AT_SIMLOCK_VALID_PARA_NUM 2


#define AT_SIMLOCK_PLMN_MAX_NUM 61
#define AT_CMDTMP_MAX_LEN 100

#define AT_BLANK_ASCII_VALUE 0x20

#define AT_CLCC_PARA_MODE_VOICE 0 /* 语音呼叫模式 */
#define AT_CLCC_PARA_MODE_DATA  1 /* 数据呼叫模式 */
#define AT_CLCC_PARA_MODE_FAX   2 /* 传真模式 */

#define AT_SEND_DATA_BUFF_INDEX_1 1
#if (FEATURE_MBB_CUST == FEATURE_ON)
#define AT_CPCM_CODEC_CLOCK_MODE_MAX_NUM 10
/* AT指令支持5种时钟设置 */
const CodecClockMode g_codecClockMode[AT_CPCM_CODEC_CLOCK_MODE_MAX_NUM] = {
    { CPCM_CODEC_CLOCK_MODE_2M, APP_VC_CPCM_CODEC_CLOCK_2M },
    { CPCM_CODEC_CLOCK_MODE_1M, APP_VC_CPCM_CODEC_CLOCK_1M },
    { CPCM_CODEC_CLOCK_MODE_512k, APP_VC_CPCM_CODEC_CLOCK_512K },
    { CPCM_CODEC_CLOCK_MODE_256k, APP_VC_CPCM_CODEC_CLOCK_256K },
    { CPCM_CODEC_CLOCK_MODE_4M, APP_VC_CPCM_CODEC_CLOCK_4M }
};
#endif
/*
 * The following tables maps the 256 characters of PBM8 to the corresponding
 * unicode characters.
 */
const AT_PB_ConversionTable g_iraToUnicode[AT_PB_IRA_MAX_NUM] = {
    { 0x00, 0, 0x0000 }, /* Null */
    { 0x01, 0, 0x0001 }, /* Start of heading */
    { 0x02, 0, 0x0002 }, /* Start of text */
    { 0x03, 0, 0x0003 }, /* End of text */
    { 0x04, 0, 0x0004 }, /* End of transmission */
    { 0x05, 0, 0x0005 }, /* Inquiry */
    { 0x06, 0, 0x0006 }, /* ACK */
    { 0x07, 0, 0x0007 }, /* Bell */
    { 0x08, 0, 0x0008 }, /* Backspace */
    { 0x09, 0, 0x0009 }, /* Tab */
    { 0x0A, 0, 0x000A }, /* Line feed */
    { 0x0B, 0, 0x000B }, /* Vertical tab */
    { 0x0C, 0, 0x000C }, /* Form feed */
    { 0x0D, 0, 0x000D }, /* Carriage return */
    { 0x0E, 0, 0x000E }, /* Shift out */
    { 0x0F, 0, 0x000F }, /* Shift in */
    { 0x10, 0, 0x0010 }, /* Data link escape */
    { 0x11, 0, 0x0011 }, /* Device control one */
    { 0x12, 0, 0x0012 }, /* Device control two */
    { 0x13, 0, 0x0013 }, /* Device control three */
    { 0x14, 0, 0x0014 }, /* Device control four */
    { 0x15, 0, 0x0015 }, /* NAK */
    { 0x16, 0, 0x0016 }, /* Synch */
    { 0x17, 0, 0x0017 }, /* End of block */
    { 0x18, 0, 0x0018 }, /* Cancel */
    { 0x19, 0, 0x0019 }, /* End of medium */
    { 0x1A, 0, 0x001A }, /* Substitute */
    { 0x1B, 0, 0x001B }, /* ESC */
    { 0x1C, 0, 0x001C }, /* File separator */
    { 0x1D, 0, 0x001D }, /* Group separator */
    { 0x1E, 0, 0x001E }, /* Record separator */
    { 0x1F, 0, 0x001F }, /* Unit separator */
    { 0x20, 0, 0x0020 }, /* Space */
    { 0x21, 0, 0x0021 }, /* Exclamation mark */
    { 0x22, 0, 0x0022 }, /* Quotation mark */
    { 0x23, 0, 0x0023 }, /* Number sign */
    { 0x24, 0, 0x0024 }, /* Dollar sign */
    { 0x25, 0, 0x0025 }, /* Percent sign */
    { 0x26, 0, 0x0026 }, /* Ampersand */
    { 0x27, 0, 0x0027 }, /* Apostrophe */
    { 0x28, 0, 0x0028 }, /* Left parenthesis */
    { 0x29, 0, 0x0029 }, /* Right parenthesis */
    { 0x2A, 0, 0x002A }, /* Asterisk */
    { 0x2B, 0, 0x002B }, /* Plus */
    { 0x2C, 0, 0x002C }, /* Comma */
    { 0x2D, 0, 0x002D }, /* Hyphen */
    { 0x2E, 0, 0x002E }, /* Full stop */
    { 0x2F, 0, 0x002F }, /* Solidus */
    { 0x30, 0, 0x0030 }, /* Zero */
    { 0x31, 0, 0x0031 }, /* One */
    { 0x32, 0, 0x0032 }, /* Two */
    { 0x33, 0, 0x0033 }, /* Three */
    { 0x34, 0, 0x0034 }, /* Four */
    { 0x35, 0, 0x0035 }, /* Five */
    { 0x36, 0, 0x0036 }, /* Six */
    { 0x37, 0, 0x0037 }, /* Seven */
    { 0x38, 0, 0x0038 }, /* Eight */
    { 0x39, 0, 0x0039 }, /* Nine */
    { 0x3A, 0, 0x003A }, /* Colon */
    { 0x3B, 0, 0x003B }, /* Semicolon */
    { 0x3C, 0, 0x003C }, /* < (Less than) */
    { 0x3D, 0, 0x003D }, /* = Equals */
    { 0x3E, 0, 0x003E }, /* > greater than */
    { 0x3F, 0, 0x003F }, /* Question mark */
    { 0x40, 0, 0x0040 }, /* AT Sign */
    { 0x41, 0, 0x0041 }, /* Latin A */
    { 0x42, 0, 0x0042 }, /* Latin B */
    { 0x43, 0, 0x0043 }, /* Latin C */
    { 0x44, 0, 0x0044 }, /* Latin D */
    { 0x45, 0, 0x0045 }, /* Latin E */
    { 0x46, 0, 0x0046 }, /* Latin F */
    { 0x47, 0, 0x0047 }, /* Latin G */
    { 0x48, 0, 0x0048 }, /* Latin H */
    { 0x49, 0, 0x0049 }, /* Latin I */
    { 0x4A, 0, 0x004A }, /* Latin J */
    { 0x4B, 0, 0x004B }, /* Latin K */
    { 0x4C, 0, 0x004C }, /* Latin L */
    { 0x4D, 0, 0x004D }, /* Latin M */
    { 0x4E, 0, 0x004E }, /* Latin N */
    { 0x4F, 0, 0x004F }, /* Latin O */
    { 0x50, 0, 0x0050 }, /* Latin P */
    { 0x51, 0, 0x0051 }, /* Latin Q */
    { 0x52, 0, 0x0052 }, /* Latin R */
    { 0x53, 0, 0x0053 }, /* Latin S */
    { 0x54, 0, 0x0054 }, /* Latin T */
    { 0x55, 0, 0x0055 }, /* Latin U */
    { 0x56, 0, 0x0056 }, /* Latin V */
    { 0x57, 0, 0x0057 }, /* Latin W */
    { 0x58, 0, 0x0058 }, /* Latin X */
    { 0x59, 0, 0x0059 }, /* Latin Y */
    { 0x5A, 0, 0x005A }, /* Latin Z */
    { 0x5B, 0, 0x005B }, /* Left square bracket */
    { 0x5C, 0, 0x005C }, /* Reverse solidus */
    { 0x5D, 0, 0x005D }, /* Right square bracket */
    { 0x5E, 0, 0x005E }, /* Circumflex accent */
    { 0x5F, 0, 0x005F }, /* Low line */
    { 0x60, 0, 0x0060 }, /* Grave accent */
    { 0x61, 0, 0x0061 }, /* Latin a */
    { 0x62, 0, 0x0062 }, /* Latin b */
    { 0x63, 0, 0x0063 }, /* Latin c */
    { 0x64, 0, 0x0064 }, /* Latin d */
    { 0x65, 0, 0x0065 }, /* Latin e */
    { 0x66, 0, 0x0066 }, /* Latin f */
    { 0x67, 0, 0x0067 }, /* Latin g */
    { 0x68, 0, 0x0068 }, /* Latin h */
    { 0x69, 0, 0x0069 }, /* Latin i */
    { 0x6A, 0, 0x006A }, /* Latin j */
    { 0x6B, 0, 0x006B }, /* Latin k */
    { 0x6C, 0, 0x006C }, /* Latin l */
    { 0x6D, 0, 0x006D }, /* Latin m */
    { 0x6E, 0, 0x006E }, /* Latin n */
    { 0x6F, 0, 0x006F }, /* Latin o */
    { 0x70, 0, 0x0070 }, /* Latin p */
    { 0x71, 0, 0x0071 }, /* Latin q */
    { 0x72, 0, 0x0072 }, /* Latin r */
    { 0x73, 0, 0x0073 }, /* Latin s */
    { 0x74, 0, 0x0074 }, /* Latin t */
    { 0x75, 0, 0x0075 }, /* Latin u */
    { 0x76, 0, 0x0076 }, /* Latin v */
    { 0x77, 0, 0x0077 }, /* Latin w */
    { 0x78, 0, 0x0078 }, /* Latin x */
    { 0x79, 0, 0x0079 }, /* Latin y */
    { 0x7A, 0, 0x007A }, /* Latin z */
    { 0x7B, 0, 0x007B }, /* Left curly bracket */
    { 0x7C, 0, 0x007C }, /* Vertical line */
    { 0x7D, 0, 0x007D }, /* Right curly bracket */
    { 0x7E, 0, 0x007E }, /* Tilde */
    { 0x7F, 0, 0x007F }, /* DEL */
    { 0x80, 0, 0x0080 }, /* control character 0x80 */
    { 0x81, 0, 0x0081 }, /* control character 0x81 */
    { 0x82, 0, 0x0082 }, /* control character 0x82 */
    { 0x83, 0, 0x0083 }, /* control character 0x83 */
    { 0x84, 0, 0x0084 }, /* control character 0x84 */
    { 0x85, 0, 0x0085 }, /* control character 0x85 */
    { 0x86, 0, 0x0086 }, /* control character 0x86 */
    { 0x87, 0, 0x0087 }, /* control character 0x87 */
    { 0x88, 0, 0x0088 }, /* control character 0x88 */
    { 0x89, 0, 0x0089 }, /* control character 0x89 */
    { 0x8A, 0, 0x008A }, /* control character 0x8A */
    { 0x8B, 0, 0x008B }, /* control character 0x8B */
    { 0x8C, 0, 0x008C }, /* control character 0x8C */
    { 0x8D, 0, 0x008D }, /* control character 0x8D */
    { 0x8E, 0, 0x008E }, /* control character 0x8E */
    { 0x8F, 0, 0x008F }, /* control character 0x8F */
    { 0x90, 0, 0x0394 }, /* Greek capital letter delta */
    { 0x91, 0, 0x20AC }, /* Euro sign */
    { 0x92, 0, 0x03A6 }, /* Greek capital letter phi */
    { 0x93, 0, 0x0393 }, /* Greek capital letter gamma */
    { 0x94, 0, 0x039B }, /* Greek capital letter lamda */
    { 0x95, 0, 0x03A9 }, /* Greek capital letter omega */
    { 0x96, 0, 0x03A0 }, /* Greek capital letter pi */
    { 0x97, 0, 0x03A8 }, /* Greek capital letter psi */
    { 0x98, 0, 0x03A3 }, /* Greek capital letter sigma */
    { 0x99, 0, 0x0398 }, /* Greek capital letter theta */
    { 0x9A, 0, 0x039E }, /* Greek capital letter xi */
    { 0x9B, 0, 0x009B }, /* control character 0x9B */
    { 0x9C, 0, 0x009C }, /* control character 0x9C */
    { 0x9D, 0, 0x009D }, /* control character 0x9D */
    { 0x9E, 0, 0x009E }, /* control character 0x9E */
    { 0x9F, 0, 0x009F }, /* control character 0x9F */
    { 0xA0, 0, 0x00A0 }, /* Non-Block Space */
    { 0xA1, 0, 0x00A1 }, /* Inverted exclamation mark */
    { 0xA2, 0, 0x00A2 }, /* Cent sign */
    { 0xA3, 0, 0x00A3 }, /* Pound sign */
    { 0xA4, 0, 0x00A4 }, /* Currency sign */
    { 0xA5, 0, 0x00A5 }, /* Yen sign */
    { 0xA6, 0, 0x00A6 }, /* Broken Vertical bar */
    { 0xA7, 0, 0x00A7 }, /* Section sign */
    { 0xA8, 0, 0x00A8 }, /* Diaeresis */
    { 0xA9, 0, 0x00A9 }, /* Copyright sign */
    { 0xAA, 0, 0x00AA }, /* Feminine ordinal indicator */
    { 0xAB, 0, 0x00AB }, /* Left-pointing double angle quotation mark */
    { 0xAC, 0, 0x00AC }, /* Not sign */
    { 0xAD, 0, 0x00AD }, /* Soft hyphen */
    { 0xAE, 0, 0x00AE }, /* Registered sign */
    { 0xAF, 0, 0x00AF }, /* Macron */
    { 0xB0, 0, 0x00B0 }, /* Degree sign */
    { 0xB1, 0, 0x00B1 }, /* Plus-minus sign */
    { 0xB2, 0, 0x00B2 }, /* Superscript two */
    { 0xB3, 0, 0x00B3 }, /* Superscript three */
    { 0xB4, 0, 0x00B4 }, /* Acute accent */
    { 0xB5, 0, 0x00B5 }, /* Micro sign */
    { 0xB6, 0, 0x00B6 }, /* Pilcrow sign */
    { 0xB7, 0, 0x00B7 }, /* Middle dot */
    { 0xB8, 0, 0x00B8 }, /* Cedilla */
    { 0xB9, 0, 0x00B9 }, /* Superscript one */
    { 0xBA, 0, 0x00BA }, /* Masculine ordinal indicator */
    { 0xBB, 0, 0x00BB }, /* Right-pointing double angle quotation mark */
    { 0xBC, 0, 0x00BC }, /* Fraction one quarter */
    { 0xBD, 0, 0x00BD }, /* Fraction one half */
    { 0xBE, 0, 0x00BE }, /* Fraction three quarters */
    { 0xBF, 0, 0x00BF }, /* Inverted question mark */
    { 0xC0, 0, 0x00C0 }, /* Latin A With grave */
    { 0xC1, 0, 0x00C1 }, /* Latin A With acute */
    { 0xC2, 0, 0x00C2 }, /* Latin A With circumflex */
    { 0xC3, 0, 0x00C3 }, /* Latin A With tilde */
    { 0xC4, 0, 0x00C4 }, /* Latin A With diaeresis */
    { 0xC5, 0, 0x00C5 }, /* Latin A With ring above */
    { 0xC6, 0, 0x00C6 }, /* Latin AE */
    { 0xC7, 0, 0x00C7 }, /* Latin C with cedilla */
    { 0xC8, 0, 0x00C8 }, /* Latin E with grave */
    { 0xC9, 0, 0x00C9 }, /* Latin E with acute */
    { 0xCA, 0, 0x00CA }, /* Latin E with circumflex */
    { 0xCB, 0, 0x00CB }, /* Latin E with diaeresis */
    { 0xCC, 0, 0x00CC }, /* Latin I with grave */
    { 0xCD, 0, 0x00CD }, /* Latin I with acute */
    { 0xCE, 0, 0x00CE }, /* Latin I with circumflex */
    { 0xCF, 0, 0x00CF }, /* Latin I with diaeresis */
    { 0xD0, 0, 0x00D0 }, /* Latin CAPITAL LETTER ETH (Icelandic) */
    { 0xD1, 0, 0x00D1 }, /* Latin CAPITAL LETTER N WITH TILDE */
    { 0xD2, 0, 0x00D2 }, /* Latin CAPITAL LETTER O WITH GRAVE */
    { 0xD3, 0, 0x00D3 }, /* Latin CAPITAL LETTER O WITH ACUTE */
    { 0xD4, 0, 0x00D4 }, /* Latin CAPITAL LETTER O WITH CIRCUMFLEX */
    { 0xD5, 0, 0x00D5 }, /* Latin CAPITAL LETTER O WITH TILDE */
    { 0xD6, 0, 0x00D6 }, /* Latin CAPITAL LETTER O WITH DIAERESIS */
    { 0xD7, 0, 0x00D7 }, /* MULTIPLICATION SIGN */
    { 0xD8, 0, 0x00D8 }, /* Latin CAPITAL LETTER O WITH STROKE */
    { 0xD9, 0, 0x00D9 }, /* Latin CAPITAL LETTER U WITH GRAVE */
    { 0xDA, 0, 0x00DA }, /* Latin CAPITAL LETTER U WITH ACUTE */
    { 0xDB, 0, 0x00DB }, /* Latin CAPITAL LETTER U WITH CIRCUMFLEX */
    { 0xDC, 0, 0x00DC }, /* Latin CAPITAL LETTER U WITH DIAERESIS */
    { 0xDD, 0, 0x00DD }, /* Latin CAPITAL LETTER Y WITH ACUTE */
    { 0xDE, 0, 0x00DE }, /* Latin CAPITAL LETTER THORN (Icelandic) */
    { 0xDF, 0, 0x00DF }, /* Latin SHARP S (German) */
    { 0xE0, 0, 0x00E0 }, /* Latin A WITH GRAVE */
    { 0xE1, 0, 0x00E1 }, /* Latin A WITH ACUTE */
    { 0xE2, 0, 0x00E2 }, /* Latin A WITH CIRCUMFLEX */
    { 0xE3, 0, 0x00E3 }, /* Latin A WITH TILDE */
    { 0xE4, 0, 0x00E4 }, /* Latin A WITH DIAERESIS */
    { 0xE5, 0, 0x00E5 }, /* Latin A WITH RING ABOVE */
    { 0xE6, 0, 0x00E6 }, /* Latin AE */
    { 0xE7, 0, 0x00E7 }, /* Latin C WITH CEDILLA */
    { 0xE8, 0, 0x00E8 }, /* Latin E WITH GRAVE */
    { 0xE9, 0, 0x00E9 }, /* Latin E WITH ACUTE */
    { 0xEA, 0, 0x00EA }, /* Latin E WITH CIRCUMFLEX */
    { 0xEB, 0, 0x00EB }, /* Latin E WITH DIAERESIS */
    { 0xEC, 0, 0x00EC }, /* Latin I WITH GRAVE */
    { 0xED, 0, 0x00ED }, /* Latin I WITH ACUTE */
    { 0xEE, 0, 0x00EE }, /* Latin I WITH CIRCUMFLEX */
    { 0xEF, 0, 0x00EF }, /* Latin I WITH DIAERESIS */
    { 0xF0, 0, 0x00F0 }, /* Latin ETH (Icelandic) */
    { 0xF1, 0, 0x00F1 }, /* Latin N WITH TILDE */
    { 0xF2, 0, 0x00F2 }, /* Latin O WITH GRAVE */
    { 0xF3, 0, 0x00F3 }, /* Latin O WITH ACUTE */
    { 0xF4, 0, 0x00F4 }, /* Latin O WITH CIRCUMFLEX */
    { 0xF5, 0, 0x00F5 }, /* Latin O WITH TILDE */
    { 0xF6, 0, 0x00F6 }, /* Latin O WITH DIAERESIS */
    { 0xF7, 0, 0x00F7 }, /* DIVISION SIGN */
    { 0xF8, 0, 0x00F8 }, /* Latin O WITH STROKE */
    { 0xF9, 0, 0x00F9 }, /* Latin U WITH GRAVE */
    { 0xFA, 0, 0x00FA }, /* Latin U WITH ACUTE */
    { 0xFB, 0, 0x00FB }, /* Latin U WITH CIRCUMFLEX */
    { 0xFC, 0, 0x00FC }, /* Latin U WITH DIAERESIS */
    { 0xFD, 0, 0x00FD }, /* Latin Y WITH ACUTE */
    { 0xFE, 0, 0x00FE }, /* Latin THORN (Icelandic) */
    { 0xFF, 0, 0x00FF }, /* Latin Y WITH DIAERESIS */
};

/*
 * The following table maps the gsm7 alphabet to the corresponding unicode
 * characters. This table is exactly 128 entries large.
 */
const AT_PB_ConversionTable g_gsmToUnicode[AT_PB_GSM_MAX_NUM] = {
    { 0x00, 0, 0x0040 }, /* COMMERCIAL AT */
    { 0x01, 0, 0x00A3 }, /* POUND SIGN */
    { 0x02, 0, 0x0024 }, /* DOLLAR SIGN */
    { 0x03, 0, 0x00A5 }, /* YEN SIGN */
    { 0x04, 0, 0x00E8 }, /* Latin E WITH GRAVE */
    { 0x05, 0, 0x00E9 }, /* Latin E WITH ACUTE */
    { 0x06, 0, 0x00F9 }, /* Latin U WITH GRAVE */
    { 0x07, 0, 0x00EC }, /* Latin I WITH GRAVE */
    { 0x08, 0, 0x00F2 }, /* Latin O WITH GRAVE */
    { 0x09, 0, 0x00E7 }, /* Latin C WITH CEDILLA */
    { 0x0A, 0, 0x000A }, /* LINE FEED */
    { 0x0B, 0, 0x00D8 }, /* Latin CAPITAL LETTER O WITH STROKE */
    { 0x0C, 0, 0x00F8 }, /* Latin O WITH STROKE */
    { 0x0D, 0, 0x000D }, /* CARRIAGE RETURN */
    { 0x0E, 0, 0x00C5 }, /* Latin CAPITAL LETTER A WITH RING ABOVE */
    { 0x0F, 0, 0x00E5 }, /* Latin A WITH RING ABOVE */
    { 0x10, 0, 0x0394 }, /* GREEK CAPITAL LETTER DELTA */
    { 0x11, 0, 0x005F }, /* LOW LINE */
    { 0x12, 0, 0x03A6 }, /* GREEK CAPITAL LETTER PHI */
    { 0x13, 0, 0x0393 }, /* GREEK CAPITAL LETTER GAMMA */
    { 0x14, 0, 0x039B }, /* GREEK CAPITAL LETTER LAMDA */
    { 0x15, 0, 0x03A9 }, /* GREEK CAPITAL LETTER OMEGA */
    { 0x16, 0, 0x03A0 }, /* GREEK CAPITAL LETTER PI */
    { 0x17, 0, 0x03A8 }, /* GREEK CAPITAL LETTER PSI */
    { 0x18, 0, 0x03A3 }, /* GREEK CAPITAL LETTER SIGMA */
    { 0x19, 0, 0x0398 }, /* GREEK CAPITAL LETTER THETA */
    { 0x1A, 0, 0x039E }, /* GREEK CAPITAL LETTER XI */
    { 0x1B, 0, 0x00A0 }, /* ESCAPE TO EXTENSION TABLE */
    { 0x1C, 0, 0x00C6 }, /* Latin CAPITAL LETTER AE */
    { 0x1D, 0, 0x00E6 }, /* Latin AE */
    { 0x1E, 0, 0x00DF }, /* Latin SHARP S (German) */
    { 0x1F, 0, 0x00C9 }, /* Latin CAPITAL LETTER E WITH ACUTE */
    { 0x20, 0, 0x0020 }, /* SPACE */
    { 0x21, 0, 0x0021 }, /* EXCLAMATION MARK */
    { 0x22, 0, 0x0022 }, /* QUOTATION MARK */
    { 0x23, 0, 0x0023 }, /* NUMBER SIGN */
    { 0x24, 0, 0x00A4 }, /* CURRENCY SIGN */
    { 0x25, 0, 0x0025 }, /* PERCENT SIGN */
    { 0x26, 0, 0x0026 }, /* AMPERSAND */
    { 0x27, 0, 0x0027 }, /* APOSTROPHE */
    { 0x28, 0, 0x0028 }, /* LEFT PARENTHESIS */
    { 0x29, 0, 0x0029 }, /* RIGHT PARENTHESIS */
    { 0x2A, 0, 0x002A }, /* ASTERISK */
    { 0x2B, 0, 0x002B }, /* PLUS SIGN */
    { 0x2C, 0, 0x002C }, /* COMMA */
    { 0x2D, 0, 0x002D }, /* HYPHEN-MINUS */
    { 0x2E, 0, 0x002E }, /* FULL STOP */
    { 0x2F, 0, 0x002F }, /* SOLIDUS */
    { 0x30, 0, 0x0030 }, /* DIGIT ZERO */
    { 0x31, 0, 0x0031 }, /* DIGIT ONE */
    { 0x32, 0, 0x0032 }, /* DIGIT TWO */
    { 0x33, 0, 0x0033 }, /* DIGIT THREE */
    { 0x34, 0, 0x0034 }, /* DIGIT FOUR */
    { 0x35, 0, 0x0035 }, /* DIGIT FIVE */
    { 0x36, 0, 0x0036 }, /* DIGIT SIX */
    { 0x37, 0, 0x0037 }, /* DIGIT SEVEN */
    { 0x38, 0, 0x0038 }, /* DIGIT EIGHT */
    { 0x39, 0, 0x0039 }, /* DIGIT NINE */
    { 0x3A, 0, 0x003A }, /* COLON */
    { 0x3B, 0, 0x003B }, /* SEMICOLON */
    { 0x3C, 0, 0x003C }, /* LESS-THAN SIGN */
    { 0x3D, 0, 0x003D }, /* EQUALS SIGN */
    { 0x3E, 0, 0x003E }, /* GREATER-THAN SIGN */
    { 0x3F, 0, 0x003F }, /* QUESTION MARK */
    { 0x40, 0, 0x00A1 }, /* INVERTED EXCLAMATION MARK */
    { 0x41, 0, 0x0041 }, /* Latin CAPITAL LETTER A */
    { 0x42, 0, 0x0042 }, /* Latin CAPITAL LETTER B */
    { 0x43, 0, 0x0043 }, /* Latin CAPITAL LETTER C */
    { 0x44, 0, 0x0044 }, /* Latin CAPITAL LETTER D */
    { 0x45, 0, 0x0045 }, /* Latin CAPITAL LETTER E */
    { 0x46, 0, 0x0046 }, /* Latin CAPITAL LETTER F */
    { 0x47, 0, 0x0047 }, /* Latin CAPITAL LETTER G */
    { 0x48, 0, 0x0048 }, /* Latin CAPITAL LETTER H */
    { 0x49, 0, 0x0049 }, /* Latin CAPITAL LETTER I */
    { 0x4A, 0, 0x004A }, /* Latin CAPITAL LETTER J */
    { 0x4B, 0, 0x004B }, /* Latin CAPITAL LETTER K */
    { 0x4C, 0, 0x004C }, /* Latin CAPITAL LETTER L */
    { 0x4D, 0, 0x004D }, /* Latin CAPITAL LETTER M */
    { 0x4E, 0, 0x004E }, /* Latin CAPITAL LETTER N */
    { 0x4F, 0, 0x004F }, /* Latin CAPITAL LETTER O */
    { 0x50, 0, 0x0050 }, /* Latin CAPITAL LETTER P */
    { 0x51, 0, 0x0051 }, /* Latin CAPITAL LETTER Q */
    { 0x52, 0, 0x0052 }, /* Latin CAPITAL LETTER R */
    { 0x53, 0, 0x0053 }, /* Latin CAPITAL LETTER S */
    { 0x54, 0, 0x0054 }, /* Latin CAPITAL LETTER T */
    { 0x55, 0, 0x0055 }, /* Latin CAPITAL LETTER U */
    { 0x56, 0, 0x0056 }, /* Latin CAPITAL LETTER V */
    { 0x57, 0, 0x0057 }, /* Latin CAPITAL LETTER W */
    { 0x58, 0, 0x0058 }, /* Latin CAPITAL LETTER X */
    { 0x59, 0, 0x0059 }, /* Latin CAPITAL LETTER Y */
    { 0x5A, 0, 0x005A }, /* Latin CAPITAL LETTER Z */
    { 0x5B, 0, 0x00C4 }, /* Latin CAPITAL LETTER A WITH DIAERESIS */
    { 0x5C, 0, 0x00D6 }, /* Latin CAPITAL LETTER O WITH DIAERESIS */
    { 0x5D, 0, 0x00D1 }, /* Latin CAPITAL LETTER N WITH TILDE */
    { 0x5E, 0, 0x00DC }, /* Latin CAPITAL LETTER U WITH DIAERESIS */
    { 0x5F, 0, 0x00A7 }, /* SECTION SIGN */
    { 0x60, 0, 0x00BF }, /* INVERTED QUESTION MARK */
    { 0x61, 0, 0x0061 }, /* Latin A */
    { 0x62, 0, 0x0062 }, /* Latin B */
    { 0x63, 0, 0x0063 }, /* Latin C */
    { 0x64, 0, 0x0064 }, /* Latin D */
    { 0x65, 0, 0x0065 }, /* Latin E */
    { 0x66, 0, 0x0066 }, /* Latin F */
    { 0x67, 0, 0x0067 }, /* Latin G */
    { 0x68, 0, 0x0068 }, /* Latin H */
    { 0x69, 0, 0x0069 }, /* Latin I */
    { 0x6A, 0, 0x006A }, /* Latin J */
    { 0x6B, 0, 0x006B }, /* Latin K */
    { 0x6C, 0, 0x006C }, /* Latin L */
    { 0x6D, 0, 0x006D }, /* Latin M */
    { 0x6E, 0, 0x006E }, /* Latin N */
    { 0x6F, 0, 0x006F }, /* Latin O */
    { 0x70, 0, 0x0070 }, /* Latin P */
    { 0x71, 0, 0x0071 }, /* Latin Q */
    { 0x72, 0, 0x0072 }, /* Latin R */
    { 0x73, 0, 0x0073 }, /* Latin S */
    { 0x74, 0, 0x0074 }, /* Latin T */
    { 0x75, 0, 0x0075 }, /* Latin U */
    { 0x76, 0, 0x0076 }, /* Latin V */
    { 0x77, 0, 0x0077 }, /* Latin W */
    { 0x78, 0, 0x0078 }, /* Latin X */
    { 0x79, 0, 0x0079 }, /* Latin Y */
    { 0x7A, 0, 0x007A }, /* Latin Z */
    { 0x7B, 0, 0x00E4 }, /* Latin A WITH DIAERESIS */
    { 0x7C, 0, 0x00F6 }, /* Latin O WITH DIAERESIS */
    { 0x7D, 0, 0x00F1 }, /* Latin N WITH TILDE */
    { 0x7E, 0, 0x00FC }, /* Latin U WITH DIAERESIS */
    { 0x7F, 0, 0x00E0 }, /* Latin A WITH GRAVE */
};

/*
 * GSM7BIT 向UCS2转换的扩展表,GSM7BIT扩展表扩展标志为0x1b；如下表当GSM7BIT编码中
 * 出现码字0x1B65时，其对应的UCS2编码为0x20AC，此码字代表欧元符
 */
const AT_PB_ConversionTable g_gsm7extToUnicode[AT_PB_GSM7EXT_MAX_NUM] = {
    { 0x65, 0, 0x20AC }, /* EURO SIGN */
    { 0x0A, 0, 0x000C }, /* FORM FEED */
    { 0x14, 0, 0x005E }, /* CIRCUMFLEX ACCENT */
    { 0x28, 0, 0x007B }, /* LEFT CURLY BRACKET */
    { 0x29, 0, 0x007D }, /* RIGHT CURLY BRACKET */
    { 0x2F, 0, 0x005C }, /* REVERSE SOLIDUS */
    { 0x3C, 0, 0x005B }, /* LEFT SQUARE BRACKET */
    { 0x3D, 0, 0x007E }, /* TILDE */
    { 0x3E, 0, 0x005D }, /* RIGHT SQUARE BRACKET */
    { 0x40, 0, 0x007C }, /* VERTICAL LINE */
};

const AT_STRING_Type g_atStringTab[] = {
    { AT_STRING_SM, (TAF_UINT8 *)"\"SM\"" },
    { AT_STRING_ME, (TAF_UINT8 *)"\"ME\"" },
    { AT_STRING_ON, (TAF_UINT8 *)"\"ON\"" },
    { AT_STRING_EN, (TAF_UINT8 *)"\"EN\"" },
    { AT_STRING_FD, (TAF_UINT8 *)"\"FD\"" },
    { AT_STRING_BD, (TAF_UINT8 *)"\"BD\"" },
    { AT_STRING_REC_UNREAD_TEXT, (TAF_UINT8 *)"\"REC UNREAD\"" },
    { AT_STRING_REC_READ_TEXT, (TAF_UINT8 *)"\"REC READ\"" },
    { AT_STRING_STO_UNSENT_TEXT, (TAF_UINT8 *)"\"STO UNSENT\"" },
    { AT_STRING_STO_SENT_TEXT, (TAF_UINT8 *)"\"STO SENT\"" },
    { AT_STRING_ALL_TEXT, (TAF_UINT8 *)"\"ALL\"" },
    { AT_STRING_REC_UNREAD_PDU, (TAF_UINT8 *)"0" },
    { AT_STRING_REC_READ_PDU, (TAF_UINT8 *)"1" },
    { AT_STRING_STO_UNSENT_PDU, (TAF_UINT8 *)"2" },
    { AT_STRING_STO_SENT_PDU, (TAF_UINT8 *)"3" },
    { AT_STRING_ALL_PDU, (TAF_UINT8 *)"4" },
    { AT_STRING_IP, (TAF_UINT8 *)"\"IP\"" },
    { AT_STRING_IPv4, (VOS_UINT8 *)"\"IPv4\"" },
    { AT_STRING_PPP, (TAF_UINT8 *)"\"PPP\"" },
    { AT_STRING_IPV6, (VOS_UINT8 *)"\"IPV6\"" },
    { AT_STRING_IPV4V6, (VOS_UINT8 *)"\"IPV4V6\"" },
    { AT_STRING_IPv6, (VOS_UINT8 *)"\"IPv6\"" },
    { AT_STRING_IPv4v6, (VOS_UINT8 *)"\"IPv4v6\"" },
    { AT_STRING_Ethernet, (VOS_UINT8 *)"\"Ethernet\"" },

    { AT_STRING_0E0, (TAF_UINT8 *)"\"0E0\"" },
    { AT_STRING_1E2, (TAF_UINT8 *)"\"1E2\"" },
    { AT_STRING_7E3, (TAF_UINT8 *)"\"7E3\"" },
    { AT_STRING_1E3, (TAF_UINT8 *)"\"1E3\"" },
    { AT_STRING_1E4, (TAF_UINT8 *)"\"1E4\"" },
    { AT_STRING_1E5, (TAF_UINT8 *)"\"1E5\"" },
    { AT_STRING_1E6, (TAF_UINT8 *)"\"1E6\"" },
    { AT_STRING_1E1, (TAF_UINT8 *)"\"1E1\"" },
    { AT_STRING_5E2, (TAF_UINT8 *)"\"5E2\"" },
    { AT_STRING_5E3, (TAF_UINT8 *)"\"5E3\"" },
    { AT_STRING_4E3, (TAF_UINT8 *)"\"4E3\"" },
    { AT_STRING_6E8, (TAF_UINT8 *)"\"6E8\"" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: CS域状态信息主动上报
     * [说明]: CS域状态信息主动上报。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: 网络注册的状态发生改变时：
     *             <CR><LF>+CREG: <stat><CR><LF>
     *     [命令]: -
     *     [结果]: 小区信息发生改变时：
     *             <CR><LF>+CREG: <stat>[,<lac>,<ci>[,<AcT>]]<CR><LF>
     * [参数]:
     *     <n>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CREG的主动上报；
     *             1：使能+CREG: <stat>的主动上报；
     *             2：使能+CREG: <stat>[,<lac>,<ci>[,<AcT>]]的主动上报。
     *     <stat>: 整型值，注册状态。
     *             0：没有注册，MT没有处于搜索和注册新的运营商状态；
     *             1：注册了归属网络；
     *             2：没有注册，MT正在搜索并注册新的运营商；
     *             3：注册被拒绝；
     *             4：未知状态；
     *             5：注册了漫游网络。
     *     <lac>: 2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *     <ci>: 4byte的16进制数的字符串类型，小区信息。
     * [示例]:
     *       网络注册的状态发生改变时
     *       +CREG: 1
     *       小区信息发生改变时
     *       +CREG: 1,"00C3","0000001A",4
     */
    { AT_STRING_CREG, (TAF_UINT8 *)"+CREG: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: PS域状态信息主动上报
     * [说明]: PS域状态信息主动上报。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: 网络注册的状态发生改变时：
     *             <CR><LF>+CGREG: <stat><CR><LF>
     *     [命令]: -
     *     [结果]: 小区信息发生改变时：
     *             <CR><LF>+CGREG: <stat>[,<lac>,<ci>[,<AcT>]]<CR><LF>
     * [参数]:
     *     <n>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CGREG的主动上报；
     *             1：使用+CGREG: <stat>的主动上报方式；
     *             2：使用+CGREG: <stat>[,<lac>,<ci>[,<AcT>]]的主动上报方式。
     *     <stat>: 整型值，注册状态。
     *             0：没有注册，MT现在没有搜索和注册新的运营商；
     *             1：注册了归属网络；
     *             2：没有注册，MT正在搜索并注册新的运营商；
     *             3：注册被拒绝；
     *             4：未知状态；
     *             5：注册了漫游网络。
     *     <lac>: 2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *     <ci>: 4byte的16进制数的字符串类型，小区信息。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             0：GSM；
     *             1：GSM Compact；
     *             2：UTRAN；
     *             3：GSM EGPRS；
     *             4：UTRAN HSDPA；
     *             5：UTRAN HSUPA；
     *             6：UTRAN HSDPA和HSUPA；
     *             7：E-UTRAN；
     *             10：EUTRAN-5GC；
     *             11：NR-5GC；
     * [示例]:
     *       网络注册的状态发生改变时
     *       +CGREG: 1
     *       小区信息发生改变时
     *       +CGREG: 1,"00C3","0000001A",4
     */
    { AT_STRING_CGREG, (TAF_UINT8 *)"+CGREG: " },
#if (FEATURE_LTE == FEATURE_ON)
    { AT_STRING_CEREG, (TAF_UINT8 *)"+CEREG: " },
#endif
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 指示服务状态变化
     * [说明]: 当服务状态发生变化时，MT主动上报新的服务状态给TE。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SRVST: <srv_status><CR><LF>
     * [参数]:
     *     <srv_status>: 整数型，系统服务状态，取值0～4。
     *             0：无服务；
     *             1：有限制服务；
     *             2：服务有效；
     *             3：有限制的区域服务；
     *             4：省电和深睡状态。
     * [示例]:
     *       服务状态改变为2时
     *       ^SRVST: 2
     */
    { AT_STRING_SRVST, (TAF_UINT8 *)"^SRVST: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 指示系统模式变化事件
     * [说明]: 当系统模式变化时，MT主动上报此指示给TE。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^MODE: <sys_mode>,<sys_submode><CR><LF>
     * [参数]:
     *     <sys_mode>: 整数型，系统模式。
     *             0：无服务；
     *             3：GSM/GPRS 模式；
     *             5：WCDMA模式；
     *             6：GPS模式（不支持）；
     *             7：LTE 模式；
     *             10：EUTUAN-5GC;
     *             11：NR-5GC;
     *             15：TD_SCDMA模式;
     *     <sys_submode>: 整数型，系统子模式。
     *             0：无服务；
     *             1：GSM模式；
     *             2：GPRS模式；
     *             3：EDGE模式；
     *             4：WCDMA模式；
     *             5：HSDPA模式；
     *             6：HSUPA mode；
     *             7：HSDPA mode and HSUPA mode；
     *             8：TD_SCDMA模式；
     *             9：HSPA+ 模式；
     *             10：LTE 模式；
     *             17：DC-HSPA+模式；
     *             18：MIMO-HSPA+ 模式；
     *             110：EUTUAN-5GC;
     *             111：NR-5GC。
     * [示例]:
     *       用户设置模式上报AT^MODE=1。如发生了如下模式变化：G->L, G->W,W->L等，^MODE上报模式变化事件。
     *       系统模式变化为WCDMA单模时
     *       ^MODE: 5,4
     *       当前工作在WCDMA模式下后驻留到TD-SCDMA模式
     *       ^MODE: 15,8
     *       当前工作在TD-SCDMA模式下后驻留到WCDMA模式下
     *       ^MODE: 5,4
     *       当前工作在GUL模式下切换L时，需要将L模显示为W
     *       ^MODE: 5,4
     */
    { AT_STRING_MODE, (TAF_UINT8 *)"^MODE: " },
    { AT_STRING_RSSI, (TAF_UINT8 *)"^RSSI: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 网络时间上报格式
     * [说明]: 本命令规定网络时间的上报格式。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^TIME: < date >,< time>,<dt><CR><LF>
     * [参数]:
     *     <date>: 日期和时间信息，格式为yy/mm/dd。
     *     <time>: 格式为"hh:mm:ss(+/-)tz"。<time>字段信息由两部分组成：时间和时区，比如“05:56:13+32”。tz:时区信息，时区单位为15分钟,  +32 表示32x15分钟 ＝ + 8小时。
     *     <dt>: 夏令时，当网络没有下发此参数时单板上报0，否则上报对应的夏令时。具体取值及其代表的意义参考table 10.5.97a/3GPP TS 24.008
     * [示例]:
     *       上报网络时间
     *       ^TIME: "12/06/06,09:02:22+32,00"
     */
    { AT_STRING_TIME, (TAF_UINT8 *)"^TIME: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 主动上报网络时区信息
     * [说明]: 网络下发NITZ (Network Identity and Time Zone)通知终端当前网络的时区及时间等信息，终端通过此命令将网络的时区上报给应用。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CTZV: < tz ><CR><LF>
     * [参数]:
     *     <tz>: 整型值，时区信息。范围：-48~+56。
     *             时区信息值被固定为2位数字表示，不足2位的用0补位。如-9输出时为-09；+0输出时为+00。
     * [示例]:
     *       收到网侧下发的NITZ信息
     *       +CTZV: "+15"
     */
    { AT_STRING_CTZV, (TAF_UINT8 *)"+CTZV: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 主动上报网络时区和时间信息
     * [说明]: 网络下发NITZ(Network Identity and Time Zone)通知终端当前网络的时区及时间信息，终端通过此命令将网络的时区，夏令时制和时间信息上报给应用。
     *         本命令遵从3GPP TS 27.007 协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CTZE: <tz>,<dst>,[<time>] <CR><LF>
     * [参数]:
     *     <tz>: 整型值，时区信息。范围：-48~+56。
     *             时区信息值被固定为2位数字表示，不足2位的用0补位。如-9输出时为-09；+0输出时为+00。
     *     <dst>: 0：无夏令时。
     *             1：设置了夏令时，<tz>中有+1小时(相当于tz值加了4)的夏令时时间调节。
     *             2：设置了夏令时，<tz>中有+2小时(相当于tz值加了8)的夏令时时间调节。
     *     <time>: 时间信息字符串。
     *             格式为yyyy/mm/dd,hh:mm:ss。
     * [示例]:
     *       收到网侧下发的NITZ信息
     *       +CTZE: "+02",1,"2015/12/01,06:25:00"
     */
    { AT_STRING_CTZE, (TAF_UINT8 *)"+CTZE: " },
    { AT_STRING_ERRRPT, (TAF_UINT8 *)"^ERRRPT:" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 上报呼叫状态
     * [说明]: 使能^CURC主动上报控制命令，呼叫过程中，主动上报呼叫状态的变化。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CCALLSTATE: <call_id>,<state>，<voice_domain><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *     <state>: 整数型，指示对应call_id的呼叫状态。
     *             0：Call originate(MO)，向网络发起呼叫；
     *             1：Call is Proceeding，呼叫发起，网络已经开始处理；
     *             2：Alerting，振铃；
     *             3：Connected，呼叫已经接通；
     *             4：Released，呼叫被释放；
     *             5：InComing，有来电；
     *             6：Waiting，有一个等待的来电；
     *             7：Hold，呼叫保持；
     *             8：Retrieve，呼叫恢复。
     *     <voice_domain>: 整型值，语音电话的呼叫域。
     *             0：3GPP CS域语音电话
     *             1：IMS域语音电话
     *             2：3GPP2 CS域语音电话
     *             3：未在任何域上发起电话
     * [示例]:
     *       有IMS域语音来电
     *       ^CCALLSTATE: 1,5,1
     */
    { AT_STRING_CCALLSTATE, (VOS_UINT8 *)"^CCALLSTATE" },

    { AT_STRING_CERSSI, (VOS_UINT8 *)"^CERSSI:" },

    { AT_STRING_ACINFO, (VOS_UINT8 *)"^ACINFO:" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 主动上报语音带宽信息
     * [说明]: 该命令用于在通话过程中主动上报语音带宽的状态。
     *         如果网络发送ALERTING消息，此时不能网络放音（网络放音的要求：A progress indicator IE indicates user attachment if it specifies a progress description in the set {1, 2, 3} or in the set {6, 7, 8, ..., 20}. ），此时通知AP侧进行本地振铃放音。
     *         如果在本地振铃过程中，又收到了网络通知可以进行放音，即又有空口消息携带了progress indicator，并且 progress description的值为{1, 2, 3}或者{6, 7, 8, ..., 20}，则通知AP侧网络振铃，此时AP侧停止本地的振铃。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CSCHANNELINFO: <status>,<voice_domain><CR><LF>
     * [参数]:
     *     <status>: 0：振铃，无带内音信息；
     *             1：带内音可用，窄带语音；
     *             2：带内音可用，宽带语音；
     *             3：带内音可用，EVS NB语音；
     *             4：带内音可用，EVS WB语音；
     *             5：带内音可用，EVS SWB语音。
     *     <voice_domain>: 整型值，语音电话的呼叫域。
     *             0：CS域语音电话
     *             1：IMS域语音电话
     * [示例]:
     *       CS域语音电话，振铃无带内信息，主动上报如下：
     *       ^ CSCHANNELINFO: 0,0
     *       CS域语音电话，通话中宽带语音变为窄带语音时，主动上报如下：
     *       ^ CSCHANNELINFO: 1,0
     *       IMS域语音电话，当前语音处于宽带语音时，主动上报如下：
     *       ^ CSCHANNELINFO: 2,1
     */
    { AT_STRING_CS_CHANNEL_INFO, (VOS_UINT8 *)"^CSCHANNELINFO:" },

    { AT_STRING_RESET, (VOS_UINT8 *)"^RESET:" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: GPS参考时钟状态上报
     * [说明]: 该命令用于AP-Modem形态下主动上报GPS参考时钟状态，以避免不确定的参考时钟状态的影响。
     * [语法]:
     *     [命令]: -
     *     [结果]: GPS参考时钟状态发生改变时：
     *             <CR><LF>^REFCLKFREQ: <ver>,<freq>,<precision> <status><CR><LF>
     * [参数]:
     *     <ver>: 整型值，本命令的版本ID，当前为0。
     *     <freq>: 整型值，GPS参考时钟的频率值，单位Hz。
     *     <precision>: 整型值，当前GPS参考时钟的精度，单位ppb。
     *     <status>: 整型值，GPS参考时钟频率锁定状态：
     *             0：未锁定；
     *             1：锁定。
     * [示例]:
     *       GPS参考时钟频率变为锁定状态
     *       ^REFCLKFREQ: 0,19200000,100,1
     */
    { AT_STRING_REFCLKFREQ, (VOS_UINT8 *)"^REFCLKFREQ" },

    { AT_STRING_C5GNSSAA, (VOS_UINT8 *)"^C5GNSSAA" },

    { AT_STRING_CPENDINGNSSAI, (VOS_UINT8 *)"^CPENDINGNSSAI" },

    { AT_STRING_REJINFO, (VOS_UINT8 *)"^REJINFO: " },

    { AT_STRING_PLMNSELEINFO, (VOS_UINT8 *)"^PLMNSELEINFO:" },

    { AT_STRING_NETSCAN, (VOS_UINT8 *)"^NETSCAN: " },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报SRVCC状态
     * [说明]: 主动上报SRVCC状态，能否主动上报受+CIREP命令控制。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CIREPH: <srvcch><CR><LF>
     * [参数]:
     *     <srvcch>: 整型值，SRVCC状态。
     *             0：SRVCC开始；
     *             1：SRVCC成功；
     *             2：SRVCC取消；
     *             3：SRVCC失败。
     * [示例]:
     *       SRVCC状态主动上报
     *       +CIREPH: 1
     */
    { AT_STRING_CIREPH, (VOS_UINT8 *)"+CIREPH" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报IMSVOPS指示
     * [说明]: 主动上报IMSVOPS指示，能否主动上报受+CIREP命令控制。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CIREPI: <nwimsvops><CR><LF>
     * [参数]:
     *     <nwimsvops>: 整型值，网络是否支持IMSVOPS的指示。
     *             0：网络不支持IMSVOPS；
     *             1：网络支持IMSVOPS。
     * [示例]:
     *       网络支持IMSVOPS指示主动上报
     *       +CIREPI: 1
     */
    { AT_STRING_CIREPI, (VOS_UINT8 *)"+CIREPI" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报IMS域注册状态
     * [说明]: 主动上报当前IMS域网络注册状态，能否主动上报受+CIREG命令控制。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CIREGU: <reg_info>[,<ext_info>]<CR><LF>
     * [参数]:
     *     <reg_info>: 整型值，注册状态。
     *             0：没有注册；
     *             1：注册了IMS网络。
     *     <ext_info>: 整型值，IMS域能力值，取值范围[1,0xFFFFFFFF]，每个bit位表示不同的能力。
     *             0：+CIREG=2时 ，扩展信息暂时回复0
     *             1：RTP-based transfer of voice
     *             2：RTP-based transfer of text
     *             4：SMS using IMS functionality
     *             8：RTP-based transfer of video
     *             例如：=5的时候，表示支持RTP-based transfer of voice和SMS using IMS functionality。
     * [示例]:
     *       设置+CIREG=2时，IMS域注册状态上报
     *       +CIREGU: 1,0
     *       OK
     */
    { AT_STRING_CIREGU, (VOS_UINT8 *)"+CIREGU" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 音视频通话类型变更通知
     * [说明]: 音视频通话类型变更通知。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^MODIFYIND: <idx>,<curr_call_type>,<curr_call_domain>,<dest_call_type>,<dest_call_domain>,<modify_reason><CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫ID，取值范围1~7
     *     <curr_call_type>: 整型值，当前呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <curr_call_domain>: 整型值，当前呼叫域
     *             0： CS域
     *             1： PS域
     *     <dest_call_type>: 整型值，目标呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <dest_call_domain>: 整型值，目标呼叫域
     *             0： CS域
     *             1： PS域
     *     <modify_reason>: 整型值，modify事件发起原因：
     *             0：远程用户正常发起modify事件；
     *             1：远程用户因RTP或者RTCP超时发起modify事件；
     *             2：远程用户因RTP和RTCP同时超时发起modify事件。
     * [示例]:
     *       音频切视频通话变更通知
     *       ^CALLMODIFYIND: 1,0,1,3,1,0
     */
    { AT_STRING_CALL_MODIFY_IND, (VOS_UINT8 *)"^CALLMODIFYIND:" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 音视频通话类型变更开始通知
     * [说明]: 音视频通话类型变更开始通知。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CALLMODIFYBEG: <idx>,<callDomain><CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫ID，取值范围1~7
     *     <callDomain>: 整型值，呼叫域
     *             0： CS域
     *             1： PS域
     * [示例]:
     *       音视频通话类型变更开始通知
     *       ^CALLMODIFYBEG: 1,1
     */
    { AT_STRING_CALL_MODIFY_BEG, (VOS_UINT8 *)"^CALLMODIFYBEG:" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 音视频通话类型变更结束通知
     * [说明]: 音视频通话类型变更结束通知。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CALLMODIFYEND: <idx>,<callDomain>,<err><CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫ID，取值范围1~7
     *     <callDomain>: 整型值，呼叫域
     *             0： CS域
     *             1： PS域
     *     <err>: 错误原因值(同CS域语言呼叫错误原因值)
     * [示例]:
     *       音视频通话类型变更结束通知
     *       ^CALLMODIFYEND: 1,1,1
     */
    { AT_STRING_CALL_MODIFY_END, (VOS_UINT8 *)"^CALLMODIFYEND:" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 主动上报ECONF与会者状态发生改变
     * [说明]: 用于主动上报增强型多方通话会议的与会者状态发生改变。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ECONFSTATE: <call_num><CR><LF>
     * [参数]:
     *     <call_num>: 呼叫个数
     * [示例]:
     *       主动上报增强型多方通话的与会者状态发生改变
     *       ^ECONFSTATE: 1
     */
    { AT_STRING_ECONFSTATE, (VOS_UINT8 *)"^ECONFSTATE:" },

#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { AT_STRING_CDISP, (TAF_UINT8 *)"^CDISP: " },
    { AT_STRING_CCONNNUM, (TAF_UINT8 *)"^CCONNNUM: " },
    { AT_STRING_CCALLEDNUM, (TAF_UINT8 *)"^CCALLEDNUM: " },
    { AT_STRING_CCALLINGNUM, (TAF_UINT8 *)"^CCALLINGNUM: " },
    { AT_STRING_CREDIRNUM, (TAF_UINT8 *)"^CREDIRNUM: " },
    { AT_STRING_CSIGTONE, (TAF_UINT8 *)"^CSIGTONE: " },
    { AT_STRING_CLCTR, (TAF_UINT8 *)"^CLCTR: " },
    { AT_STRING_CCWAC, (TAF_UINT8 *)"^CCWAC: " },
#endif
    { AT_STRING_FILECHANGE, (VOS_UINT8 *)"^FILECHANGE:" },

#if ((FEATURE_LTE == FEATURE_ON) && (FEATURE_LTE_MBMS == FEATURE_ON))
    { AT_STRING_MBMSEV, (VOS_UINT8 *)"^MBMSEV:" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 列表搜网过程中搜到PLMN的主动上报
     * [说明]: 该命令用于用户列表搜网过程中搜到PLMN的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SRCHEDPLMNLIST: <rslt1>[,<rslt2>,…,<rsltx>]<CR><LF>
     * [参数]:
     *     <rslt>: 上报的结果，每一条结果由一下元素组成：(<stat>,<operLong>,<operShort>,<operNum>,<rat>,<tac>,<rscp>)
     *     <stat>: 整型值，网络的状态标识。
     *             0：未知；
     *             1：可用；
     *             2：已注册；
     *             3：禁止。
     *     <operLong>: 字符串类型，长字符串格式的运营商信息
     *     <operShort>: 字符串类型，短字符串格式的运营商信息
     *     <operNum>: 字符串类型，数字格式的运营商信息
     *     <rat>: 整型值，无线接入技术，默认值与SYSCFG中设置的优先级相同。
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE；
     *             12: NR；
     *     <tac>: 整型值，表示区域跟踪码。
     *     <rscp>: 整型值，在NR和LTE下：代表服务小区的信号强度信息，即RSRP：NR/L-Rsrp的单位:0.125dbm, 无效值99，其余为负值
     *             W下：代表服务小区信号质量信息；即RSCP；
     *             G下：代表服务小区RSSI信息；
     * [示例]:
     *       搜网过程中搜到4个有效网络，NR,L,W,G各一个:
     *       ^SRCHEDPLMNLIST: (2,"CHINA MOBILE","CMCC","46001",12,1,-65),(1,"CHINA MOBILE","CMCC","46001",7,2,-89),(1,"CHINA MOBILE","CMCC","46001",2,3,-70),(1,"CHN-UNICOM","UNICOM","46000",0,4,-65)
     *       OK
     */
    { AT_STRING_SRCHEDPLMNLIST, (VOS_UINT8 *)"^SRCHEDPLMNLIST:" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: MCC主动上报
     * [说明]: CSS无该国家对应的频点频段信息时主动上报MCC，AP收到^MCC的上报，需要下发^MCC对应的预置频点频段信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^MCC: <VER> ,<MCC>[,MCC]<CR><LF>
     * [参数]:
     *     <VER>: 云通讯匹配频点/频段版本号
     *     <MCC>: MCC信息，可能一个，也可能多个，多个时以”逗号”隔开
     * [示例]:
     *       主动上报MCC
     *       ^MCC: 00.00.000,460,230
     */
    { AT_STRING_MCC, (TAF_UINT8 *)"^MCC:" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 上报上行定位请求错误
     * [说明]: 上行定位请求错误的上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CMOLRE: <err><CR><LF>
     * [参数]:
     *     <err>: 上行定位请求的错误码。格式既可以是数字也可以是字符串，受+CMOLRE设置命令控制。
     * [表]: +CMOLRE错误码表
     *       错误码, 错误描述,                                                 错误说明,
     *       0,      Method not supported,                                     定位方法不支持,
     *       1,      Additional assistance data required,                      需要额外的辅助信息,
     *       2,      Not enough satellites,                                    没有足够的卫星,
     *       3,      UE busy,                                                  设备繁忙,
     *       4,      Network error,                                            网络错误,
     *       5,      Failed to open internet connection, too many connections, 打开网络连接失败，过多连接,
     *       6,      Failed to open internet connection, too many users,       打开网络连接失败，用户过多,
     *       7,      Failure due to handover,                                  由于切换导致失败,
     *       8,      Internet connection failure,                              网络连接失败,
     *       9,      Memory error,                                             内存错误,
     *       255,    Unknown error,                                            未知错误,
     * [示例]:
     *       主动上报上行定位请求网络错误（数字格式）
     *       +CMOLRE: 4
     *       主动上报上行定位请求网络连接错误（字符串格式）
     *       +CMOLRE: Internet connection failure
     */
    { AT_STRING_CMOLRE, (VOS_UINT8 *)"+CMOLRE: " },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 上报定位报告
     * [说明]: NMEA字符串格式的定位报告上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CMOLRN: <NMEA-string><CR><LF>
     * [参数]:
     *     <NMEA-string>: UTF-8字符串类型，此参数为协议标准IEC 61162中定义的NMEA字符串，编码类型不受常规字符转换命令+CSCS影响。
     * [示例]:
     *       主动上报定位信息
     *       +CMOLRN: "$GPRMC,235947.000,V,0000.0000,N,00000.0000,E,,,041299,,*1D"
     */
    { AT_STRING_CMOLRN, (VOS_UINT8 *)"+CMOLRN: " },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 上报定位报告
     * [说明]: GAD形状类型的定位报告上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CMOLRG: <location_parameters><CR><LF>
     * [参数]:
     *     <location_parameters>: UTF-8字符串类型，此参数为XML字符串格式表示的GAD形状类型的定位信息，XML格式参见3GPP 27007 Table 8.50-2。XML可以分多段上报。
     *             注：编码类型不受常规字符转换命令+CSCS影响。
     * [示例]:
     *       主动上报定位信息
     *       +CMOLRG:
     *       <?xml version="1.0" ?>
     *       <location_parameters>
     *        <shape_data>
     *          <ellipsoid_point>
     *            <coordinate>
     *              <latitude>
     *                <north>0</north>
     *                <degrees>0</degrees>
     *             </latitude>
     *           <longitude>0</longitude>
     *          </coordinate>
     *         </ellipsoid_point>
     *        </shape_data>
     *       </location_parameters>
     */
    { AT_STRING_CMOLRG, (VOS_UINT8 *)"+CMOLRG: " },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 上报下行定位请求指示
     * [说明]: 下行定位请求指示的上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CMTLR: <handle-id>,<notification-type>,<location-type>, [<client-external-id>], [<client-name>][,<plane>]<CR><LF>
     * [参数]:
     *     <handle-id>: 整型值，每个MT-LR的ID用于区分多个请求中的指定请求，取值范围为0~255。
     *     <notification-type>: 整型值，有关用户隐私的指示类型：
     *             0：允许第三方定位用户，网络可以选择用户友好的提示；
     *             1：当用户忽略指示时，允许定位用户；
     *             2：当用户忽略指示时，禁止定位用户。
     *     <location-type>: 整型值，请求的位置类型：
     *             0：当前位置；
     *             1：当前位置或最后可知位置；
     *             2：初始位置。
     *     <client-external-id>: 字符串类型，指示外部客户端定位信息发送的位置（可选）。
     *     <client-name>: 字符串类型，请求用户位置的外部客户端标识。
     *     <plane>: 整型值，MT-LR上报的方式：
     *             0：控制面；
     *             1：SUPL。
     * [示例]:
     *       主动上报下行定位请求指示
     *       +CMTLR: 1,0,0
     */
    { AT_STRING_CMTLR, (VOS_UINT8 *)"+CMTLR: " },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: IMS通知AP需要重连DM服务器
     * [说明]: 该命令用于IMSA指示AP需要重连DM服务器。
     * [语法]:
     *     [命令]:
     *     [结果]: <CR><LF>^DMCN<CR><LF>
     * [参数]:
     *     <无>: 此主动上报不包含参数。
     * [示例]:
     *       IMS注册收到403响应，通知AP需要重连DM服务器特性
     *       ^DMCN
     */
    { AT_STRING_DMCN, (VOS_UINT8 *)"^DMCN" },
#endif

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: IMS通知AP当前拒绝信息
     * [说明]: 该命令用于IMS指示AP当前拒绝情况。
     * [语法]:
     *     [命令]: -
     *     [结果]: ^IMSREGFAIL: <FAIL_CAUSE>
     * [参数]:
     *     <FAIL_CAUSE>: 1：表示IMS永久被拒；
     * [示例]:
     *       IMS注册收到两个403响应后，通知AP当前IMS永久被拒：
     *       ^IMSREGFAIL: 1
     */
    { AT_STRING_IMS_REG_FAIL, (VOS_UINT8 *)"^IMSREGFAIL: " },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: IMSI号刷新的主动上报
     * [说明]: 该命令用于IMSI号刷新时的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSIREFRESH <CR><LF>
     * [参数]:
     * [示例]:
     *       IMSI号刷新
     *       ^IMSIREFRESH
     */
    { AT_STRING_IMSI_REFRESH, (VOS_UINT8 *)"^IMSIREFRESH" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: AFC时钟失锁状态指示
     * [说明]: 该命令用于GPS芯片在使用Modem提供的AFC时钟进行时钟校准时，查询Modem的AFC时钟状态为锁定态，之后Modem将AFC时钟由于多模物理层睡眠等原因导致的AFC时钟失锁的事件通知到GPS芯片，以便GPS芯片即时终止本次校准流程。
     *         注意：此命令只有在AT^AFCCLKINFO?查询结果为锁定状态后，出现<cause>引起的AFC时钟失锁时主动广播上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^AFCCLKUNLOCK: <cause><CR><LF>
     * [参数]:
     *     <cause>: 整数类型，AFC时钟失锁的原因：
     *             0：多模物理层睡眠导致的失锁。
     * [示例]:
     *       GPS芯片查询AFC时钟处于锁定状态后，多模物理层进入睡眠导致时钟失锁时：
     *       ^AFCCLKUNLOCK: 0
     */
    { AT_STRING_AFCCLKUNLOCK, (VOS_UINT8 *)"^AFCCLKUNLOCK: " },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 是否开启播放本地提示音主动上报
     * [说明]: 该命令用于IMS域中是否开启播放本地提示音主动上报给AP。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSHOLDTONE: <hold_tone><CR><LF>
     * [参数]:
     *     <hold_tone>: 是否播放本地提示音类型，整型值
     *             0：不播放；
     *             1：播放。
     * [示例]:
     *       收到是否开启播放本地提示音主动上报
     *       ^IMSHOLDTONE: 1
     */
    { AT_STRING_IMS_HOLD_TONE, (VOS_UINT8 *)"^IMSHOLDTONE" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 主动上报PDP激活是否被禁止
     * [说明]: ^LIMITPDPACT用于主动上报PDP激活是否被禁止
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LIMITPDPACT: <FLG><CAUSE><CR><LF>
     * [参数]:
     *     <FLG>: 1:禁止PDP激活
     *             0:允许PDP激活
     *     <CAUSE>: PDP激活被拒原因值，仅在FLG为1时有效；
     *             FLG为0时，CAUSE值固定为0
     * [示例]:
     *       PDP激活被拒#33达到3次，APS启2h定时器，禁止PDP激活时：
     *       ^LIMITPDPACT: 1,289
     */
    { AT_STRING_LIMITPDPACT, (VOS_UINT8 *)"^LIMITPDPACT" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS失败原因值主动上报
     * [说明]: IMS注册失败时，主动上报当前IMS注册失败的原因；
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSREGERR: <err_domain>,<err_stage>,<fail_cause>,<fail_str><CR><LF>
     * [参数]:
     *     <err_domain>: 整数型，IMS注册失败域，取值0～4；
     *             0：IMS注册失败域为LTE；
     *             1：IMS注册失败域为WIFI；
     *             2：IMS注册失败域为Utran；
     *             3：IMS注册失败域为Gsm；
     *             4：IMS注册失败域为NR；
     *             示例：
     *             当UE在LTE上IMS注册成功，IMS域从LTE切换到WIFI的过程中，此时上报IMSREGERR的err_domain是指正在尝试IMS注册的那个域，即WIFI域；
     *     <err_stage>: 整数型，IMS注册失败阶段，取值0～1；
     *             0：PDN建立阶段；
     *             1：IMS注册阶段；
     *     <fail_cause>: 整数型，IMS注册失败阶段，取值0~699；
     *             注意：
     *             可能存在err_stage不同，fail_cause相同的场景；但此时是完全不同的两种错误，一个是APN建立阶段的错误，一个是IMS注册阶段的错误，只不过原因值恰好相同
     *     <fail_str>: 字符串，网络下发的失败原因字符串；
     *             其中IMS协议栈内部原因值时，字符串为空；
     *             当网络携带的失败原因值字符串为空时，此参数也为空；
     *             此外失败原因值字符串最大长度为255；
     * [示例]:
     *       LTE上PDN建立过程失败，失败原因值为10
     *       ^IMSREGERR: 0,0,10,"network fail"
     *       WIFI上注册过程失败，失败原因值为111，对应的字符串为空
     *       ^IMSREGERR: 1,0,111,""
     */
    { AT_STRING_IMS_REG_ERR, (VOS_UINT8 *)"^IMSREGERR:" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 云通信受限小区信息主动上报
     * [说明]: CSS无该国家对应的受限小区信息时主动上报^REPORTBLOCKCELLMCC，AP收到^REPORTBLOCKCELLMCC的上报，需要下发^BLOCKCELLLIST设置对应的受限小区信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^REPORTBLOCKCELLMCC: <VER> ,<MCC>[,<MCC>….]<CR><LF>
     * [参数]:
     *     <VER>: 云通讯匹配版本号,包含在“”中
     *     <MCC>: MCC信息，可能一个，也可能多个（目前最多17个），多个时以”逗号”隔开
     * [示例]:
     *       主动上报云通信受限小区MCC信息
     *       ^REPORTBLOCKCELLMCC: "00.00.001",460,230
     */
    { AT_STRING_BLOCK_CELL_MCC, (VOS_UINT8 *)"^REPORTBLOCKCELLMCC: " },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 云通信相关信息主动上报
     * [说明]: AT转发CSS上报的云通信相关信息给AP。
     * [语法]:
     *     [命令]:
     *     [结果]: <CR><LF>^REPORTCLOUDDATA: <LENGTH>,<BSINFO><CR><LF>
     * [参数]:
     *     <LENGTH>: ASCII码流表示的字节长度，长度不超过500，其中（两个ASCII码流表示一个字节）
     *     <BSINFO>: ASCII码流
     * [示例]:
     *       上报云通信相关信息
     *       ^REPORTCLOUDDATA: 21,000000000200000002000000FF0406000001000F00
     */
    { AT_STRING_CLOUD_DATA, (VOS_UINT8 *)"^REPORTCLOUDDATA: " },

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报APN backoff定时器信息
     * [说明]: AT转发THROT主动上报的APN backoff定时器相关信息给AP。
     *         是否上报受+CABTSR命令控制，+CABTSR为1时才能上报。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、modem2暂不支持，当从modem2发送此命令时，直接返回error；
     *         2、此命令受FEATURE_APN_BASED_BO_TIMER_PARA_RPT宏控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CABTSRI: <apn>,<event_type>[,<residual_backoff_time>,<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>,<NSLPI>[,<procedure>]]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <apn>: APN信息。字符串类型，最大长度为99。
     *     <event_type>: 指示Back-off timer事件信息。整数类型，取值如下：
     *             0：启动back-off timer定时器。
     *             1：Back-off timer停止。
     *             2：Back-off timer定时器超时。
     *             3：Back-off timer处于deactivated。
     *     <residual_backoff_time>: 该参数表示backoff定时器剩余时长，单位是秒。整数类型。
     *             注：
     *             当定时器处于deactivated 状态时，此值省略，输出逗号。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <NSLPI>: 指示信令的优先级。整数类型，取值如下：
     *             0：配置为信令低优先级;
     *             1：没有配置为信令低优先级。
     *     <procedure>: 指示受到backoff定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1: backoff定时器适用STAND ALONE PDN流程；
     *             2: backoff定时器适用BEARER ALLOC流程；
     *             3: backoff定时器适用BEARER MODIFY流程；
     *             4: backoff定时器适用PDP_ACTIVE流程；
     *             5: backoff定时器适用SECOND_PDP_ACTIVE流程；
     *             6: backoff定时器适用PDP_MODIFY流程；
     *             7: backoff定时器适用PDU_ESTABLISH流程；
     *             8: backoff定时器适用PDU_MODIFY流程；
     *             9: backoff定时器适用ATTACH和STAND ALONE的PDN流程。
     * [示例]:
     *       定时器处于开启状态，上报APN backoff定时器相关信息
     *       +CABTSRI: "abc",0,240,0,1,1,1
     *       定时器处于停止状态时，上报的信息
     *       +CABTSRI: "abcD",1
     */
    { AT_STRING_CABTSRI, (VOS_UINT8 *)"+CABTSRI: " },
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报S-NSSAI backoff定时器信息
     * [说明]: AT转发THROT主动上报的S-NSSAI backoff定时器相关信息给AP。
     *         是否主动上报受+CSBTSR命令控制。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT宏控制
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CSBTSRI: <S-NSSAI>,<event_type>[,<S-NSSAI_backoff_time>,<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>[,<procedure>]]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <S-NSSAI>: 字符串类型，5GS的分片信息。
     *             切片格式如下所示：
     *             1、sst
     *             2、sst.sd
     *             3、sst;mappedSst
     *             4、sst.sd;mappedSst
     *             5、sst.sd;mappedSst.mappedSd
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F,a-f。
     *     <event_type>: 指示Back-off timer事件信息。整数类型，取值如下：
     *             0：启动back-off timer定时器。
     *             1：Back-off timer停止。
     *             2：Back-off timer定时器超时。
     *             3：Back-off timer处于deactivated。
     *     <S-NSSAI_backoff_time>: 该参数表示S-NSSAI backoff定时器剩余时长。单位是秒，整数类型。
     *             注意：当定时器处于deactivated 状态时，此值省略，输出逗号。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <procedure>: 指示受到T3585定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1:定时器适用PDU_ESTABLISH流程；
     *             2:定时器适用PDP_MODIFY流程。
     * [示例]:
     *       定时器处于开启状态，上报S-NSSAI backoff定时器相关信息
     *       +CSBTSRI: ,0,180,0,1,0
     *       定时器处于停止状态时，上报的信息
     *       +CSBTSRI: "01.010101",1
     */
    { AT_STRING_CSBTSRI, (VOS_UINT8 *)"+CSBTSRI: " },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报基于S-NSSAI和DNN的backoff定时器信息
     * [说明]: AT转发THROT主动上报的基于S-NSSAI和DNN的backoff定时器相关信息给AP。
     *         是否主动上报受+CSDBTSR命令控制。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT宏控制
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CSDBTSRI: <S-NSSAI>,<DNN>,<event_type>[,<S-NSSAI_DNN_backoff_time>,<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>[,<procedure>]]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <S-NSSAI>: 字符串类型，5GS的分片信息。
     *             切片格式如下所示：
     *             1、sst
     *             2、sst.sd
     *             3、sst;mappedSst
     *             4、sst.sd;mappedSst
     *             5、sst.sd;mappedSst.mappedSd
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F,a-f。
     *     <DNN>: DNN信息。字符串类型，最大长度为99。
     *     <event_type>: 指示Back-off timer事件信息。整数类型，取值如下：
     *             0：启动back-off timer定时器。
     *             1：Back-off timer停止。
     *             2：Back-off timer定时器超时。
     *             3：Back-off timer处于deactivated。
     *     <S-NSSAI_DNN_backoff_time>: 该参数表示基于S-NSSAI和DNN的backoff定时器剩余时长。单位是秒，整数类型。
     *             注意：当定时器处于deactivated 状态时，此值省略，输出逗号。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <procedure>: 指示受到T3585定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1:定时器适用PDU_ESTABLISH流程；
     *             2:定时器适用PDP_MODIFY流程。
     * [示例]:
     *       定时器处于开启状态，上报基于S-NSSAI和DNN的backoff定时器相关信息
     *       +CSDBTSRI: ,"huawei.com",0,15,0,1,0
     *       定时器处于停止状态时，上报的信息
     *       +CSDBTSRI: ,"huawei.com",1
     */
    { AT_STRING_CSDBTSRI, (VOS_UINT8 *)"+CSDBTSRI: " },
#endif
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: ECC状态主动上报
     * [说明]: 该命令用于主动上报ECC状态。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ECCSTATUS: <ValidFlg>,<RxAntNum>,<TmMode>,<CorrQR1Data0011>,<CorrQR1Data01IQ>,<CorrQR2Data0011>,<CorrQR2Data01IQ>,<CorrQR3Data0011>,<CorrQR3Data01IQ>,<Rssi0>,<Rssi1>,<Rssi2>,<Rssi3>,<Rssi0>,<Rssi1>,<Rssi2>,<Rssi3><CR><LF>
     * [参数]:
     *     <ValidFlg>: 整型值，无符号字符型：
     *             1：主卡主CC TM1时数据有效；
     *             0：无效数据。
     *     <RxAntNum>: 整型值，UE接收天线数。
     *     <TmMode>: 整型值，传输模式
     *     <CorrQR1Data0011>: 整型值，CorrQR1Data0011寄存器值。
     *     <CorrQR1Data01IQ>: 整型值，CorrQR1Data01IQ寄存器值。
     *     <CorrQR2Data0011>: 整型值，CorrQR2Data0011寄存器值。仅天线数为4时有效。
     *     <CorrQR2Data01IQ>: 整型值，CorrQR2Data01IQ寄存器值。仅天线数为4时有效。
     *     <CorrQR3Data0011>: 整型值，CorrQR3Data0011寄存器值。仅天线数为4时有效。
     *     <CorrQR3Data01IQ>: 整型值，CorrQR3Data01IQ寄存器值。仅天线数为4时有效。
     *     <RsrpR0>: 整型值, 天线0 rsrp
     *     <RsrpR1>: 整型值, 天线1 rsrp
     *     <RsrpR2>: 整型值, 天线2 rsrp
     *     <RsrpR3>: 整型值, 天线3 rsrp
     *     <Rssi0>: 整型值, 天线0 rssi
     *     <Rssi1>: 整型值, 天线1 rssi
     *     <Rssi2>: 整型值, 天线2 rssi
     *     <Rssi3>: 整型值, 天线3 rssi
     * [示例]:
     *       主动上报ECC状态
     *       ^ECCSTATUS:
     *       1,2,0,483661589,723914177,0,0,0,0,-85,-86,-141,-141,-66,-68,-130,-130
     */
    { AT_STRING_ECC_STATUS, (VOS_UINT8 *)"^ECCSTATUS: " },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 将网侧下发的EPDU序列上报给AP
     * [说明]: 该命令用于将Modem收到网络下发的ePDU序列广播给上层应用（GPS芯片等）。
     *         受AT命令上报长度限制，采用分片的方式将ePDU序列码流拆分上报给上层应用。
     *         该命令无查询命令、测试命令。
     * [语法]:
     *     [命令]: -
     *     [结果]: ^EPDUR: <transaction_id>,<msg_type>,<common_info_valid_flg>,<end_flag>, <id>,[<name>], <total>,<index>,[<data>]
     * [参数]:
     *     <transaction_id>: ePDU码流对应的Transaction Id，整数类型，取值范围为0~65535。
     *     <msg_type>: ePDU码流的消息类型，整数类型，取值范围0~7。
     *             0：请求能力；
     *             1：提供能力；
     *             2：请求辅助数据；
     *             3：提供辅助数据；
     *             4：请求位置信息；
     *             5：提供位置信息；
     *             6：取消；
     *             7：错误。
     *     <common_info_valid_flg>: LPP CommonIEs信息是否有效标志位，整数类型，取值范围0~1。
     *             0：无效；
     *             1：有效。
     *     <end_flag>: ENDFLAG标志位，整数类型，取值范围0~1。
     *             0：ePDU流程结束；
     *             1：还有后续ePDU流程。
     *     <id>: ePDU Identifier，ePDU标识，整数类型，取值范围1~256。
     *     <name>: ePDU Name，ePDU名称，字符串类型，最多32个字符。
     *     <total>: ePDU Body码流总分片个数，整数类型，取值范围1~5。
     *     <index>: ePDU Body码流分片索引，整数类型，取值范围1~5。
     *     <data>: ePDU Body分片码流，通过Half-Byte（16进制）编码成的字符串，最大长度为1000个字符。
     * [示例]:
     *       主动上报：
     *       ^EPDUR: 1,2,0,0,121,"LPPE",1,1,0123456789ABCDEF0123456789ABCDEF
     */
    { AT_STRING_EPDUR, (VOS_UINT8 *)"^EPDUR: " },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE小区CA状态信息主动上报
     * [说明]: UE在驻留LTE网络时，SCell激活状态变化、CA添加/释放等场景通过此命令主动上报LTE小区的CA状态信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LCACELLEX: <total_num >,<cell_index>,<ul_cfg>, <dl_cfg>, <act_flg>,<laa_flg>,<band>,<band_width>,<earfcn>,<CR><LF> [<CR><LF>^LCACELLEX: <total_num >,<cell_index>,<ul_cfg>,<dl_cfg>, <act_flg>,<laa_flg>, <band>,<band_width>,<earfcn>,<CR><LF>[…]]
     * [参数]:
     *     <total_num>: 整型值，本次上报LTE小区个数。
     *     <cell_index>: 整型值，LTE小区索引，0表示PCell，其余为SCell。
     *     <ul_cfg>: 整型值，该小区上行CA是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <dl_cfg>: 整型值，该小区下行CA是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <act_flg>: 整型值，该小区 CA是否被激活：
     *             0：未激活；
     *             1：已激活。
     *     <laa_flg>: 整型值，该小区是否为Laa小区：
     *             0：不是Laa小区；
     *             1：是Laa小区。
     *     <band>: 整型值，该小区的Band频段，如7代表Band VII。
     *     <band_width>: 整型值，该小区占用带宽：
     *             0：带宽为1.4MHz；
     *             1：带宽为3MHz；
     *             2：带宽为5MHz；
     *             3：带宽为10MHz；
     *             4：带宽为15MHz；
     *             5：带宽为20MHz。
     *     <earfcn>: 整型值，该小区频点。
     * [示例]:
     *       LTE网络下SCell激活状态变化、CA添加/释放等场景主动上报小区CA状态信息
     *       ^LCACELLEX: 2,0,1,1,1,0,7,2,21120
     *       ^LCACELLEX: 2,1,1,1,1,1,34,1,36230
     */
    { AT_STRING_LCACELLEX, (VOS_UINT8 *)"^LCACELLEX: " },
    { AT_STRING_VT_FLOW_RPT, (VOS_UINT8 *)"^VTFLOWRPT: " },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 飞行模式VoWiFi下，普通呼叫网络回380主动上报
     * [说明]: 该命令在MODEM飞行模式时，VoWiFi注册成功，通过VoWiFi发起普通呼叫，网络回380场景时，主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CALLALTSRV<CR><LF>
     * [参数]:
     * [示例]:
     *       主动上报：
     *       ^CALLALTSRV
     */
    { AT_STRING_CALL_ALT_SRV, (VOS_UINT8 *)"^CALLALTSRV" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: GPS Fine Time主动上报
     * [说明]: GPS芯片下发^FINETIMEREQ命令后触发GPS Fine Time上报流程，UE通过此命令上报对应SYNC时钟脉冲的GPS Fine Time信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^FINETIMEINFO: <rat>,<ta>,<sinr>,<state>,<fine_time>, <fine_time_offset>[, <leap_sec>]<CR><LF>
     * [参数]:
     *     <rat>: 整数类型，提供Fine Time的接入技术：
     *             0：GSM（暂不支持）；
     *             1：WCDMA（暂不支持）；
     *             2：LTE；
     *             3：TDS-CDMA（暂不支持）；
     *             4：CDMA（暂不支持）；
     *             5：HRPD（暂不支持）；
     *             6：NR。
     *     <ta>: 整数类型，网络侧配置的时间提前量。
     *     <sinr>: 整数类型，当前信噪比。
     *     <state>: 整数类型，对应接入技术下物理层状态：
     *             0：IDLE态；
     *             1：CONNECTED态；
     *             2：INACTIVE态（仅NR模式下）；
     *             3：其他状态。
     *     <fine_time>: 16进制长整数类型：
     *             LTE模式时，网络UTC时间，单位10ms。
     *     <fine_time_offset>: 16进制长整数类型：
     *             LTE模式时，网络UTC时间偏移，单位10ms。
     *     <leap_sec>: 整数类型，UTC时间与GPS时间的跳秒。
     * [示例]:
     *       LTE接入层连接态上报GPS Fine Time信息
     *       ^FINETIMEINFO: 2,1,1,1,41478472,683957289,16
     */
    { AT_STRING_FINETIMEINFO, (VOS_UINT8 *)"^FINETIMEINFO: " },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: LTE模式SFN上报
     * [说明]: GPS芯片下发^FINETIMEREQ命令后触发SFN帧脉冲流程，UE通过此命令上报当前LTE模式的SFN帧号，并同时触发一个SYNC时钟脉冲。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SFN: <num><CR><LF>
     * [参数]:
     *     <num>: 整数类型，LTE模式的SFN帧号。
     * [示例]:
     *       LTE模式下上报SFN帧号
     *       ^SFN: 12850
     */
    { AT_STRING_SFN, (VOS_UINT8 *)"^SFN: " },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: Fine Time异常上报
     * [说明]: 命令^FINETIMEREQ触发GPS Fine Time/SFN脉冲异常时上报此命令通知GPS芯片。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^FINETIMEFAIL: <cause><CR><LF>
     * [参数]:
     *     <cause>: 整型值，触发GPS Fine Time/SFN脉冲异常场景：
     *             1：强制触发脉冲或GPS Fine Time脉冲失败；
     *             2：触发SFN帧脉冲失败。
     * [示例]:
     *       触发GPS Fine Time脉冲异常：
     *       ^FINETIMEFAIL: 1
     */
    { AT_STRING_FINETIMEFAIL, (VOS_UINT8 *)"^FINETIMEFAIL: " },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 获取AP侧LTE的UE能力数据参数
     * [说明]: 本命令用于LRRC主动上报UE当前的参数信息给AP侧，获取AP侧定制LTE的UE能力数据信息，和^LRRCUECAPPARAMSET命令配合使用。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LRRCUECAPINFONTF: <Len>,<Param><CR><LF>
     * [参数]:
     *     <Len>: 整型，<Param>的长度
     *     <Param>: MODEM上报的参数信息码流，长度不超过4800字节。
     * [示例]:
     *       主动上报UE的参数信息：
     *       ^LRRCUECAPINFONTF: 8,"12345678"
     */
    { AT_STRING_LRRCUECAPINFONTF, (VOS_UINT8 *)"^LRRCUECAPINFONTF: " },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 物理层ACK通用命令
     * [说明]: 该命令用于支持物理层ACK主动上报通用命令
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^PHYCOMACK: <ack type>,<value1>,<value2>,<value3><CR><LF>
     * [参数]:
     *     <ack type>: 整型十进制 范围（0-4294967295）
     *             ACK类型
     *     <value1>: 整型十进制 范围（0-4294967295）
     *     <value2>: 整型十进制 范围（0-4294967295）
     *     <value3>: 整型十进制 范围（0-4294967295）
     * [示例]:
     *       物理层上报ACK，其中ack type，value1，value2，value3值均为1。
     *       ^PHYCOMACK: 1,1,1,1
     */
    { AT_STRING_PHYCOMACK, (VOS_UINT8 *)"^PHYCOMACK: " },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 底层组件通用的BOOSTER主动上报命令
     * [说明]: 底层组件通用的BOOSTER主动上报命令：^BOOSTERNTF。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^BOOSTERNTF: <MODULD_ID>,<LEN>,<PARAM><CR><LF>
     * [参数]:
     *     <MODULEID>: 消息接收组件，整形：
     *             0：LRRC；
     *             1：LPHY；
     *             2：MMC；
     *             3：BASTET。
     *     <LEN>: <PARAM>的长度
     *     <PARAM>: 字符串，TLV格式的参数，T和L各2个字节，具体定义由各组件维护，设置失败的错误原因值也由各个组件各自维护
     * [示例]:
     *       LRRC的BOOSTER主动上报命令
     *       ^BOOSTERNTF: 0,4,"01021236"
     */
    { AT_STRING_BOOSTERNTF, (VOS_UINT8 *)"^BOOSTERNTF: " },

    { AT_STRING_MTREATTACH, (VOS_UINT8 *)"^MTREATTACH" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IP多媒体公共标识类型主动上报
     * [说明]: 该命令是IP多媒体公共标识类型主动上报给应用。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMPU: <type><CR><LF>
     * [参数]:
     *     <type>: 整型值，取值范围为(0,1)：
     *             0：MSISDN类型；
     *             1：IMSI类型。
     * [示例]:
     *       IP多媒体公共标识类型主动上报
     *       ^IMPU: 1
     */
    { AT_STRING_IMPU, (VOS_UINT8 *)"^IMPU: " },

#if (FEATURE_DSDS == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: DSDS状态主动上报
     * [说明]: 该命令用于Modem当前的DSDS状态的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: ^DSDSSTATE: <status>
     * [参数]:
     *     <status>: 整型值，Modem的DSDS状态：
     *             0：DSDS2.0；
     *             1：DSDS3.0。
     * [示例]:
     *       DSDS上报DSDS3.0的状态
     *       ^DSDSSTATE: 1
     */
    { AT_STRING_DSDSSTATE, (VOS_UINT8 *)"^DSDSSTATE: " },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 零度温保主动上报
     * [说明]: 该命令用于上报芯片温度变化低于0度或高于0度时温保事件。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ TEMPPROTECT:<n><CR><LF>
     * [参数]:
     *     <n>: 整型值：
     *             0：温保结束
     *             1：需要启动温保
     * [示例]:
     *       上报温保事件
     *       ^TEMPPROTECT: 0
     */
    { AT_STRING_TEMPPROTECT, (VOS_UINT8 *)"^TEMPPROTECT: " },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: RTT事件上报
     * [说明]: 该命令用于RTT事件上报，上报场景如下：
     *         Voice被叫，RTT接听，Text流建立成功；
     *         Voice通话中，远端发起RTT升级请求；
     *         Voice通话中，远端发起RTT升级，Text流建立成功；
     *         Voice通话中，本端发起RTT升级，Text流建立成功；
     *         RTT通话中，远端RTT降级，Text流Close成功；
     *         RTT通话中，本端RTT降级，Text流Close成功；
     *         RTT参数变更，当前仅CPS值。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^RTTEVT: <call_id>,<event>,<reason><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫标识，取值范围1~7。
     *     <event>: 整型值，RTT事件类型：
     *             0：Add Text媒体流；
     *             1：Close Text媒体流；
     *             2：远端请求Add Text媒体流；
     *             3：RTT参数变更。
     *     <reason>: 整型值，RTT事件原因：
     *             0：Local User Trigger；
     *             1：Remote User Trigger。
     * [示例]:
     *       Voice通话中，本端发起RTT升级，Text流建立成功
     *       ^RTTEVT: 1,1,0
     */
    { AT_STRINT_RTTEVENT, (VOS_UINT8 *)"^RTTEVT" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: RTT异常错误上报
     * [说明]: 该命令用于RTT错误事件上报，上报场景如下：
     *         RTT主叫，Text流建立失败；
     *         Voice被叫，RTT接听，Text流建立失败；
     *         Voice通话中，本端发起RTT升级，Text流建立失败；
     *         RTT通话中，本端发起RTT降级，Text流Close失败。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^RTTERR: <call_id>,<operation>,<cause>, <reason_text><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫标识，取值范围1~7。
     *     <operation>: 整型值，异常的事件对应的操作类型：
     *             0：Add Text媒体流；
     *             1：Close Text媒体流。
     *     <cause>: 整型值，异常响应码为SipStatusCode或者Q850，缺省原因为-1：
     *             1~127：Q850，对应cs的原因；
     *             400~699：Sip响应码。
     *     <reason_text>: 字符串类型，异常响应原因文本信息，缺省原因为“unknown”。
     * [示例]:
     *       Voice通话中，本端发起RTT升级，Text流建立失败
     *       ^RTTERR: 1,0,-1,”unknown”
     */
    { AT_STRINT_RTTERROR, (VOS_UINT8 *)"^RTTERR" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: RSSI预警上报命令
     * [说明]: 当RSSI变化超过阈值时，MT主动上报此指示给TE。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^EMRSSIRPT: <rat>,<rssi><CR><LF>
     * [参数]:
     *     <rat>: 整型值，网络制式。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     *     <rssi>: 整型值，接收信号强度指示。
     *             0：rssi <-120 dBm
     *             1：-120 dBm ?0?5 rssi <-119 dBm
     *             2：-119 dBm ?0?5 rssi <-118 dBm
     *             :   :::
     *             94：-27 dBm ?0?5 rssi <-26 dBm
     *             95：-26 dBm ?0?5 rssi <-25 dBm
     *             96：-25 dBm ?0?5 rssi
     *             255：未知或不可测
     * [示例]:
     *       LTE服务小区RSSI变化超过阈值
     *       ^EMRSSIRPT: 2,30
     */
    { AT_STRING_EMRSSIRPT, (VOS_UINT8 *)"^EMRSSIRPT: " },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 电梯模式状态上报
     * [说明]: 该命令用于电梯模式状态发生变化时，将电梯模式状态和当前网络服务状态上报给AP，用于后续指示AP下发指定历史搜命令，快速回LTE。
     *         注：如果当前配置不支持LTE（SYSCFG配置）、无卡、卡无效，则不会上报该消息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF> ^ELEVATOR: <sensor state>,<service state><sensor scene><CR><LF>
     * [参数]:
     *     <sensor state>: 整型值，电梯或车库状态：
     *             0：出电梯或出车库状态；
     *             1：进电梯或进车库状态。
     *     <Service state>: 整型值，当前网络服务状态：
     *             0：无服务
     *             1：有服务
     *     <Sensor scene>: 整型值，Sensor指示的状态变化场景：
     *             0：电梯场景；
     *             1：车库场景。
     * [示例]:
     *       SENSOR检测当前进入电梯，且当前正常服务状态：
     *       ^ELEVATOR: 1,1,0
     *       SENSOR检测当前出车库，且当前无服务状态：
     *       ^ELEVATOR: 0,0,1
     */
    { AT_STRING_ELEVATOR, (VOS_UINT8 *)"^ELEVATOR" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 服务小区上行频率上报
     * [说明]: 该命令用于主动上报当LTE/WCDMA/HRPD频率变化时的服务小区上行频率信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ULFREQRPT: <rat>,<ulfreq>,<ulbw><CR><LF>
     * [参数]:
     *     <rat>: 整型值，接入技术：
     *             0：无服务
     *             1：WCDMA
     *             2：HRPD
     *             3：LTE
     *     <ulfreq>: 整型值，服务小区上行频率。单位100kHz。
     *     <ulbw>: 整型值，服务小区上行带宽。单位1kHz。
     * [示例]:
     *       主动上报服务小区上行频率
     *       ^ULFREQRPT: 1,19200,5000
     */
    { AT_STRING_ULFREQRPT, (VOS_UINT8 *)"^ULFREQRPT" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 伪基站拦截主动上报
     * [说明]: 该命令用于接入层识别到伪基站或进行拦截时的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: ^PSEUDBTS: <RAT>
     * [参数]:
     *     <RAT>: 1:GSM
     *             2:WCDMA（暂不支持）
     *             3:LTE
     * [示例]:
     *       2G伪基站拦截主动上报
     *       ^PSEUDBTS: 1
     */
    { AT_STRING_PSEUDBTS, (VOS_UINT8 *)"^PSEUDBTS" },
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 5GC注册状态主动上报
     * [说明]: 若终端支持NR的能力：
     *         在n=2时，如果终端在5G下注册，则该命令上报完整的小区和切片信息
     *         +C5GREG: <stat>,<tac>,<ci>,<AcT>,<Allowed_NSSAI_length>[,<Allowed_NSSAI>]
     *         否则，此命令主动上报PS域的注册结果+C5GREG: <stat>。
     *         该AT命令具体描述如下：
     *         该命令仅在单板支持NR时才能使用（即当前单板支持NR能力）。
     * [语法]:
     *     [命令]: -
     *     [结果]: 网络PS域注册状态发生改变时
     *             <CR><LF>+C5GREG: <stat><CR><LF>
     *     [命令]: -
     *     [结果]: 小区信息发生改变或者Allowed NSSAI发生变化时：
     *             <CR><LF>+C5GREG: <stat>[,[<tac>],[<ci>],[<AcT>],[<Allowed_NSSAI_length>],[<Allowed_NSSAI>]]<CR><LF>
     * [参数]:
     *     <n>: 整型值，默认值为0。
     *             0：禁止+C5GREG的主动上报；
     *             1：使能+C5GREG: <stat>的主动上报；
     *             2：使能+C5GREG: <stat>[,[<tac>],[<ci>],[<AcT>],[<Allowed_NSSAI_length>],[<Allowed_NSSAI>] ]的主动上报。
     *     <stat>: 0：没有注册，MT现在并没有在搜寻要注册的新的运营商；
     *             1：注册了本地网络；
     *             2：没有注册，但MT正在搜寻要注册的新的运营商；
     *             3：注册被拒绝；
     *             4：未知原因；
     *             5：注册了漫游网络。
     *             8：registered for emergency services only；
     *     <tac>: 位置码信息，六个字符，16进制表示。（例：“0000C3”＝10进制的195）
     *     <ci>: 小区信息，8个字节，16个字符，16进制表示。使用Half-Byte码（半字节）编码，例如000000010000001A，表示高4字节是0x00000001，低4字节是0x0000001A。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             10：EUTRAN-5GC
     *             11：NR-5GC
     *     <Allowed_NSSAI_length>: 整型值，表示<Allowed_NSSAI>信息中切片字节个数；
     *             无<Allowed_NSSAI>信息时值为0。
     *     <Allowed_NSSAI>: 字符串类型。每个S-NSSAI使用“:”区分。单个S-NSSAI的格式
     *             "sst" if only slice/service type (SST) is present
     *             "sst;mapped_sst" if SST and mapped configured SST are present
     *             "sst.sd" if SST and slice differentiator (SD) are present
     *             "sst.sd;mapped_sst" if SST, SD and mapped configured SST are present
     *             "sst.sd;mapped_sst.mapped_sd" if SST, SD, mapped configured SST and mapped configured SD are present
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F，a-f。
     *             无Allowed_NSSAI列表信息时值为空。
     * [示例]:
     *       网络注册的状态发生改变时
     *       +C5GREG: 1
     *       注册在5G上小区信息发生改变时
     *       +C5GREG: 1,"0000C3","000000010000001A",11,9,"01:01.123456;2F.654321"
     */
    { AT_STRING_C5GREG, (TAF_UINT8 *)"+C5GREG: " },
    { AT_STRING_CSERSSI, (VOS_UINT8 *)"^CSERSSI:" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 获取AP侧NR的UE能力数据参数
     * [说明]: 本命令用于NRRC主动上报UE当前的参数信息给AP侧，获取AP侧定制NR的UE能力数据信息，和^NRRCUECAPPARAMSET命令配合使用。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^NRRCUECAPINFONTF: <Len>,<Param><CR><LF>
     * [参数]:
     *     <Len>: 整型，<PARAM>的长度
     *     <Param>: MODEM上报的参数信息码流，长度不超过4800字节。
     * [示例]:
     *       主动上报UE的参数信息：
     *       ^NRRCUECAPINFONTF: 8,"12345678"
     */
    { AT_STRING_NRRCUECAPINFONTF, (VOS_UINT8 *)"^NRRCUECAPINFONTF: " },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR CA小区信息主动上报
     * [说明]: UE在驻留NR或LTE NSA网络时，CA信息变化主动上报NR CA信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^NRCACELLRPT: <total_num>[,<cell_index>,<dl_cfg>, <nul_cfg>,<sul_cfg>[,<cell_index>,<dl_cfg>,<nul_cfg>,<sul_cfg> <[…]]] <CR><LF>
     * [参数]:
     *     <total_num>: 整型值，本次上报的NR小区个数
     *     <cell_index>: 整型值，NR小区索引，0表示PCell，其余为PSCell或Scell。
     *     <dl_cfg>: 整型值，该小区下行是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <nul_cfg>: 整型值，该小区上行Nul是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <sul_flg>: 整型值，该小区上行Sul是否被配置：
     *             0：未配置；
     *             1：已配置。
     * [示例]:
     *       NR CA信息主动上报
     *       ^NRCACELLRPT: 2,0,1,1,1,1,0,1,1
     *       NR CA小区个数变为0时主动上报
     *       ^NRCACELLRPT: 0
     */
    { AT_STRING_NRCACELLRPT, (VOS_UINT8 *)"^NRCACELLRPT" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR网络能力主动上报命令
     * [说明]: UE主动上报NR某网络能力项参数。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^NRNWCAP:  <type>,<para1>,<para2>,<para3>,<para4>,<para5>,<para6>,<para7>,<para8>,<para9>,<para10><CR><LF>
     * [参数]:
     *     <type>: 整型值，取值0～255。
     *             1：网络SA能力，对应后面<para1>参数有效
     *             其他：暂无特别含义，预留使用
     *     <para1>: 整型值，对应NR网络能力的第一项参数
     *             <type>为1时，取值范围:
     *             0：网络不支持SA;
     *             1：网络支持SA。
     *     <para2>: 整型值，对应NR网络能力的第二项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para3>: 整型值，对应NR网络能力的第三项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para4>: 整型值，对应NR网络能力的第四项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para5>: 整型值，对应NR网络能力的第五项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para6>: 整型值，对应NR网络能力的第六项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para7>: 整型值，对应NR网络能力的第七项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para8>: 整型值，对应NR网络能力的第八项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para9>: 整型值，对应NR网络能力的第九项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para10>: 整型值，对应NR网络能力的第十项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     * [示例]:
     *       主动上报网络的SA能力支持
     *       ^NRNWCAP: 1,1,0,0,0,0,0,0,0,0,0
     */
    { AT_STRING_NRNWCAP, (VOS_UINT8 *)"^NRNWCAP" },
#endif
    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 连接状态主动上报
     * [说明]: 当设备的连接状态发生变化时，MT主动上报此指示给TE。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^NDISSTAT: <stat>,[<err>],[<wx_state>],<PDP_type><CR ><LF>
     * [参数]:
     *     <stat>: 整型值，连接状态。
     *             0：连接断开；
     *             1：已连接；
     *             2：连接中（仅在单板自动连接时上报，暂不支持）；
     *             3：断开中（仅在单板自动断开时上报，暂不支持）。
     *     <err>: 整型值，错误码，仅在拨号失败时上报该参数。
     *             0：unknow error/ unspecified error；
     *             其他值：遵循《3GPP TS 24.008 V5.5.0 (2002-09)》及后续版本中10.5.6.6SM Cause描述。
     *     <wx_state>: 整型值，WiMax数据卡子状态，暂不支持。
     *             1：DL Synchronization；
     *             2：Handover DL acquisition；
     *             3：UL Acquisition；
     *             4：Ranging；
     *             5：Handover ranging；
     *             6：Capabilities negotiation；
     *             7：Authorization；
     *             8：Registration。
     *     <PDP_type>: 取值为字符串。
     *             "IPV4"
     *             "IPV6"
     *             "Ethernet"
     * [示例]:
     *       NDIS设备从断开连接状态变为已连接状态
     *       ^NDISSTAT: 1,,,"IPV6"
     *       NDIS设备从已连接状态变为断开连接状态
     *       ^NDISSTAT: 0,,,"IPV4"
     *       NDIS拨号为IPV4V6类型，拨号失败，SM cause为”27”
     *       ^NDISSTAT: 0,27,,"IPV4"
     *       ^NDISSTAT: 0,27,,"IPV6"
     */
    { AT_STRING_NDISSTAT, (VOS_UINT8 *)"^NDISSTAT" },
    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 连接状态主动上报扩展命令
     * [说明]: 当设备的连接状态发生变化时，MT主动上报此指示给TE。只用于E5形态APP端口拨号上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^NDISSTATEX: <cid>,<stat>,[<err>],[<wx_state>],<PDP_type><CR><LF>
     * [参数]:
     *     <cid>: cid信息
     *     <stat>: 整型值，连接状态。
     *             0：连接断开；
     *             1：已连接；
     *             2：连接中（仅在单板自动连接时上报，暂不支持）；
     *             3：断开中（仅在单板自动断开时上报，暂不支持）。
     *     <err>: 整型值，错误码，仅在拨号失败时上报该参数。
     *             0：unknow error/ unspecified error；
     *             其他值：遵循《3GPP TS 24.008 V5.5.0 (2002-09)》及后续版本中10.5.6.6SM Cause描述。
     *     <wx_state>: 整型值，WiMax数据卡子状态，暂不支持。
     *             1：DL Synchronization；
     *             2：Handover DL acquisition；
     *             3：UL Acquisition；
     *             4：Ranging；
     *             5：Handover ranging；
     *             6：Capabilities negotiation；
     *             7：Authorization；
     *             8：Registration。
     *     <PDP_type>: 取值为字符串。
     *             "IPV4"
     *             "IPV6"
     *             "Ethernet"
     * [示例]:
     *       设备从断开连接状态变为已连接状态
     *       ^NDISSTATEX: 1,1,,,"IPV6"
     *       设备从已连接状态变为断开连接状态
     *       ^NDISSTAT: 1,0,,,"IPV4"
     *       拨号为IPV4V6类型，拨号失败，SM cause为”27”
     *       ^NDISSTATEX: 1,0,27,,"IPV4"
     *       ^NDISSTATEX: 1,0,27,,"IPV6"
     *       拨号为Ethernet类型，网络去激活，SM cause为”36”
     *       ^NDISSTATEX: 1,0,36,,"Ethernet"
     */
    { AT_STRING_NDISSTATEX, (VOS_UINT8 *)"^NDISSTATEX" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 信号强度探测结果主动上报命令
     * [说明]: 该命令当前主要用于AP下发的搜网探测命令后，探测结果返回时，通过该命令以主动上报的方式通知AP指定PLMN的最大信号强度。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^DETECTPLMN: <plmn>,<rat>,<rsrp><CR><LF>
     * [参数]:
     *     <plmn>: 数字类型，要搜索的PLMN信息。
     *     <rat>: 整型值，无线接入技术(当前只支持LTE)。
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE。
     *     <rsrp>: 对LTE：
     *             <rsrp>，取值范围<-141,-44>，0为无效值；搜网失败，按无效值0返回
     * [示例]:
     *     · AP下发指定PLMN+RAT探测请求后，探测成功，都会上报AP探测结果：
     *       AT^DETECTPLMN=46085,7
     *       OK
     *       ^DETECTPLMN: 46085,7,-100
     *     · AP下发指定PLMN+RAT探测请求后，如果接入层返回探测结果，或者探测过程被主动打断，都会上报AP探测结果：
     *       AT^DETECTPLMN=46085,7
     *       OK
     *       ^DETECTPLMN: 46085,7,0
     *       注：上报的PLMN和RAT为AP探测命令所携带的PLMN和RAT，以供AP区分该次上报是否为之前查询命令所触发；如果探测成功，会透传接入层返回的信号强度；任何原因引起的探测失败，上报信号强度均为0。
     */
    { AT_STRING_DETECTPLMN, (VOS_UINT8 *)"^DETECTPLMN" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 来电指示上报（非扩展模式）
     * [说明]: 来电指示和网络发起的PDP激活请求上报（非扩展模式）。
     *         如果使用非扩展模式的上报，则用户无法分清RING表示PS（Packet Switched domain）域还是CS（Circuit Switched domain）域。建议使用扩展模式的来电指示上报+CRING。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>RING<CR><LF>
     * [参数]:
     * [示例]:
     *       有来电时，并且+CLIP=1
     *       RING
     *       有PDP激活指示时
     *       RING
     */
    { AT_STRING_RING, (VOS_UINT8 *)"RING" },
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 来电指示上报（扩展模式）
     * [说明]: 来电指示和网络发起的PDP激活请求上报（扩展模式）。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CRING: <type><CR><LF>
     * [参数]:
     *     <type>: 业务类型。
     *             VOICE：普通语音呼叫；
     *             SYNC：电路域64kbit/s数据业务呼叫（同步透明模式）；
     *             GPRS <PDP_type>,<PDP_addr>[,[<L2P>][,<APN>]]：网络发起的PDP激活指示。
     *     <PDP_type>: 字符串类型，分组数据协议类型。
     *             IP：网际协议（IPV4）。
     *             IPV6：IPV6协议。
     *             IPV4V6：IPV4和IPV6。
     *     <PDP_address>: 字符串类型，IPV4类型的PDP地址。
     *     <L2P>: 字符串类型，标识TE和MT之间使用的层二的协议，该参数被忽略。
     *     <APN>: 字符串类型，接入点名称，用于选择GGSN或外部分组数据网络；若该值为空，则使用签约值。
     * [示例]:
     *       有来电时，并且+CLIP=1
     *       +CRING: VOICE
     *       +CLIP: "13900000000",129
     *       有PDP激活指示时
     *       +CRING: GPRS "IP","10.161.91.51",,"abc.com"
     */
    { AT_STRING_CRING, (VOS_UINT8 *)"+CRING" },
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: IMS域来电指示上报
     * [说明]: IMS域来电指示，能否主动上报受+CLIP命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>IRING<CR><LF>
     * [参数]:
     * [示例]:
     *       有IMS域来电时，并且+CLIP=1
     *       IRING
     */
    { AT_STRING_IRING, (VOS_UINT8 *)"IRING" },
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: USIMM在接收到底软回调的插拔卡槽指示后，通过AT命令主动上报给应用
     * [说明]: 该命令为主动上报命令，不支持查询。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SIMHOTPLUG: <status><CR><LF>
     * [参数]:
     *     <status>: Status状态。
     *             0：卡处于OUT状态；
     *             1：卡处于IN状态。
     * [示例]:
     *       卡在位状态主动上报
     *       ^SIMHOTPLUG: 1
     */
    { AT_STRING_SIMHOTPLUG, (VOS_UINT8 *)"^SIMHOTPLUG" },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: IMS URSP版本上报
     * [说明]: 该命令用于AP侧将IMS相关的URSP配置信息发送给Modem，如果Modem解析码流异常，会上报当前支持的版本信息
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSURSPVERISON: <version><CR><LF>
     * [参数]:
     *     <version>: AP与IMSA约定的码流解析版本，当前仅支持取值1
     * [示例]:
     *       触发IMS URSP Version信息上报：
     *       ^IMSURSPVERSION: 1
     */
    { AT_STRING_IMSURSPVERSION, (VOS_UINT8 *)"^IMSURSPVERSION" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 呼叫发起指示
     * [说明]: 主动上报命令，在用户发起电路域呼叫时，通知用户UE正在向网络发起呼叫。
     *         该命令主动上报通道为用户发起呼叫时使用的AT通道。此主动上报命令受^CURC命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ORIG: <call_id>,<call_type><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *     <call_type>: 整型值，呼叫类型。
     *             0：语音呼叫；
     *             1：视频通话：单向发送视频，双向语音；
     *             2：视频通话：单向接收视频，双向语音；
     *             3：视频通话：双向视频，双向语音；
     *             9：紧急呼叫。
     * [示例]:
     *       UE向网络发起一个语音呼叫
     *       ^ORIG: 1,0
     */
    { AT_STRING_ORIG, (VOS_UINT8 *)"^ORIG" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 网络接通指示
     * [说明]: 主动上报命令，通知用户发起呼叫后，UE收到了网络接通指示。
     *         该命令主动上报通道为用户发起呼叫时使用的AT通道。此主动上报命令受^CURC命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CONF: <call_id><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     * [示例]:
     *       UE收到网络接通指示
     *       ^CONF: 1
     */
    { AT_STRING_CONF, (VOS_UINT8 *)"^CONF" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 呼叫接通指示
     * [说明]: 主动上报命令，当呼叫接通后，UE通知用户呼叫已经接通。
     *         此主动上报命令受^CURC命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CONN: <call_id>,<call_type><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *     <call_type>: 整型值，呼叫类型。
     *             0：语音呼叫；
     *             3：视频呼叫；
     *             9：紧急呼叫
     * [示例]:
     *       语音呼叫已经接通
     *       ^CONN: 1,0
     */
    { AT_STRING_CONN, (VOS_UINT8 *)"^CONN" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 通话结束指示
     * [说明]: 主动上报命令，在通话结束后，UE通知用户通话结束原因和通话时长。
     *         用户主动结束通话，则上报到用户下发结束通话命令的通道；网络结束通话，则上报到建立呼叫的通道上。此主动上报命令受^CURC命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CEND: <call_id>, <duration>,<no_cli_cause>, <cc_cause><CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *     <duration>: 整型值，通话时长，单位s。
     *     <no_cli_cause>: 整型值，无来电号码原因值。
     *             0：号码不可用；
     *             1：用户拒绝提供号码；
     *             2：受其他服务影响；
     *             3：付费电话；
     *             4：初始值(不需要关注)。
     *     <cc_cause>: 整型值，呼叫错误原因值。
     *             和4.5章节域错误码对应。
     * [示例]:
     *       通话结束
     *       ^CEND: 1,10,0,0
     */
    { AT_STRING_CEND, (VOS_UINT8 *)"^CEND" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 转发IMSA给MAPCON的消息
     * [说明]: 转发IMSA给MAPCON的消息
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSCTRLMSGU: <msg_id>,<msg_len>,<msg_context><CR><LF>
     * [参数]:
     *     <msg_id>: 对应接收模块
     *             0：IMSA
     *             1：MAPCONN
     *     <msg_len>: 消息内容<msg_context>的长度
     *     <msg_context>: 模块间接口消息码流
     * [示例]:
     *       转发IMSA给MAPCON的消息
     *       ^IMSCTRLMSGU: 1,4,"12345678"
     */
    { AT_STRING_IMSCTRLMSGU, (VOS_UINT8 *)"^IMSCTRLMSGU" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 主动上报IMS被叫状态
     * [说明]: 本命令用于IMS域被叫处理状态信息主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSMTRPT: < number >,<status>,< cause><CR><LF>
     * [参数]:
     *     <number>: 被叫来电号码
     *     <status>: 0：收到被叫，开始处理被叫过程
     *             1：被叫异常，未收到RING就结束
     *     <cause>: 参考CS域错误码章节中IMSA的错误原因值
     * [示例]:
     *       IMSA开始处理被叫主动上报
     *       ^IMSMTRPT: 18910963686,0,0
     */
    { AT_STRING_IMSMTRPT, (VOS_UINT8 *)"^IMSMTRPT" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: ECALL会话状态主动上报
     * [说明]: 主动上报ECALL会话的状态
     *         说明：本命令仅用于MBB产品上ecall功能支持的版本，例如BalongV722C60/C70，V711C60，V7R5C60，B5000C60。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ECLSTAT: <ecall_state>[,<description>]<CR><LF>
     * [参数]:
     *     <ecall_state>: 整型值，表示ECALL会话的状态：
     *             0：开始传输MSD数据
     *             1：MSD数据传输成功
     *             2：MSD数据传输异常
     *             3：收到PSAP要求更新MSD数据并重传的指示
     *     <description>: ECALL会话状态的附加描述
     *             当<ecall_state>为 2（MSD数据传输出现异常）时，<description> 表示MSD传输异常的原因：
     *             0：等待PSAP的传输指示超时
     *             1：MSD数据传输超时
     *             2：等待应用层的确认超时
     *             3：其他异常错误
     *             4：网络错误
     *             当<ecall_state>为 3（收到PSAP要求更新MSD数据并重传的指示）时，<description> 表示允许更新MSD的时间窗长度，整数值，单位：毫秒，范围：500~10000。
     * [示例]:
     *       MED开始传输MSD数据
     *       ^ECLSTAT: 0
     */
    { AT_STRING_ECLSTAT, (VOS_UINT8 *)"^ECLSTAT" },
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: eCall AL_ACK信息主动上报命令
     * [说明]: 本命令用于指示eCall会话成功后自动报告AL_ACK的值和时间戳信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ECLREC: <timestamp>,<AL_ACK><CR><LF>
     * [参数]:
     *     <timestamp>: 字符串类型，指示IVS接收PSAP发送的AL_ACK的时间。
     *             格式为：
     *             “YYYY/MM/DD,HH:MM:SS+ZZ”
     *             YYYY的取值范围从2000到2100。
     *     <AL_ACK>: 整数值，当PSAP接收并成功验证MSD数据时，PSAP将AL_ACK发送到IVS，带有Clear Down标志的AL_ACK要求IVS挂断eCall。（参考协议16062  7.5.5）
     *             AL_ACK由4 Bit组成，其中Bit3和Bit4为保留位
     *             Bit1为Format version，取值如下：
     *             0：Format version0；
     *             1：Format version1。
     *             Bit2为Status，取值如下：
     *             0：MSD数据被认证；
     *             1：MSD数据被认证，而且要求IVS挂断eCall。
     * [示例]:
     *       在eCall MSD传输成功的主动上报信息
     *       ^ECLSTAT: 3,4000
     *       ^ECLSTAT: 0
     *       ^ECLSTAT: 1
     *       ^ECLREC: "2018/11/28,03:18:16+32",0
     */
    { AT_STRING_ECLREC, (VOS_UINT8 *)"^ECLREC" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报来电号码
     * [说明]: 设置使能CLIP后，来电时上报主叫号码+CLIP: <number>,<type>。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CLIP: <number>,<type>,,,,<CLI validity><CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 整型值，号码地址类型，其结构如表2-8所示。
     *             Type-of-number（bit[6:4]），取值如下：
     *             000：用户不了解目的地址号码时，选用此值，此时目的地址号码由网络填写。
     *             001：若用户能识别是国际号码时，选用此值；或者认为是国内范围也可以填写。
     *             010：国内号码，不允许加前缀或者后缀。在用户发送国内电话时，选用此值。
     *             011：本网络内的特定号码，用于管理或者服务，用户不能选用此值。
     *             101：号码类型为GSM的缺省7bit编码方式。
     *             110：短号码。
     *             111：扩展保留。
     *             Numbering-plan-identification（bit[3:0]），取值如下：
     *             0000：号码由网络的号码方案确定。
     *             0001：ISDN/telephone numbering plan。
     *             0011：Data numbering plan。
     *             0100：Telex numbering plan。
     *             1000：National numbering plan。
     *             1001：Private numbering plan。
     *             1010：ERMES numbering plan。
     *             说明：当bit[6:4]取值为000、001、010时bit[3:0]才有效。
     *     <CLI validity>: 来电号码显示有效性，其取值如下：
     *             0：CLI有效；
     *             1：CLI被呼叫发起方拒绝；
     *             2：由于发起网络的限制或网络问题造成CLI不可用。
     *             <type>与<CLI validity>之间保留了三个字段。
     * [表]: 号码地址类型的结构
     *       bit:   7,    6,              5,              4,              3,                             2,                             1,                             0,
     *              1,    Type-of-number, Type-of-number, Type-of-number, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification,
     * [示例]:
     *       有来电时
     *       +CLIP: "13988868668",129,"",,"",0
     */
    { AT_STRING_CLIP, (VOS_UINT8 *)"+CLIP" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报连接号码
     * [说明]: 设置使能COLP后，主叫时上报连接号码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+COLP: <number>,<type>[,<subaddr>,<satype>[,<alpha>]]<CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <subaddr>: 字符串类型，子电话号码，格式由<satype>决定，目前不处理此参数。
     *     <satype>: 子电话号码类型，整型值，目前不处理此参数。
     *     <alpha>: 字符串，号码在电话本中对应的姓名，目前不处理此参数。
     * [示例]:
     *     · 发起主叫时
     *       ATD13902100013;
     *       +COLP: "1234567890",129
     *       OK
     */
    { AT_STRING_COLP, (VOS_UINT8 *)"+COLP" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报补充业务指示
     * [说明]: 如果+CSSI被使能，补充业务发生时将上报+CSSI: <code1>[,<index>]。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CSSI: <code1>[,<index>]<CR><LF>
     * [参数]:
     *     <code1>: 整型值，补充业务指示码。
     *             0：已激活无条件呼叫转移；
     *             1：已激活某些有条件呼叫转移；
     *             2：发出的呼叫被转移；
     *             3：发出的呼叫正在等待状态；
     *             4：发起一个CUG呼叫（将带有CUG索引号<index>）；
     *             5：发出的呼叫被禁止；
     *             6：被叫方禁止呼入；
     *             7：对CLIR的禁止被拒绝；
     *             8：呼叫发生偏转；
     *             16：发起了网络发起呼叫的回呼（将带有回呼索引号<index>）。
     *     <index>: 请参见2.4.4 闭合用户群：+CCUG中参数<index>的具体定义。
     * [示例]:
     *       呼叫被转移，收到+CSSI事件上报
     *       +CSSI: 2
     */
    { AT_STRING_STD_CSSI, (VOS_UINT8 *)"+CSSI" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报补充业务指示
     * [说明]: 用于IMS域。
     *         如果+CSSI被使能，补充业务发生时将上报^CSSI: <code1>,[<index>],<callId>[,<number>,<type>]。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CSSI: <code1> ,[<index>],<callId>[,<number>,<type>]<CR><LF>
     * [参数]:
     *     <code1>: 整型值，补充业务指示码。
     *             0：已激活无条件呼叫转移；
     *             1：已激活某些有条件呼叫转移；
     *             2：发出的呼叫被转移（将带有CUG索引号<index>）；
     *             3：发出的呼叫正在等待状态；
     *             4：发起一个CUG呼叫（将带有CUG索引号<index>）；
     *             5：发出的呼叫被禁止；
     *             6：被叫方禁止呼入；
     *             7：对CLIR的禁止被拒绝；
     *             8：呼叫发生偏转；
     *             16：发起了网络发起呼叫的回呼（将带有回呼索引号<index>）。
     *     <callId>: 整型值，呼叫ID，取值范围1~7
     *     <index>: 请参见2.4.4 闭合用户群：+CCUG中参数<index>的具体定义。
     *     <number>: 字符串类型，电话号码，格式由<type>决定
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     * [示例]:
     *       IMS域语音呼叫被转移，收到^CSSI事件上报
     *       ^CSSI: 2,1,1,"13800138000",129
     */
    { AT_STRING_CUST_CSSI, (VOS_UINT8 *)"^CSSI" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报补充业务指示
     * [说明]: 如果+CSSU被使能，补充业务发生时将上报+CSSU: <code2>[,<index>[, <number>,<type>]]。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CSSU: <code2>[,<index>[,<number>,<type>]]<CR><LF>
     * [参数]:
     *     <index>: 请参见2.4.4 闭合用户群：+CCUG中参数<index>的具体定义。
     *     <code2>: 整型值，补充业务指示码。
     *             0：来电是一个被转移的呼叫；
     *             1：来电是一个CUG呼叫；
     *             2：通话被对方保持；
     *             3：通话被对方恢复；
     *             4：加入多方通话中；
     *             5：保持的通话被挂断；
     *             6：收到前向检查的补充业务消息；
     *             7：通话被转移到第三方，主叫显示的被叫号码是第三方号码；
     *             8：通话被转移到第三方，被叫显示的被叫号码是第三方号码；
     *             9：来电是一个被偏转的呼叫；
     *             10：来电被转移；
     *             22：呼入电话是网络发起的呼叫。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     * [示例]:
     *       通话中，被对方加入多方通话，收到+CSSU事件上报
     *       +CSSU: 4
     */
    { AT_STRING_STD_CSSU, (VOS_UINT8 *)"+CSSU" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报补充业务指示
     * [说明]: 用于IMS域。
     *         如果+CSSU被使能，补充业务发生时将上报^CSSU: <code2> [,<index>[, <number>,<type>]]。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CSSU: <code2>[,<index>[,<number>,<type>]]<CR><LF>
     * [参数]:
     *     <index>: 整型值，闭合用户群的索引值，默认值为0。
     *             0～9：群组的索引值；
     *             10：无索引（用户在网侧签约的首选的群组）。
     *     <code2>: 整型值，补充业务指示码。
     *             0：来电是一个被转移的呼叫；
     *             1：来电是一个CUG呼叫；
     *             2：通话被对方保持；
     *             3：通话被对方恢复；
     *             4：加入多方通话中；
     *             5：保持的通话被挂断；
     *             6：收到前向检查的补充业务消息；
     *             7：通话被转移到第三方，主叫显示的被叫号码是第三方号码；
     *             8：通话被转移到第三方，被叫显示的被叫号码是第三方号码；
     *             9：来电是一个被偏转的呼叫；
     *             10：来电被转移；
     *             22：呼入电话是网络发起的呼叫。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             Type-of-number（bit[6:4]），取值如下：
     *             000：用户不了解目的地址号码时，选用此值，此时目的地址号码由网络填写。
     *             001：若用户能识别是国际号码时，选用此值；或者认为是国内范围也可以填写。
     *             010：国内号码，不允许加前缀或者后缀。在用户发送国内电话时，选用此值。
     *             011：本网络内的特定号码，用于管理或者服务，用户不能选用此值。
     *             101：号码类型为GSM的缺省7bit编码方式。
     *             110：短号码。
     *             111：扩展保留。
     *             Numbering-plan-identification（bit[3:0]），取值如下：
     *             0000：号码由网络的号码方案确定。
     *             0001：ISDN/telephone numbering plan。
     *             0011：Data numbering plan。
     *             0100：Telex numbering plan。
     *             1000：National numbering plan。
     *             1001：Private numbering plan。
     *             1010：ERMES numbering plan。
     *             说明：当bit[6:4]取值为000、001、010时bit[3:0]才有效。
     * [示例]:
     *       IMS域语音通话中，被对方加入多方通话，收到^CSSU事件上报
     *       ^CSSU: 4
     */
    { AT_STRING_CUST_CSSU, (VOS_UINT8 *)"^CSSU" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报补充业务指示
     * [说明]: 用于IMS域。
     *         如果+CSSU被使能，补充业务发生时将上报^CSSUEX: <code2>,[<index>],<callId>[,<number>,<type>[,<forward_cause>]]。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CSSUEX: <code2>,[<index>],<callId>[,<number>,<type>[,<forward_cause>]]<CR><LF>
     * [参数]:
     *     <code2>: 整型值，补充业务指示码。
     *             0：来电是一个被转移的呼叫；
     *             1：来电是一个CUG呼叫；
     *             2：通话被对方保持；
     *             3：通话被对方恢复；
     *             4：加入多方通话中；
     *             5：保持的通话被挂断；
     *             6：收到前向检查的补充业务消息；
     *             7：通话被转移到第三方，主叫显示的被叫号码是第三方号码；
     *             8：通话被转移到第三方，被叫显示的被叫号码是第三方号码；
     *             9：来电是一个被偏转的呼叫；
     *             10：来电被转移；
     *             22：呼入电话是网络发起的呼叫。
     *     <callId>: 整型值，呼叫ID，取值范围1~7
     *     <index>: 整型值，闭合用户群的索引值，默认值为0。
     *             0～9：群组的索引值；
     *             10：无索引（用户在网侧签约的首选的群组）。
     *     <number>: 字符串类型，电话号码，格式由<type>决定
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             Type-of-number（bit[6:4]），取值如下：
     *             000：用户不了解目的地址号码时，选用此值，此时目的地址号码由网络填写。
     *             001：若用户能识别是国际号码时，选用此值；或者认为是国内范围也可以填写。
     *             010：国内号码，不允许加前缀或者后缀。在用户发送国内电话时，选用此值。
     *             011：本网络内的特定号码，用于管理或者服务，用户不能选用此值。
     *             101：号码类型为GSM的缺省7bit编码方式。
     *             110：短号码。
     *             111：扩展保留。
     *             Numbering-plan-identification（bit[3:0]），取值如下：
     *             0000：号码由网络的号码方案确定。
     *             0001：ISDN/telephone numbering plan。
     *             0011：Data numbering plan。
     *             0100：Telex numbering plan。
     *             1000：National numbering plan。
     *             1001：Private numbering plan。
     *             1010：ERMES numbering plan。
     *             说明：当bit[6:4]取值为000、001、010时bit[3:0]才有效。
     * [示例]:
     *       IMS域语音通话中，被对方保持，收到^CSSU事件上报
     *       ^CSSU: 2,,1
     */
    { AT_STRING_CSSUEX, (VOS_UINT8 *)"^CSSUEX" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报呼叫等待
     * [说明]: 设置使能呼叫等待结果码上报后，通话中有来电时上报+CCWA: <number>,<type>,<class>,,<CLI_validity>。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CCWA: [<number>],[<type>],<class>,,<CLI_validity><CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <class>: 整型值，业务类别，取值为以下业务类别的组合，默认值为255。
     *             1：voice (telephony)；
     *             2：data；
     *             4：fax (facsimile services)；
     *             8：short message service；
     *             16：data circuit sync；
     *             32：data circuit async；
     *             64：dedicated packet access；
     *             128：dedicated PAD access。
     *     <CLI_validity>: 整型值，表明<number>中未能显示呼叫号码的原因。
     *             0：显示号码有效
     *             1：用户拒绝提供
     *             2：网络内部原因
     *             3：公共电话
     *             4：不可用
     * [示例]:
     *       在通话中，有新的来电，网络不提供号码
     *       +CCWA: "",128,1,,0
     *       在通话中，有新的来电，网络提供号码
     *       +CCWA: "12345678",145,1,,0
     */
    { AT_STRING_STD_CCWA, (VOS_UINT8 *)"+CCWA" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报呼叫等待
     * [说明]: 用于IMS域。
     *         设置使能+CCWA呼叫等待结果码上报后，通话中有来电时上报^CCWA: <number>,<type>,<class>。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CCWA: [<number>],[<type>],<class><CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <class>: 业务类别。
     *             1：语音业务。
     *             2：视频业务。
     * [示例]:
     *       在通话中，有新的IMS域语音来电
     *       ^CCWA: "12345678",145,1
     */
    { AT_STRING_CUST_CCWA, (VOS_UINT8 *)"^CCWA" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报USSD业务
     * [说明]: 网络响应TE发起的USSD业务，或网络发起USSD业务的请求时，上报
     *         +CUSD: <m>[,<str>,<dcs>]。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CUSD: <m>[,<str>,<dcs>]<CR><LF>
     * [参数]:
     *     <m>: 整型值。
     *             0：网络不需要TE回复（网络发起的USSD-Notify，或者TE发起操作以后网络并不需要进一步的信息）；
     *             1：网络需要TE回复（网络发起的USSD-Request，或者TE发起操作以后网络需要进一步的信息）；
     *             2：USSD会话被网络释放；
     *             3：其他本地客户端已经作出响应；
     *             4：操作不支持；
     *             5：网络超时。
     *     <str>: USSD字符串，最大长度为160个字符。
     *     <dcs>: 整型值，编码方式。
     *             15：不指定语言的缺省7bit编码；
     *             68：8bit编码；
     *             72：UCS2编码。
     * [示例]:
     *       发起一个USSD操作后，网络回复
     *       +CUSD: 1,"12345678",68
     */
    { AT_STRING_CUSD, (VOS_UINT8 *)"+CUSD" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 用户信令服务1主动上报指示
     * [说明]: MO过程中或主动挂掉电话过程中，收到网络发送的消息中包含User-to-User Information Element信息时，如果+CUUS1I使能开关打开，需要以+CUUS1I格式上报给应用。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CUUS1I: <messageI>,<UUIE><CR><LF>
     * [参数]:
     *     <messageI>: +CUUS1I的结果码上报的消息类型。
     *             0：ANY；
     *             1：ALERT；
     *             2：PROGRESS；
     *             3：CONNECT (sent after +COLP if enabled)；
     *             4：RELEASE。
     *     <UUIE>: 16进制字符，等同于协议24008规定的User-to-User Information Element协议格式。
     * [示例]:
     *       收到网络消息PROGRESS中携带User-to-User Information Element信息
     *       +CUUS1I: 2,7e0110
     */
    { AT_STRING_CUUS1I, (VOS_UINT8 *)"+CUUS1I" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 用户信令服务1主动上报指示
     * [说明]: MT过程中或对方挂掉电话过程中，收到网络发送的消息中包含User-to-User Information Element信息时，如果+CUUS1U使能开关打开，需要以+CUUS1U格式上报给应用。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CUUS1U: <messageU>,<UUIE><CR><LF>
     * [参数]:
     *     <messageU>: +CUUS1U的结果码上报的消息类型。
     *             0：ANY；
     *             1：SETUP (returned after +CLIP if presented, otherwise after every RING or +CRING)；
     *             2：DISCONNECT；
     *             3：RELEASE_COMPLETE。
     *     <UUIE>: 16进制字符，等同于协议24008规定的User-to-User Information Element协议格式。
     * [示例]:
     *       收到网络消息DISCONNECT中携带User-to-User Information Element信息
     *       +CUUS1U: 2,7e021010
     */
    { AT_STRING_CUUS1U, (VOS_UINT8 *)"+CUUS1U" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主动上报来电名称
     * [说明]: 打开主叫名称显示功能后，来电是上报主叫名称+CNAP: <name>,<CNI_validity>。
     *         本命令遵从3GPP TS 27.007协议。
     *         本命令不支持显示UCS-2编码格式的主叫名称，网络上报UCS-2格式时，参数<name>显示为空，可通过^CNAP查询Unicode编码格式码流。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CNAP: <name>,<CNI_validity><CR><LF>
     * [参数]:
     *     <name>: 字符串类型，主叫方名称，最多80个字符。
     *     <CNI_validity>: 整型值，来电名称显示有效性：
     *             0：CNI有效；
     *             1：CNI被主叫方拒绝显示；
     *             2：由于发起网络的限制或网络问题造成CNI不可用。
     * [示例]:
     *       来电时主动上报主叫名称
     *       +CNAP: "John Doe",0
     */
    { AT_STRING_CNAP, (VOS_UINT8 *)"+CNAP" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 上报短信内容
     * [说明]: 短信内容上报。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: if text mode (+CMGF=1):
     *             <CR><LF>+CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data><CR><LF>
     *             if PDU mode (+CMGF=0):
     *             <CR><LF>+CMT: [<alpha>],<length><CR><LF><pdu><CR><LF>
     * [参数]:
     * [示例]:
     *       文本模式短信内容上报
     *       +CMT: "13900000000",,"02/11/19,09:58:42+00",129,36,0,0,"+8613800000000",145,5
     *       Hello
     *       +CSCS设置TE和MT之间的字符集为GSM7BIT编码，短信内容通过二进制码流输出，例如：文本@123
     *       +CMT: "13900000000",,"02/11/19,09:58:42+00",129,36,0,0,"+8613800000000",145,5
     *       00313233
     *       PDU模式短信内容上报
     *       +CMT: ,24
     *       0891683108703705F1040D91683109120090F60008603070
     */
    { AT_STRING_CMT, (VOS_UINT8 *)"+CMT" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 上报短信状态
     * [说明]: 短信状态上报。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: if text mode (+CMGF=1):
     *             <CR><LF>+CDS: <fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st><CR><LF>
     *             if PDU mode (+CMGF=0):
     *             <CR><LF>+CDS: <length><CR><LF><pdu><CR><LF>
     * [参数]:
     * [示例]:
     *       文本模式短信状态上报
     *       +CDS: 2,10,"358501234567",129,"98/07/04/13:12:14+04",
     *       "98/07/04/13:12:20+04",0
     */
    { AT_STRING_CDS, (VOS_UINT8 *)"+CDS" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 上报短信索引
     * [说明]: 短信索引上报。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CMTI: <mem>,<index><CR><LF>
     * [参数]:
     * [示例]:
     *       短信索引上报
     *       +CMTI: "SM",1
     */
    { AT_STRING_CMTI, (VOS_UINT8 *)"+CMTI" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 上报短信状态索引
     * [说明]: 短信状态索引上报。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+CDSI: <mem>,<index><CR><LF>
     * [参数]:
     * [示例]:
     *       短信状态索引上报
     *       +CDSI: "SM",1
     */
    { AT_STRING_CDSI, (VOS_UINT8 *)"+CDSI" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 存储器容量满上报
     * [说明]: 当前的存储器满时，收到新短信时主动上报^SMMEMFULL，并回复网络接收短信失败，原因为无可用存储空间。
     *         当前的存储器满时，收到用户的写短信请求，先回复请求处理失败，然后主动上报^SMMEMFULL。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SMMEMFULL: [<mem>]<CR><LF>
     * [参数]:
     * [示例]:
     *       当前的存储器为(U)SIM，且存储器容量满时，收到新短信上报
     *       ^SMMEMFULL: "SM"
     *     · 当前写操作的存储器为(U)SIM，且存储器容量满时，收到用户的文本格式写短信请求
     *       AT+CMGW="13800000000"
     *       〉HELLO,WORLD.
     *       +CMS ERROR: memory full
     *       ^SMMEMFULL: "SM"
     */
    { AT_STRING_SMMEMFULL, (VOS_UINT8 *)"^SMMEMFULL" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 上报小区广播内容
     * [说明]: 小区广播(CBS)内容上报。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: if text mode (+CMGF=1):
     *             <CR><LF>+CBM: <sn>,<mid>,<dcs>,<page>,<pages><CR><LF><data><CR><LF>
     *             if PDU mode (+CMGF=0):
     *             <CR><LF>+CBM: <length><CR><LF><pdu><CR><LF>
     * [参数]:
     *     <sn>: 小区广播的序列号。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <mid>: 小区广播的消息ID。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <dcs>: 小区广播消息的编码方案。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <page>: 小区广播消息的页序号。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <pages>: 小区广播消息的总页数。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <data>: 解码后的小区广播内容。
     *     <length>: 整型值，PDU数据的字节数。
     *     <pdu>: 16进制数字符串，PDU内容。
     *             具体参见3GPP TS 23.041 9.4节。
     * [示例]:
     *       文本模式短信内容上报
     *       +CBM: 12288,4354,0,1,2
     *       CHARSTRING
     *       +CBM: 12288,4354,0,2,2
     *       CHARSTRING
     */
    { AT_STRING_CBM, (VOS_UINT8 *)"+CBM" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: ETWS主信息上报
     * [说明]: ETWS主消息上报。本命令遵从3GPP TS 23.041协议。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ETWSPN: <plmn_id>,<warning_type>,<msg_id>,<sn>,<auth>[,<warning_security_information>]<CR><LF >
     * [参数]:
     *     <plmn_id>: 当前驻留网络的PLMN。
     *     <warning_type>: 告警类型，十六进制形式。
     *             具体参见3GPP TS 23.041 9.4节。
     *     <msg_id>: 消息ID。十六进制形式，
     *             具体参见3GPP TS 23.041 9.4节。
     *     <sn>: 序列号。十六进制形式，
     *             具体参见3GPP TS 23.041 9.4节。
     *     <auth>: 鉴权结果。
     *             0，通过鉴权
     *             1，未通过鉴权
     * [示例]:
     *       ETWS主信息上报
     *       ^ETWSPN: "24005",0000,1105,1000,1
     */
    { AT_STRING_ETWSPN, (VOS_UINT8 *)"^ETWSPN" },
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 拨号成功主动上报
     * [说明]: 该命令在拨号成功后主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^DCONN: <cid>,<pdpType><CR><LF>
     * [参数]:
     *     <cid>: 当前拨号成功的cid。
     *     <pdpType>: 拨号类型：IPV4、IPV6或者Ethernet。
     * [示例]:
     *       拨号成功后上报
     *       ^DCONN: 1,"IPV4"
     */
    { AT_STRING_DCONN, (VOS_UINT8 *)"^DCONN" },
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 拨号断开主动上报
     * [说明]: 该命令在拨号断开后主动上报。
     *         当IPV6地址分配失败或者地址失效后也会主动上报IPV6拨号断开。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^DEND: <cid>,<endCause>,<pdpType>[,<backOffTimer>[,<allowedSscMode>]] <CR><LF>
     * [参数]:
     *     <cid>: 当前拨号断开的cid。
     *     <endCause>: 拨号断开的原因值。
     *     <pdpType>: 拨号类型：IPV4、IPV6或者Ethernet。
     *     <backOffTimer>: 整形值，backOff timer时长，单位s
     *     <allowedSscMode>: 整型值，allowed-SSC-MODE类型，使用掩码表示；
     *             Bit0：为1表示支持SSC MODE1，为0表示不支持
     *             Bit1：为1表示支持SSC MODE2，为0表示不支持
     *             Bit2：为1表示支持SSC MODE3，为0表示不支持
     * [示例]:
     *       IPV6地址分配失败或者地址失效后上报
     *       ^DEND: 1,34,IPV6
     *       拨号断开后上报
     *       ^DEND: 1,255,"IPV4"
     *       拨号断开后上报时，携带backoff timer和allowed SSC mode
     *       ^DEND: 1,255,"IPV4",25,0x03
     */
    { AT_STRING_DEND, (VOS_UINT8 *)"^DEND" },
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: IP地址信息变更上报
     * [说明]: IP 地址新增、删除或变更后，Modem使用此命令通知AP；本命令上报的时候携带变更类型和内容。
     *         注意：AP可以根据本命令中携带的IP地址刷新网卡；也可以收到本命令后使用^DHCP/^DHCPV6命令查询IP地址，根据查询的结果按照原有的流程处理。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IPCHANGE: <cid>,<opreate>,<ip_type>,<ipv4_addr>,<ipv6_addr>
     *             <CR><LF>
     * [参数]:
     *     <cid>: 整型值，数据业务拨号时，AP下发的拨号命令中携带的CID值
     *     <opreate>: 整型值，IP地址变更类型
     *             0：新增IP地址
     *             1：删除IP地址
     *             2：变更IP地址
     *     <ip_type>: 字符串类型，分组数据协议类型
     *             IP：网际协议（IPv4）
     *             IPV6：IPv6协议
     *             IPV4V6：IPv4和IPv6
     *     <ipv4_addr>: 整型值，主机IPv4地址
     *             取值范围0x0000_0000～0xFFFF_FFFF
     *     <ipv6_addr>: 整型值，主机IPv6地址（使用压缩格式）
     * [示例]:
     *       新增IPv4v6地址上报
     *       ^IPCHANGE: 1,0,IPV4V6,0AA80036,fe80::0001:0000
     *       删除IPv6地址fe80:0000:0000:0000:0000:0000:0001:0000上报
     *       ^IPCHANGE: 1,1,IPV6,,fe80::0001:0000
     *       修改IPv4地址上报，新地址为0A.168.0.54
     *       ^IPCHANGE: 1,2,IP,0AA80036
     */
    { AT_STRING_IPCHANGE, (VOS_UINT8 *)"^IPCHANGE" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 指示(U)SIM状态变化
     * [说明]: 当(U)SIM状态发生改变时，主动上报当前(U)SIM卡的状态，同时上报LOCK状态。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^SIMST: <sim_state>[,<lock_state>]<CR><LF>
     * [参数]:
     *     <sim_state>: 整数型，(U)SIM卡状态，取值0～4，240，255。
     *             0：(U)SIM卡状态无效；
     *             1：(U)SIM卡状态有效；
     *             2：(U)SIM在CS下无效；
     *             3：(U)SIM在PS下无效；
     *             4：(U)SIM在PS+CS下均无效；
     *             240：ROMSIM版本；
     *             255：(U)SIM卡不存在，包括(U)SIM卡确实不存在和因为被CardLock锁住而不存在两种情况。此时，应通过<lock_state>的取值判定(U)SIM卡的真实状态。
     *     <lock_state>: 整数型，(U)SIM卡的LOCK状态，取值0～1。
     *             0：(U)SIM卡未被CardLock功能锁定；
     *             1：(U)SIM卡被CardLock功能锁定。
     * [示例]:
     *       当前(U)SIM卡不在位
     *       ^SIMST: 0,0
     *       当前(U)SIM卡有效
     *       ^SIMST: 1,0
     */
    { AT_STRING_SIMST, (VOS_UINT8 *)"^SIMST" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: USIM模块异常事件上报
     * [说明]: USIMM模块在FEATURE_UE_UICC_MULTI_APP_SUPPORT打开，初始化卡的时候，如果有异常事件发生，会通过这个AT命令上报对应的异常原因，异常原因编码为16进制，AP可以根据异常值判断是否需要保存对应的CHR信息。
     *         该命令一次可以上报多个错误值。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^USIMMEX: <Error><CR><LF>
     * [参数]:
     *     <Error>: 0x10000001：调用驱动接口复位卡失败；
     *             0x10000002：调用驱动接口读取ATR失败；
     *             0x10000003：初始化选择UICC的MF失败；
     *             0x10000004：初始化选择EFDIR失败；
     *             0x10000005：初始化读取EFDIR失败；
     *             0x10000006：初始化解析EFDIR失败；
     *             0x10000007：初始化选择ICC卡MF失败；
     *             0x10000008：调用驱动接口切换SIM卡电压失败；
     *             0x10000009：下载Profile Download失败；
     *             0x1000000A：切换电压之后选择MF失败；
     *             0x1000000B：NV9090配置需要CSIM应用存在的检查失败；
     *             0x1000000C：NV9090配置需要CDMA目录存在的检查失败；
     *             0x1000000D：获取PIN1信息错误；
     *             0x1000000E：初始化PIN1剩余次数失败；
     *             0x1000000F：初始化PUK1剩余次数失败；
     *             0x10010000：读取SIM卡必选文件失败，失败的文件ID会放在低2字节上报，下同；
     *             0x10020000：读取USIM卡的必选文件失败；
     *             0x10030000：读取ISIM卡的必选文件失败；
     *             0x10040000：读取UIM卡必选文件失败；
     *             0x10050000：读取CSIM卡必选文件失败；
     *             0x10060000：读取NV失败；
     * [示例]:
     *       ^USIMMEX: 0x0x10000009, 0x10056F22
     */
    { AT_STRING_USIMMEX, (VOS_UINT8 *)"^USIMMEX" },
    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: STK事件通知上报
     * [说明]: 该命令用来通知TE，SIM卡上报了一个主动命令给MS，TE收到该通知后，应下发^STGI命令获取主动命令数据，完成主动请求。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^STIN: <CmdType>,<CmdIndex>,<isTimeOut><CR><LF>
     * [参数]:
     * [示例]:
     *       无。
     */
    { AT_STRING_STIN, (VOS_UINT8 *)"^STIN" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS EMC PDN状态上报
     * [说明]: 该命令在IMS EMC PDN建立/断开时主动上报。
     *         说明：用于IMS紧急呼叫定位特性。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSEMCSTAT: <stat><CR><LF>
     * [参数]:
     *     <stat>: 整型值，IMS EMC PDN连接状态：
     *             0：IMS EMC PDN断开；
     *             1：IMS EMC PDN建立。
     * [示例]:
     *       IMS EMC PDN建立
     *       ^IMSEMCSTAT: 1
     *       IMS EMC PDN断开
     *       ^IMSEMCSTAT: 0
     */
    { AT_STRING_IMSEMCSTAT, (VOS_UINT8 *)"^IMSEMCSTAT" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 清除辅助定位信息指示主动上报
     * [说明]: Modem 在CS域链路释放时上报此命令，用于清除GPS芯片缓存的辅助定位信息。
     *         该命令同时受^XCPOSR 和^XCPOSRRPT命令控制。
     *         在GPS芯片支持清除辅助定位信息功能时，且打开了主动上报清除辅助定位信息指示时才会上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^XCPOSRRPT: <value><CR><LF>
     * [参数]:
     *     <value>: 整数型，清除辅助定位信息，取值1。
     *             1：GPS芯片清除辅助定位信息。
     * [示例]:
     *       清除辅助定位信息指示的主动上报功能支持且打开时Modem的CS域链路释放：
     *       ^XCPOSRRPT: 1
     */
    { AT_STRING_XCPOSRRPT, (VOS_UINT8 *)"^XCPOSRRPT" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: Call Control或MO SMS Control结果上报
     * [说明]: 该命令用于Call Control或MO SMS Control结果主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CCIN: <Type>,<Result>,<Alpha_id>[,<service_center_address>,<service_center_address_type>,<services_center_address_type>,<destination_address>,<destination_address_type>,<dcs>,<data>]<CR><LF>
     * [参数]:
     *     <Type>: 整形值，Call/SMS Control的类型。
     *             0：MO Call Control by USIM；
     *             1：SS Call Control by USIM；
     *             2：USSD Call Control by USIM；
     *             3：SMS Control by USIM。
     *     <Result>: 整形值，Call/SMS Control的结果。
     *             0：允许；
     *             1：不允许；
     *             2：允许但号码变更；
     *             3：USIM忙。
     *     <Alpha_id>: 字符串类型，字符标识。
     *     <service_center_address>: 字符串类型，服务中心号码。
     *     <service_center_address_type>: 整形值，服务中心号码类型。
     *     <destination_address>: 字符串类型，接收号码。
     *     <destination_address_type>: 整形值，接收号码类型。
     *     <dcs>: 整形值，编码方式。
     *     <data>: 字符串类型，USSD字串。
     * [示例]:
     *       MO Call Control结果上报
     *       ^CCIN: 0,2,"","010203",145
     */
    { AT_STRING_CCIN, (VOS_UINT8 *)"^CCIN" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 温保状态主动上报命令
     * [说明]: 该命令用于在温保状态发生变化时主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^THERM: <value><CR><LF>
     * [参数]:
     *     <value>: 0：退出温度保护；
     *             1：进入温度保护。
     * [示例]:
     *       进入温度保护状态，主动上报如下：
     *       ^THERM:1
     *       退出温度保护状态，主动上报如下：
     *       ^THERM:0
     */
    { AT_STRING_THERM, (VOS_UINT8 *)"^THERM" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 主动上报上电协议栈初始化结果
     * [说明]: 主动上报上电时协议栈初始化结果。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^PSINIT: <psinit_result><CR><LF>
     * [参数]:
     *     <psinit_result>: 整型值，上电协议栈初始化结果。
     *             0：初始化失败；
     *             1：初始化成功；
     *             2：物理层初始化失败（V8R1使用）；
     *             3：物理层初始化超时
     * [示例]:
     *       上电协议栈初始化成功
     *       ^PSINIT: 1
     *       上电协议栈初始化失败
     *       ^PSINIT: 0
     */
    { AT_STRING_PSINIT, (VOS_UINT8 *)"^PSINIT" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS注册域切换指示
     * [说明]: IMS业务过程中，IMS注册域的切换指示。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSRATHO: <ho_status>,<source_rat>,<target_rat>,<cause_code><CR><LF>
     * [参数]:
     *     <ho_status>: 整型值，IMS注册域的切换状态：
     *             0： RAT handover was successful
     *             1： RAT handover failed
     *             2： RAT handover could not be triggered
     *     <source_rat>: 整型值，原始域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <target_rat>: 整型值，目标域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <cause_code>: 整型值，切换失败原因
     * [示例]:
     *       IMS注册域由WIFI成功切换到LTE
     *       ^IMSRATHO: 0,1,0,0
     */
    { AT_STRING_IMSRATHO, (VOS_UINT8 *)"^IMSRATHO" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS服务状态更新指示
     * [说明]: IMS服务状态或者注册状态发生变化的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^IMSSRVSTATUS: <sms_srv_status>,<sms_srv_rat>,<voip_srv_status>,<voip_srv_rat>,<vt_srv_status>,<vt_srv_rat>,<vs_srv_status>,<vs_srv_rat><CR><LF>
     * [参数]:
     *     <sms_srv_status>: 整型值，IMS短信的服务状态：
     *             0： IMS SMS service is not available
     *             1： IMS SMS is in limited service
     *             2： IMS SMS is in full service
     *     <sms_srv_rat>: 整型值，IMS短信的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <voip_srv_status>: 整型值，IMS VoIP的服务状态：
     *             0： IMS VoIP service is not available
     *             1： IMS VoIP is in limited service
     *             2： IMS VoIP is in full service
     *     <voip_srv_rat>: 整型值，IMS VoIP的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <vt_srv_status>: 整型值，IMS VT的服务状态：
     *             0： IMS VT service is not available
     *             1： IMS VT is in limited service
     *             2： IMS VT is in full service
     *     <vt_srv_rat>: 整型值，IMS VT的服务状态：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <vs_srv_status>: 整型值，IMS VS的服务状态：
     *             0： IMS VS service is not available
     *             1： IMS VS is in limited service
     *             2： IMS VS is in full service
     *     <vs_srv_rat>: 整型值，IMS VS的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered onUtran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     * [示例]:
     *       IMS的服务状态发生变化
     *       ^IMSSRVSTATUS: 2,0,2,0,2,0,2,0
     */
    { AT_STRING_IMSSRVSTATUS, (VOS_UINT8 *)"^IMSSRVSTATUS" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: ENDC双连接状态变化主动上报
     * [说明]: 该命令用于LTE下ENDC双连接状态变化的主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LENDC: <endc_available>,<endc_plmn_avail>,<endc_restricted>,<nr_pscell> <CR><LF>
     * [参数]:
     *     <endc_available>: 整数值，当前小区是否支持Eutran-NR模式：
     *             0：不支持；
     *             1：支持。
     *     <endc_plmn_avail>: PLMN LIST中是否有PLMN支持Eutran-NR模式：
     *             0：代表没有PLMN支持Eutran-NR模式；
     *             1：代表有PLMN支持Eutran-NR模式。
     *     <endc_restricted>: 核心网是否限制Eutran-NR能力：
     *             0：代表核心网没有限制endc能力；
     *             1：代表核心网限制endc能力。
     *     <nr_pscell>: 整型值，当前PSCell是否为NR，即是否进入ENDC双连接状态：
     *             0：非ENDC状态；
     *             1：ENDC已经建立状态。
     * [示例]:
     *       主动上报ENDC双连接状态变化
     *       ^LENDC: 1,1,0,1
     */
    { AT_STRING_LENDC, (VOS_UINT8 *)"^LENDC" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: Modem链接状态主动上报
     * [说明]: Modem在链接状态发生变化时主动上报此命令。
     *         本命令仅提供给Modem与Wifi共天线方案使用，用于上层判断Wifi天线切换MIMO或SISO模式。
     *         当前链接状态变化涉及主要业务流程如下：CS域普通呼叫、CS域紧急呼叫、CS域短信业务、CS域补充业务、LAU流程、去注册流程、RAU流程、TAU流程、PS域数据业务信令流程、PS域短信业务等。
     *         本命令受^CRRCONN设置命令控制。
     *         该命令上报的链接状态依据是NAS层业务起始状态，并不完全等同接入层的连接态。
     *         该命令仅在Modem0上主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: Modem链接状态发生变化时：
     *             <CR><LF>^CRRCONN: <status0>,<status1>,<status2> <CR><LF>
     * [参数]:
     *     <status0>: 整型值，Modem0的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     *     <status1>: 整型值，Modem1的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     *     <status2>: 整型值，Modem2的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     * [示例]:
     *       Modem0由非连接态进入连接态，Modem1、Modem2均为非连接态
     *       ^CRRCONN: 1,0,0
     */
    { AT_STRING_CRRCONN, (VOS_UINT8 *)"^CRRCONN" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: VoLTE诊断信息主动上报
     * [说明]: AP打开LTE空口质量信息上报开关后，Modem将周期性上报此命令。用于通知AP在VoLTE视频通话中空口速率的LPDCP诊断信息。
     *         该命令受^VTRLQUALRPT命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LPDCPINFORPT: <CurrBuffTime>,<CurrBuffPktNum>, <MacUlThrput>,<MaxBuffTime><CR><LF>
     * [参数]:
     *     <CurrBuffTime>: 整型值，最老数据包缓存时长，单位ms。
     *     <CurrBuffPktNum>: 整型值，当前缓存数据包个数，单位个。
     *     <MacUlThrput>: 整型值，MAC上行速率，只包含上行新授权速率，不包含重传授权，单位bytes/s。
     *     <MaxBuffTime>: 整型值，数据包最大缓存时长，单位ms。
     * [示例]:
     *       开启LTE空口质量信息上报后
     *       ^LPDCPINFORPT: 1200,5,2048000,2000
     */
    { AT_STRING_LPDCPINFORPT, (VOS_UINT8 *)"^LPDCPINFORPT" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: LTE链路质量主动上报
     * [说明]: AP打开LTE空口质量信息上报开关后，Modem将周期性上报此命令。用于通知AP当前LTE的链路质量信息。
     *         该命令受^VTRLQUALRPT命令控制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LTERLQUALINFO: <RSRP>,<RSRQ>,<RSSI>,<BLER> <CR><LF>
     * [参数]:
     *     <RSRP>: 整型值，参考信号接收功率，单位dBm。
     *     <RSRQ>: 整型值，参考信号接收质量，单位dBm。
     *     <RSSI>: 整型值，信号强度，单位dBm。
     *     <BLER>: 整型值，误码率。
     * [示例]:
     *       开启LTE空口质量信息上报后
     *       ^LTERLQUALINFO: -95,-15,-89,5
     */
    { AT_STRING_LTERLQUALINFO, (VOS_UINT8 *)"^LTERLQUALINFO" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 干扰检测结果上报
     * [说明]: 干扰检测状态主动上报。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^JDINFO: <state>,<rat><CR><LF>
     * [参数]:
     *     <state>: 整型值，干扰检测结果。
     *             0：干扰不存在
     *             1：干扰存在
     *     <rat>: 整型值，网络制式。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     * [示例]:
     *       干扰检测功能开启后，检测GSM制式干扰存在
     *       ^JDINFO: 1,0
     */
    { AT_STRING_JDINFO, (VOS_UINT8 *)"^JDINFO" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 卡初始化状态上报
     * [说明]: 该命令主动上报卡状态信息(包括卡介质类型、ICCID、IMSI等)。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CARDSTATUSIND: <value><CR><LF>
     * [参数]:
     *     <value>: 卡状态二进制码流
     * [示例]:
     *       开机卡主动上报
     *       ^CARDSTATUSIND：10000000900000000000000010008094601101305756340000000000000000000000000000000000000000000000000000000000000000000000000090000000000000001000007520E8686FC50876100000000000000000000000000000000000000000000000000000000000000000000000009000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010108986111473021185793710000000
     */
    { AT_STRING_CARDSTATUSIND, (VOS_UINT8 *)"^CARDSTATUSIND" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 主动上报LADN信息
     * [说明]: Modem收到网络侧下发的LADN信息，解码后按照当前所处的位置区，主动上报给AP当前区域支持的LADN DNN信息，和当前区域禁止使用的LADN DNN 信息；UE的位置区变更后，如果LADN 关联的DNN可用/不可用信息变化，Modem也使用此命令上报给AP。
     *         注意：主动上报开关默认关闭，如果需要主动上报，AP需要使用^CLADN命令单独配置主动上报功能。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CLADNU: <allowed_ladn_dnn_num>,<allowed_dnn_list>,<nonallowed_ladn_dnn_num>, <nonallowed_dnn_list><CR><LF>
     * [参数]:
     *     <allowed_ladn_num>: 整形值，当前位置区内可以正常使用的LADN DNN个数。
     *     <allowed_dnn_list>: 字符串类型，当前位置区内可以正常使用的LADN DNN；格式如下：“dnn1;dnn2;…dnn8”
     *     <nonallowed_ladn_dnn_num>: 整形值，当前位置区内不可以正常使用的LADN DNN个数。allowed_ladn_num与nonallowed_ladn_num的和需小于等于8。
     *     <nonallowed_dnn_list>: 字符串类型，当前位置区内不可以正常使用的LADN DNN；格式如下：“dnn1;dnn2;…dnn8”
     * [示例]:
     *       可用/不可用LADN DNN同时存在时
     *       ^CLADNU: 1,"Ladn.dnn.01",2,"Ladn.dnn.02;Ladn.dnn.03"
     *       NR去注册、或者清除LADN信息时
     *       ^CLADNU: 0,"",0,""
     */
    { AT_STRING_CLADNU, (VOS_UINT8 *)"^CLADNU" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 高铁小区信息主动上报
     * [说明]: MODEM判断当前LRRC，NR的小区是否有变化，触发主动上报^ HSRCELLINFO。
     *         注：此命令由FEATRUE_MBB_HSRCELLINFO宏控制，宏为产品线定制打开。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^HSRCELLINFO: <Rat>,<HighSpeedFlg>,<Rsrp>,<CellIdLowBit>[,<CellIdHighBit>]<CR><LF>
     * [参数]:
     *     <Rat>: 整型值，当前的无线接入技术。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     *             3：TD-SCDMA
     *             5：NR
     *             6：UNKNOWN
     *     <HighSpeedFlg>: 高铁小区标识信息，整形值。取值：
     *             0：非高铁小区
     *             1：高铁小区
     *     <Rsrp>: 整型值，参考信号接收功率。
     *     <CellIdLowBit>: 小区cell id，整形值
     *             如果rat为“LTE”，则表示LTE的小区id。
     *             如果rat为“NR”，和<CellIdHighBit>合起来表示NR的小区id
     *     <CellIdHighBit>: 如果rat为“NR”， 和<CellIdLowBit>合起来表示NR的小区id
     * [示例]:
     *       主动上报高铁线路LTE小区信息信息
     *       ^HSRCELLINFO: 2,1,-90,280
     *       主动上报高铁线路NR小区信息信息
     *       ^HSRCELLINFO: 5,1,-90,280,0
     */
    { AT_STRING_HSRCELLINFO, (VOS_UINT8 *)"^HSRCELLINFO" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 主动上报用于通知AP进行主动链路恢复快速添加SCG
     * [说明]: 主动快速回5G（NSA）需求，在满足快速填加SCG场景条件下，LRRC通知AP，AP触发主动链路恢复快速添加SCG的操作。5G下使用。
     * [语法]:
     *     [命令]: -
     *     [结果]: 返回结果：
     *             <CR><LF>^CONNECTRECOVERY <CR><LF>
     * [参数]:
     * [示例]:
     *       在满足快速填加SCG场景条件下，主动上报命令
     *       ^CONNECTRECOVERY
     */
    { AT_STRING_CONNECTRECOVERY, (VOS_UINT8 *)"^CONNECTRECOVERY" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 通话补包状态通知命令
     * [说明]: 上报通话补包业务状态信息给AP侧，如：通话补包特性支持能力。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^FUSIONCALLRAWU: <msg_len>，<msg_id>,<msg_context><CR><LF>
     * [参数]:
     *     <msg_id>: 对应发送模块
     *             0：IMSA
     *             1：IMS SDK
     *     <msg_len>: 消息内容<msg_context>的长度，消息长度最大不超过500
     *     <msg_context>: 模块间接口消息码流
     * [示例]:
     *       上报AP侧启动通话补包业务结果
     *       ^FUSIONCALLRAWU: 0,16,"01010000000001000100000000000000"
     */
    { AT_STRING_FUSIONCALLRAWU, (VOS_UINT8 *)"^FUSIONCALLRAWU" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 开机初搜位置信息上报
     * [说明]: 该命令用于第一次无卡开机初搜后，将获得的位置信息上报给AP侧。在^PSINIT命令上报前上报。
     * [语法]:
     *     [命令]:
     *     [结果]: <CR><LF>^INITLOCINFO: <Mcc>,<Sid><CR><LF>
     * [参数]:
     *     <Mcc>: 手机所在的当前位置的国家码，通过GUL搜网得到。无效值：全F
     *     <Sid>: 手机所在的当前位置的Sid，通过X模搜网得到。无效值：-1
     * [示例]:
     *       上报INITLOCINFO命令，MCC、SID都是有效值
     *       ^INITLOCINFO: 460,23
     *       上报INITLOCINFO命令，MCC是有效值，SID是无效值
     *       ^INITLOCINFO: 460,-1
     *       上报INITLOCINFO命令，MCC是无效值，SID是有效值
     *       ^INITLOCINFO: fff,23
     */
    { AT_STRING_INITLOCINFO, (VOS_UINT8 *)"^INITLOCINFO" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE网络频率信息主动上报
     * [说明]: 当LTE网络频点进入或退出WiFi频率干扰范围时，主动上报^LWURC命令通知应用，规避与WiFi模块的同频干扰。LTE与WiFi的频率干扰范围默认是(23700-24000)，可通过NV项动态调整。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LWURC: <state>,<ulfreq>,<ulbw>,<dlfreq>,<dlbw>,<band>,<ant_num>,<ant1_ulfreq>,<ant1_ulbw>,<ant1_dlfreq>,<ant1_dlbw>,<ant2_ulfreq>,<ant2_ulbw>,<ant2_dlfreq>,<ant2_dlbw>,<ant3_ulfreq>,<ant3_ulbw>,<ant3_dlfreq>,<ant3_dlbw>,<ant4_ulfreq>,<ant4_ulbw>,<ant4_dlfreq>,<ant4_dlbw>,<ant_dlmm><ant1_dlmm><ant2_dlmm><ant3_dlmm><ant4_dlmm>,<dl256QAMflag><CR><LF>
     * [参数]:
     *     <state>: 当前LTE频率与WIFI频率冲突状态。
     *             0：不冲突，即LTE与WIFI不处于频率干扰范围；
     *             1：冲突，即LTE与WIFI处于频率干扰范围；
     *             2：无效值（当前LTE非接入状态，无固定的上行频率）。
     *     <ulfreq>: LTE上行频率，单位100kHz，取值范围：0~4294967295。
     *     <ulbw>: LTE上行带宽。
     *             0：1.4M；
     *             1：3M；
     *             2：5M；
     *             3：10M；
     *             4：15M；
     *             5：20M。
     *     <dlfreq>: LTE下行频率，单位100kHz，取值范围：0~4294967295。
     *     <dlbw>: LTE下行带宽。
     *             0：1.4M；
     *             1：3M；
     *             2：5M；
     *             3：10M；
     *             4：15M；
     *             5：20M。
     *     <band>: LTE模式的频段号，取值范围：1~70。
     *     <ant_num>: 天线的频点和频段个数，取值范围0~3。
     *     <dlmm>: UE MIMO层数
     *     <dl256QAMflag>: 是否支持下行256QAM。
     *             0：不支持；
     *             1：支持。
     */
    { AT_STRING_LWURC, (VOS_UINT8 *)"^LWURC" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 主动上报信号质量信息
     * [说明]: 本命令用于上报信号质量信息。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ANLEVEL: <rscp><ecno><rssi><level><rsrp><rsrq><CR><LF>
     * [参数]:
     *     <rscp>: 整型值，RSCP值，单位为：dBm。
     *     <ecno>: 整型值，ECNO值，单位为：dBm。
     *     <rssi>: 整型值，信号强度，单位：dBm。
     *     <level>: 整型值，信号质量等级。
     *     <rsrp>: 整型值，参考信号接收功率，单位：dBm。
     *     <rsrq>: 整型值，参考信号接收质量，单位：dB。
     * [示例]:
     *       ^ANLEVEL: 0,99,31,4,69,3
     */
    { AT_STRING_ANLEVEL, (VOS_UINT8 *)"^ANLEVEL" },
    { AT_STRING_CARDFETCHIND, (VOS_UINT8 *)"^CARDFETCHIND" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE小区CA状态主动上报
     * [说明]: 用于Modem主动上报LTE小区上行、下行CA配置状态和CA激活状态。
     *         变化条件包括：
     *         上行CA配置打开和关闭、下行CA配置打开和关闭、CA激活和去激活。
     *         该主动上报受^CURC控制，可打开或者关闭。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^LCACELLURC: "<cell_id> <ul_cfg> <dl_cfg> <act>"[, "<cell_id> <ul_cfg> <dl_cfg> <act>"[,......]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cell_id>: 整型值，cell id，0表示Pcell，其他表示Scell。
     *     <ul_cfg>: 整型值，本cell上行CA是否被配置，0表示未配置，1表示已配置。
     *     <dl_cfg>: 整型值，本cell下行CA是否被配置，0表示未配置，1表示已配置。
     *     <act>: 整型值，本cell CA是否被激活，0表示未激活，1表示已激活。
     * [示例]:
     *       主动上报LTE小区CA配置状态
     *       ^LCACELLURC: "0 0 0 0", "1 1 1 1", "2 1 1 1", "3 0 0 0", "4 0 0 0", "5 0 0 0", "6 0 0 0", "7 0 0 0"
     */
    { AT_STRING_LCACELLURC, (VOS_UINT8 *)"^LCACELLURC" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 主动上报定位信息
     * [说明]: 该命令用于AP-Modem形态下，Modem向AP主动上报网络测量指示及定位信息。
     *         命令使用场景：Modem接收到网络的测量指示及定位信息后主动上报。
     *         命令使用限制：只限定于AP-Modem形态使用。
     *         命令使用通道：只能通过与AP对接的AT通道或MUX通道下发。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]:
     *     [结果]: <CR><LF>+CPOSR: <xml><CR><LF>
     * [参数]:
     *     <xml>: 字符串类型，一行XML紧凑格式数据，具体格式详见3GPP 27007协议8.56章节描述。
     * [示例]:
     *       定位信息主动上报
     *       +CPOSR: <?xml version="1.0" encoding="utf-8"?><pos><pos_meas><RRLP_meas><RRLP_pos_instruct><RRLP_method_type><ms_based><method_accuracy>0</method_accuracy></ms_based></RRLP_method_type><RRLP_method literal="gps"/><resp_time_seconds>7</resp_time_seconds><mult_sets literal="one"/></RRLP_pos_instruct></RRLP_meas></pos_meas></pos>
     */
    { AT_STRING_CPOSR, (VOS_UINT8 *)"+CPOSR" },
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 主动上报CLT（天线闭环调谐）状态信息
     * [说明]: 主动上报当前MT工位CLT状态信息(只有WPHY和LPHY才会上报)。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CLTINTO:  <GammaReal>, <GammaImag>, <GammaAmpUc0>, <GammaAmpUc1>, <GammaAmpUc2>, <GammaAntCoarseTune>, <FomcoarseTune>, <CltAlgState>, <CltDetectCount>, <Dac0>, <Dac1>, <Dac2>, <Dac3> <CR><LF>
     * [参数]:
     *     <GammaReal>: 有符号整型值，反射系数实部
     *     <GammaImag>: 有符号整型值，反射系数虚部
     *     <GammaAmpUc0>: 无符号整型值，驻波检测场景0反射系数幅度
     *     <GammaAmpUc1>: 无符号整型值，驻波检测场景1反射系数幅度
     *     <GammaAmpUc2>: 无符号整型值，驻波检测场景2反射系数幅度
     *     <GammaAntCoarseTune>: 无符号整型值，粗调格点位置
     *     <FomcoarseTune>: 无符号整型值，粗调FOM值
     *     <CltAlgState>: 无符号整型值，闭环算法收敛状态
     *     <CltDetectCount>: 无符号整型值，闭环收敛总步数
     *     <Dac0>: 无符号整型值，Dac0
     *     <Dac1>: 无符号整型值，Dac1
     *     <Dac2>: 无符号整型值，Dac2
     *     <Dac3>: 无符号整型值，Dac3
     * [示例]:
     *       主动上报
     *       ^CLTINFO: 3150,0,4465,2214,3150,22536,185,17,3,106,11,0,0
     */
    { AT_STRING_CLTINFO, (VOS_UINT8 *)"^CLTINFO" },

    { AT_STRING_BUTT, (TAF_UINT8 *)"\"\"" },
};

VOS_UINT8       g_atCrLf[]        = "\r\n";         /* 回车换行 */
const TAF_UINT8 g_atCmeErrorStr[] = "+CME ERROR: "; /* 错误提示字串 */
const TAF_UINT8 g_atCmsErrorStr[] = "+CMS ERROR: "; /* 错误提示字串 */

const TAF_UINT8 g_atTooManyParaStr[] = "TOO MANY PARAMETERS"; /* 参数太多提示字串 */

AT_CMD_FORMAT_TYPE g_atCmdFmtType = AT_CMD_TYPE_BUTT;

AT_SEND_DataBuffer g_atSendDataBuff;                       /* 单个命令的返回信息存储区 */
AT_SEND_DataBuffer g_atCombineSendData[AT_MAX_CLIENT_NUM]; /* 组合命令的返回信息存储区 */

/* g_atSendDataBuff.aucBuffer的第0项为MUX的帧头标志 */
TAF_UINT8 *g_atSndCrLfAddr    = &g_atSendDataBuff.buffer[AT_SEND_DATA_BUFF_INDEX_1];
TAF_UINT8 *g_atSndCodeAddress = &g_atSendDataBuff.buffer[AT_SEND_DATA_BUFF_INDEX_3];

TAF_LogPrivacyAtCmdUint32 g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;

/*
 * 调用底软的串口数据发送接口时，所传入的指针必需为4字节对齐格式。
 * 而g_atSndCrLfAddr和g_atSndCodeAddress都没有依照4字节对齐，所以需
 * 做一次拷贝操作。(AT命令处理对性能要求不高，故增加一次拷贝是可行的)
 */
TAF_UINT8 g_atSendDataBuffer[AT_CMD_MAX_LEN];

AT_V_TYPE    g_atVType    = AT_V_ENTIRE_TYPE; /* 指示命令返回码类型 */
AT_CSCS_TYPE g_atCscsType = AT_CSCS_IRA_CODE; /* 指示TE编码类型 */

AT_CMD_ECHO_TYPE g_atEType = AT_E_ECHO_CMD;
VOS_UINT32       g_atXType = 0; /* CONNECT <text> result code is given upon entering online data state.
         Dial tone and busy detection are disabled. */
VOS_UINT32 g_atQType = 0;       /* DCE transmits result codes */

MN_OPERATION_ID_T g_opId = 0;

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT8 g_modemLoop = 0;
#endif

AT_CSTA_NumTypeUint8 g_atCstaNumType = (0x80 | (MN_CALL_TON_INTERNATIONAL << 4) | (MN_CALL_NPI_ISDN));
AT_PB_Info           g_pbatInfo;


TAF_UINT8 g_atPppIndexTab[AT_MAX_CLIENT_NUM]; /* PppId和Index的对应表，下标是PppId */

/* 记录是否解锁，初始值为未解锁，通过命令^DATALOCK解锁成功后修改该变量 */
VOS_BOOL g_atDataLocked = VOS_TRUE;

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
AT_SP_WordCtx g_spWordCtx = {0};
#endif

LOCAL AT_RreturnCodeUint32 AT_ProcNetWorkTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcPppOrgTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcAtdIpCallTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcPsCallEndTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcTmodeTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcAbortCmdTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcUnkonwnTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcCpbrTimerOut(VOS_UINT8 indexNum);
LOCAL AT_RreturnCodeUint32 AT_ProcVmsetTimerOut(VOS_UINT8 indexNum);

static const AT_PS_InternalTimeoutResult g_atInterTimeoutResultTab[] = {
    /* Inter Timer Name */ /* 消息处理函数 */
    { AT_S0_TIMER, AT_RcvTiS0Expired },
    { AT_HOLD_CMD_TIMER, AT_BlockCmdTimeOutProc },
    { AT_SIMLOCKWRITEEX_TIMER, AT_RcvTiSimlockWriteExExpired },
#if (FEATURE_AT_HSUART == FEATURE_ON)
    { AT_VOICE_RI_TIMER, AT_RcvTiVoiceRiExpired },
    { AT_SMS_RI_TIMER, AT_RcvTiSmsRiExpired },
#endif
#if (FEATURE_LTE == FEATURE_ON)
    { AT_SHUTDOWN_TIMER, AT_RcvTiShutDownExpired },
#endif
    { AT_AUTH_PUBKEY_TIMER, AT_RcvTiAuthPubkeyExpired },
};

static const AT_PS_CmdTimeoutResult g_atCmdTimeoutResultTab[] = {
    /* cmd Timer Name */ /* 消息处理函数 */
    { AT_CMD_COPS_SET_AUTOMATIC, AT_ProcNetWorkTimerOut },
    { AT_CMD_COPS_SET_MANUAL, AT_ProcNetWorkTimerOut },
    { AT_CMD_COPS_SET_DEREGISTER, AT_ProcNetWorkTimerOut },
    { AT_CMD_COPS_SET_MANUAL_AUTOMATIC_MANUAL, AT_ProcNetWorkTimerOut },
    { AT_CMD_CUSD_REQ, AT_ProcNetWorkTimerOut },
    { AT_CMD_PPP_ORG_SET, AT_ProcPppOrgTimerOut },
    { AT_CMD_D_IP_CALL_SET, AT_ProcAtdIpCallTimerOut },
    { AT_CMD_H_PS_SET, AT_ProcPsCallEndTimerOut },
    { AT_CMD_PS_DATA_CALL_END_SET, AT_ProcPsCallEndTimerOut },
#if (FEATURE_LTE == FEATURE_ON)
    { AT_CMD_SET_TMODE, AT_ProcTmodeTimerOut },
#endif
    { AT_CMD_COPS_ABORT_PLMN_LIST, AT_ProcAbortCmdTimerOut },
    { AT_CMD_NETSCAN_ABORT, AT_ProcAbortCmdTimerOut },
    { AT_CMD_CMGS_TEXT_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CMGS_PDU_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CMGC_TEXT_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CMGC_PDU_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CMSS_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CMST_SET, AT_ProcUnkonwnTimerOut },
    { AT_CMD_CPBR2_SET, AT_ProcCpbrTimerOut },
    { AT_CMD_CPBR_SET, AT_ProcCpbrTimerOut },
    { AT_CMD_VMSET_SET, AT_ProcVmsetTimerOut },
};

static const AT_MnOptTypeConvertAtCmdOptType g_atMnOptTypeConvertAtCmdTypeTab[] = {
    { TAF_MMI_CALL_ORIG, AT_CMD_D_CS_VOICE_CALL_SET },
    { TAF_MMI_CALL_CHLD_REQ, AT_CMD_CHLD_SET },
    { TAF_MMI_CHANGE_PIN, AT_CMD_CPIN_UNBLOCK_SET },
    { TAF_MMI_CHANGE_PIN2, AT_CMD_CPIN2_UNBLOCK_SET },
    { TAF_MMI_UNBLOCK_PIN, AT_CMD_CPIN_VERIFY_SET },
    { TAF_MMI_UNBLOCK_PIN2, AT_CMD_CPIN2_VERIFY_SET },
    { TAF_MMI_REGISTER_SS, AT_CMD_SS_REGISTER },
    { TAF_MMI_ERASE_SS, AT_CMD_SS_ERASE },
    { TAF_MMI_ACTIVATE_SS, AT_CMD_SS_ACTIVATE },
    { TAF_MMI_DEACTIVATE_SS, AT_CMD_SS_DEACTIVATE },
    { TAF_MMI_INTERROGATE_SS, AT_CMD_SS_INTERROGATE },
    { TAF_MMI_REGISTER_PASSWD, AT_CMD_SS_REGISTER_PSWD },
    { TAF_MMI_PROCESS_USSD_REQ, AT_CMD_CUSD_REQ },
    { TAF_MMI_SUPPRESS_CLIP, AT_CMD_CLIP_READ },
    { TAF_MMI_INVOKE_CLIP, AT_CMD_CLIP_READ },
    { TAF_MMI_DEACTIVATE_CCBS, AT_CMD_SS_DEACTIVE_CCBS },
    { TAF_MMI_INTERROGATE_CCBS, AT_CMD_SS_INTERROGATE_CCBS },
};

/*
 * 3 函数、变量声明
 */

VOS_UINT8 *AT_GetCrlf(VOS_VOID)
{
    return g_atCrLf;
}
TAF_UINT8 *AT_GetSendDataBuffer(VOS_VOID)
{
    return g_atSendDataBuffer;
}
AT_SEND_DataBuffer *AT_GetSendDataBuff(VOS_VOID)
{
    return &g_atSendDataBuff;
}
TAF_UINT8 *AT_GetSndCodeAddress(VOS_VOID)
{
    return g_atSndCodeAddress;
}

MN_OPERATION_ID_T At_GetOpId(VOS_VOID)
{
    g_opId++;
    g_opId %= MN_OP_ID_BUTT;
    if (g_opId == 0) {
        g_opId++;
    }
    return (g_opId);
}

VOS_VOID AT_StopTimerCmdReady(VOS_UINT8 indexNum)
{
    AT_StopRelTimer(indexNum, &g_atClientTab[indexNum].hTimer);
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_atClientTab[indexNum].opId          = 0;
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    if (g_current_user_at == indexNum) {
        g_atClientTab[AT_CLIENT_ID_APP].isWaitAts = 0;
        g_current_user_at = -1;
    }
#endif
}

TAF_UINT32 At_ClientIdToUserId(TAF_UINT16 clientId, TAF_UINT8 *indexId)
{
    TAF_UINT8 i;
    *indexId = 0;

    /* 判断是否为广播client id */
    /* MODEM 0的广播ClientId */
    if (clientId == AT_BROADCAST_CLIENT_ID_MODEM_0) {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
        return AT_SUCCESS;
    }

    /* MODEM 1的广播ClientId */
    if (clientId == AT_BROADCAST_CLIENT_ID_MODEM_1) {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_1;
        return AT_SUCCESS;
    }

    /* MODEM 2的广播ClientId */
    if (clientId == AT_BROADCAST_CLIENT_ID_MODEM_2) {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_2;
        return AT_SUCCESS;
    }

    /* 查找用户管理表 */
    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if (g_atClientTab[i].used == AT_CLIENT_NULL) {
            continue;
        }
        if (g_atClientTab[i].clientId == clientId) {
            *indexId = i; /* 返回结果 */
            return AT_SUCCESS;
        }
    }

    return AT_FAILURE;
}

TAF_UINT32 At_ClientIdToUserBroadCastId(TAF_UINT16 clientId, TAF_UINT8 *indexId)
{
    VOS_UINT32    rst;
    ModemIdUint16 modemId = MODEM_ID_0;

    rst = AT_GetModemIdFromClient(clientId, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("At_ClientIdToUserBroadCastId:Get ModemID From ClientID fail,ClientID=%d", clientId);
        return AT_FAILURE;
    }

    if (modemId == MODEM_ID_0) {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    }
#if (MULTI_MODEM_NUMBER > 1)
    else if (modemId == MODEM_ID_1) {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_1;
    }
#if (MULTI_MODEM_NUMBER > 2)
    else {
        *indexId = AT_BROADCAST_CLIENT_INDEX_MODEM_2;
    }
#endif
#endif

    return AT_SUCCESS;
}

DMS_PortIdUint16 AT_GetDmsPortIdByClientId(AT_ClientIdUint16 clientId)
{
    return (clientId >= AT_CLIENT_ID_BUTT) ? DMS_PORT_BUTT : g_atClientTab[clientId].portNo;
}

TAF_VOID At_SendReportMsg(TAF_UINT8 type, TAF_UINT8 *data, TAF_UINT16 len)
{
    MN_AT_IndEvt *event = VOS_NULL_PTR;
    VOS_UINT_PTR  tmpAddr;
    errno_t       memResult;

    /* 增加自定义的ITEM，共4个字节 */
    /*lint -save -e516 */
    event = (MN_AT_IndEvt *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, (len + AT_MN_INTERFACE_MSG_HEADER_LEN));
    /*lint -restore */
    if (event == TAF_NULL_PTR) {
        AT_ERR_LOG("At_SendCmdMsg:ERROR:Alloc Msg");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)event, AT_GetDestPid(((SI_PB_EventInfo *)data)->clientId, I0_WUEPS_PID_TAF), WUEPS_PID_AT,
                  (len + AT_MN_INTERFACE_MSG_HEADER_LEN));
    event->msgName = type;
    event->len     = len;

    tmpAddr = (VOS_UINT_PTR)event->content;
    if (len > 0) {
        memResult = memcpy_s((VOS_UINT8 *)tmpAddr, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    /* 发送消息到AT_PID; */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, event) != 0) {
        AT_ERR_LOG("At_SendReportMsg:ERROR");
        return;
    }
}

/*
 * Description: 扫描退格符
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_ScanDelChar(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT32 chkLen  = 0;
    TAF_UINT16 lenTemp = 0;
    TAF_UINT8 *write   = data;
    TAF_UINT8 *read    = data;

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 输入参数检查 */
    while (chkLen++ < *len) {
        if (g_atS5 == *read) { /* 删除上一字符 */
            if (lenTemp > 0) {
                write--;
                lenTemp--;
            }
        } else { /* 选择有效字符 */
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }
    *len = lenTemp;
    return AT_SUCCESS;
}

/*
 * Description: 扫描控制符
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_ScanCtlChar(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT8 *write   = data;
    TAF_UINT8 *read    = data;
    TAF_UINT32 chkLen  = 0;
    TAF_UINT16 lenTemp = 0;

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 检查 */
    while (chkLen++ < *len) {
        /* 选择有效字符 */
        if (*read >= 0x20) {
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }
    *len = lenTemp;
    return AT_SUCCESS;
}

TAF_UINT32 At_ScanBlankChar(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT8 *check     = data;
    TAF_UINT8 *write     = data;
    TAF_UINT8 *read      = data;
    TAF_UINT32 chkQout   = 0;
    TAF_UINT32 chkLen    = 0;
    TAF_UINT32 qoutNum   = 0;
    TAF_UINT16 qoutCount = 0;
    TAF_UINT16 lenTemp   = 0;

    if (*len == 0) {
        return AT_FAILURE;
    }

    /* 检查引号 */
    while (qoutNum++ < *len) {
        if (*check++ == '"') {
            chkQout++;
        }
    }
    /* 其值只能为偶数，奇数返回错误 */
    if ((chkQout % 2) == 1) {
        return AT_FAILURE;
    }

    /* 检查 */
    while (chkLen++ < *len) {
        /* 当前是第几个引号状态 */
        if (*read == '\"') {
            qoutCount++;
        }

        /* 去掉引号外的空格,除2是为了判断奇偶性 */
        if (((*read != AT_BLANK_ASCII_VALUE) && ((qoutCount % 2) == 0)) || ((qoutCount % 2) == 1)) {
            *write++ = *read;
            lenTemp++;
        }
        read++;
    }
    *len = lenTemp;
    return AT_SUCCESS;
}

VOS_UINT32 AT_ConvertCharToHex(VOS_UINT8 charValue, VOS_UINT8 *hexValue)
{
    if ((charValue >= '0') && (charValue <= '9')) {
        *hexValue = charValue - '0';
    } else if ((charValue >= 'a') && (charValue <= 'f')) {
        /* 转换'a'到'f'为数字 */
        *hexValue = (charValue - 'a') + AT_DECIMAL_BASE_NUM; /* 将字符串转换成十进制数 */
    } else if ((charValue >= 'A') && (charValue <= 'F')) {
        /* 转换'a'到'f'为数字 */
        *hexValue = (charValue - 'A') + AT_DECIMAL_BASE_NUM; /* 将字符串转换成十进制数 */
    } else {
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_ParseSimLockPara(VOS_UINT8 *data, VOS_UINT16 len, AT_ParseParaType *paralist, VOS_UINT32 paraCnt)
{
    VOS_UINT8 *paraStart = VOS_NULL_PTR;
    VOS_UINT32 paraIdx;
    VOS_UINT32 chkRslt;
    VOS_UINT32 i;

    if (paralist == VOS_NULL_PTR) {
        AT_ERR_LOG("At_PareseSimLockPara: pstParalist = VOS_NULL_PTR!");
        return AT_FAILURE;
    }

    paraStart = data;
    paraIdx   = 0;

    /* 从AT^simlock=pucData的pucData开始解析  */
    for (i = 0; i < len; i++) {
        if (data[i] == ',') {
            /* 当前参数的长度 */
            chkRslt = At_CheckAndParsePara((paralist + paraIdx), paraStart, (data + i));
            if (chkRslt != AT_OK) {
                return AT_FAILURE;
            }

            paraStart = (data + i) + 1;
            ++paraIdx;

            if (paraIdx >= paraCnt) {
                return AT_FAILURE;
            }
        }
    }

    /*  将最后一段参数修改copy到参数列表中 */
    chkRslt = At_CheckAndParsePara((paralist + paraIdx), paraStart, (data + len));
    if (chkRslt != AT_OK) {
        return AT_FAILURE;
    }

    ++paraIdx;

    if (paraIdx > paraCnt) {
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_DispatchSimlockCmd(VOS_UINT8 indexNum, VOS_UINT32 paraCnt, AT_ParseParaType *paralist)
{
    VOS_UINT8 simLockOP;

    if ((paraCnt < 1) || (paralist == VOS_NULL_PTR)) {
        At_FormatResultData(indexNum, AT_ERROR);
        return AT_FAILURE;
    }

    if ((paralist[0].paraLen != 1) || (paralist[0].para[0] < '0') || (paralist[0].para[0] > '2')) {
        /* 输出错误 */
        At_FormatResultData(indexNum, AT_ERROR);
        return AT_FAILURE;
    }

    simLockOP = paralist[0].para[0] - '0';

    if (simLockOP == AT_SIMLOCK_OPRT_UNLOCK) {
        At_UnlockSimLock(indexNum, paraCnt, paralist);
    } else if (simLockOP == AT_SIMLOCK_OPRT_SET_PLMN_INFO) {
        At_SetSimLockPlmnInfo(indexNum, paraCnt, paralist);
    } else {
        AT_GetSimLockStatus(indexNum);
    }
    return AT_SUCCESS;
}

/*
 * 功能描述: 检查字符串长度以及头部内容
 */
LOCAL VOS_UINT32 At_CheckSimLockPara(VOS_UINT8 *dataPara, const VOS_UINT8 *data, VOS_UINT16 len, VOS_UINT16 *cmdLen)
{
    VOS_INT8 ret;
    errno_t memResult;

    memResult = memcpy_s(dataPara, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    /* 待处理的字符串长度小于等于"AT^SIMLOCK"长度直接返回AT_FAILURE */
    (*cmdLen) = (VOS_UINT16)VOS_StrLen("AT^SIMLOCK=");

    if (len <= (*cmdLen)) {
        (VOS_VOID)memset_s(dataPara, len, 0x00, len);
        return AT_FAILURE;
    }

    /* 待处理的字符串头部不是"AT^SIMLOCK"直接返回AT_FAILURE */
    ret = VOS_StrNiCmp((VOS_CHAR *)dataPara, "AT^SIMLOCK=", (*cmdLen));

    if (ret != 0) {
        (VOS_VOID)memset_s(dataPara, len, 0x00, len);
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_ProcSimLockPara(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16        cmdLen = 0;
    VOS_UINT8        *dataPara = TAF_NULL_PTR;
    VOS_UINT32        paraCnt;
    AT_ParseParaType *paralist = VOS_NULL_PTR;

    if (len == 0) {
        return AT_FAILURE;
    }
    /*lint -save -e516 */
    dataPara = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, len);
    /*lint -restore */
    if (dataPara == VOS_NULL_PTR) {
        AT_ERR_LOG("At_ProcSimLockPara: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(dataPara, len, 0x00, len);

    /* 检查字符串长度以及头部内容 */
    if (At_CheckSimLockPara(dataPara, data, len, &cmdLen) == AT_FAILURE) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(indexNum, (VOS_UINT8 *)"^SIMLOCK", AT_EXTEND_CMD_TYPE);

    /* 检测参数个数 */
    paraCnt = At_GetParaCnt(dataPara, len);

    if (paraCnt < 1) {
        (VOS_VOID)memset_s(dataPara, len, 0x00, len);
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        At_FormatResultData(indexNum, AT_ERROR);
        return AT_SUCCESS;
    }

    /* 如果参数是超过61个返回失败 */
    if (paraCnt > AT_SIMLOCK_PLMN_MAX_NUM) {
        (VOS_VOID)memset_s(dataPara, len, 0x00, len);
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        At_FormatResultData(indexNum, AT_SIMLOCK_PLMN_NUM_ERR);
        return AT_SUCCESS;
    }

    paralist = (AT_ParseParaType *)PS_MEM_ALLOC(WUEPS_PID_AT, (paraCnt * sizeof(AT_ParseParaType)));

    if (paralist == VOS_NULL_PTR) {
        AT_ERR_LOG("At_ProcSimLockPara: pstParalist Memory malloc failed!");
        (VOS_VOID)memset_s(dataPara, len, 0x00, len);
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        At_FormatResultData(indexNum, AT_ERROR);
        return AT_SUCCESS;
    } else {
        (VOS_VOID)memset_s(paralist, paraCnt * sizeof(AT_ParseParaType), 0x00,
            (VOS_SIZE_T)(paraCnt * sizeof(AT_ParseParaType)));
    }

    /* 将 At^simlock的参数解析到 At格式的参数列表中 */
    if (At_ParseSimLockPara((dataPara + cmdLen), (len - cmdLen), paralist, paraCnt) == AT_SUCCESS) {
        /* 根据at^simlock=oprt,paralist中的oprt分发 Simlock的命令处理 */
        At_DispatchSimlockCmd(indexNum, paraCnt, paralist);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    /*lint -save -e516 */
    (VOS_VOID)memset_s(paralist, paraCnt * sizeof(AT_ParseParaType), 0x00, (VOS_SIZE_T)(paraCnt * sizeof(AT_ParseParaType)));
    (VOS_VOID)memset_s(dataPara, len, 0x00, len);
    PS_MEM_FREE(WUEPS_PID_AT, paralist);
    PS_MEM_FREE(WUEPS_PID_AT, dataPara);
    /*lint -restore */

    return AT_SUCCESS;
}

/*
 * 功能描述: 初始化参数
 */
LOCAL VOS_UINT32 AT_HandleFacAuthPubKeyExCmdInit(VOS_UINT8 indexNum, VOS_UINT16 *cmdlen,
    AT_ParseCmdNameType *atCmdName)
{
    VOS_UINT32 tempIndex;
    VOS_UINT32 timerName;

    (*cmdlen) = (VOS_UINT16)VOS_StrLen("AT^FACAUTHPUBKEYEX=");
    tempIndex = (VOS_UINT32)indexNum;
    timerName = AT_AUTH_PUBKEY_TIMER;
    timerName |= AT_INTERNAL_PROCESS_TYPE;
    timerName |= (tempIndex << 12);
    (VOS_VOID)memset_s(atCmdName, sizeof(AT_ParseCmdNameType), 0x00, sizeof(AT_ParseCmdNameType));

    return timerName;
}

/*
 * 功能描述: 检查该命令是否为AT^FACAUTHPUBKEYEX
 */
LOCAL VOS_INT8 AT_CheckIsFacAuthPubKeyExCmd(VOS_UINT8 *dataPara, VOS_UINT16 cmdlen, VOS_UINT8 *data)
{
    errno_t memResult;
    VOS_INT8 ret;

    (VOS_VOID)memset_s(dataPara, cmdlen, 0x00, cmdlen);

    /* 拷贝命令名，供后续比较使用 */
    memResult = memcpy_s(dataPara, cmdlen, data, cmdlen);
    TAF_MEM_CHK_RTN_VAL(memResult, cmdlen, cmdlen);

    /* AT命令头字符转大写 */
    At_UpString(dataPara, cmdlen);

    /* 待处理的字符串头部不是"AT^FACAUTHPUBKEYEX="直接返回AT_FAILURE */
    ret = VOS_StrNiCmp((VOS_CHAR *)dataPara, "AT^FACAUTHPUBKEYEX=", cmdlen);

    return ret;
}

/*
 * 功能描述: 获取命令(不包含命令前缀AT)名称及长度
 */
LOCAL VOS_UINT16 AT_GetFacAuthPubKeyCmdParametersLen(VOS_UINT8 indexNum, VOS_UINT8 *data)
{
    AT_ParseCmdNameType atCmdName;
    VOS_UINT16 pos;
    errno_t memResult;

    (VOS_VOID)memset_s(&atCmdName, sizeof(atCmdName), 0x00, sizeof(atCmdName));

    AT_SaveCmdElementInfo(indexNum, (VOS_UINT8 *)"^FACAUTHPUBKEYEX", AT_EXTEND_CMD_TYPE);

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos = (VOS_UINT16)VOS_StrLen("AT");

    atCmdName.cmdNameLen = (VOS_UINT16)VOS_StrLen("^FACAUTHPUBKEYEX");
    memResult            = memcpy_s(atCmdName.cmdName, sizeof(atCmdName.cmdName), (data + pos), atCmdName.cmdNameLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmdName.cmdName), atCmdName.cmdNameLen);
    atCmdName.cmdName[atCmdName.cmdNameLen] = '\0';
    pos += atCmdName.cmdNameLen;

    pos += (VOS_UINT16)VOS_StrLen("=");

    return pos;
}

/*
 * 功能描述: FacAuthPubKey参数不正确的处理
 */
LOCAL VOS_VOID AT_ProcFacAuthPubKeyIncorrectPara(VOS_UINT8 indexNum, VOS_UINT32 timerName)
{
    AT_AUTH_PUBKEYEX_CMD_PROC_CTX *authPubKeyCtx = VOS_NULL_PTR;

    authPubKeyCtx = AT_GetAuthPubkeyExCmdCtxAddr();

    At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);
    AT_ClearAuthPubkeyCtx();
    (VOS_VOID)AT_StopRelTimer(timerName, &(authPubKeyCtx->hAuthPubkeyProtectTimer));
}

/*
 * 功能描述: 获取命令中的逗号位置和个数
 */
LOCAL VOS_UINT16 AT_GetFacAuthPubKeyCmdCommaNumAndPosition(VOS_UINT16 pos, VOS_UINT16 len, VOS_UINT8 *data,
    VOS_UINT16 *firstCommaPos, VOS_UINT16 *secCommaPos)
{
    VOS_UINT16 loop;
    VOS_UINT16 commaCnt = 0;

    /* 获取命令中的逗号位置和个数 */
    for (loop = pos; loop < len; loop++) {
        if (*(data + loop) != ',') {
            continue;
        }

        commaCnt++;
        /* 记录下第一个逗号的位置 */
        if ((*firstCommaPos) == 0) {
            (*firstCommaPos) = loop + 1;
        } else {
            if ((*secCommaPos) == 0) {
                (*secCommaPos) = loop + 1;
            }
        }
    }

    return commaCnt;
}

VOS_UINT32 AT_HandleFacAuthPubKeyExCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    AT_ParseCmdNameType            atCmdName;
    VOS_UINT8                     *dataPara = VOS_NULL_PTR;
    VOS_UINT32                     result;
    VOS_UINT32                     firstParaVal = 0;
    VOS_UINT32                     secParaVal = 0;
    VOS_UINT32                     timerName;
    VOS_UINT16                     cmdlen;
    VOS_UINT16                     pos = 0;
    VOS_UINT16                     commaCnt = 0;
    VOS_UINT16                     firstCommaPos = 0;
    VOS_UINT16                     secCommaPos = 0;
    VOS_UINT16                     firstParaLen = 0;
    VOS_UINT16                     secondParaLen = 0;
    VOS_UINT16                     thirdParaLen;

    timerName = AT_HandleFacAuthPubKeyExCmdInit(indexNum, &cmdlen, &atCmdName);

    if (len < cmdlen) {
        return AT_FAILURE;
    }

    /*lint -save -e516 */
    dataPara = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, cmdlen);
    /*lint -restore */
    if (dataPara == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_HandleFacAuthPubKeyExCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    if (AT_CheckIsFacAuthPubKeyExCmd(dataPara, cmdlen, data) != 0) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        return AT_FAILURE;
    }

    /* 获取命令中参数开始的位置 */
    pos = AT_GetFacAuthPubKeyCmdParametersLen(indexNum, data);

    /* 获取命令中的逗号位置和个数 */
    commaCnt = AT_GetFacAuthPubKeyCmdCommaNumAndPosition(pos, len, data, &firstCommaPos, &secCommaPos);

    /* 若逗号个数不为2，则AT命令结果返回失败 */
    if (commaCnt != AT_VALID_COMMA_NUM) {
        AT_WARN_LOG("AT_HandleFacAuthPubKeyExCmd: usCommaCnt != 2!");
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        AT_ProcFacAuthPubKeyIncorrectPara(indexNum, timerName);
        return AT_SUCCESS;
    }

    /* 计算参数的长度 */
    firstParaLen  = (firstCommaPos - pos) - (VOS_UINT16)VOS_StrLen(",");
    secondParaLen = secCommaPos - firstCommaPos - (VOS_UINT16)VOS_StrLen(",");
    thirdParaLen  = len - secCommaPos;

    /* 获取第一个参数值 */
    if (atAuc2ul(data + pos, firstParaLen, &firstParaVal) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleFacAuthPubKeyExCmd: ulFirstParaVal value invalid");
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        AT_ProcFacAuthPubKeyIncorrectPara(indexNum, timerName);
        return AT_SUCCESS;
    }

    /* 获取第二个参数值 */
    if (atAuc2ul(data + firstCommaPos, secondParaLen, &secParaVal) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleFacAuthPubKeyExCmd: ulSecParaVal value invalid");
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        AT_ProcFacAuthPubKeyIncorrectPara(indexNum, timerName);
        return AT_SUCCESS;
    }

    result = AT_SetFacAuthPubkeyExPara(indexNum, firstParaVal, secParaVal, thirdParaLen, (data + secCommaPos));
    if (result != AT_WAIT_ASYNC_RETURN) {
        At_FormatResultData(indexNum, result);
    }

    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, dataPara);
    /*lint -restore */
    return AT_SUCCESS;
}

/*
 * 功能描述: 获取命令(不包含命令前缀AT)名称及长度
 */
LOCAL VOS_UINT16 AT_GetAtCmdParametersLen(const VOS_UINT8 *data, const VOS_CHAR *atCmdStr,
    AT_ParseCmdNameType *cmdName)
{
    errno_t memResult;
    VOS_UINT16 pos;

    pos = (VOS_UINT16)VOS_StrLen("AT");

    cmdName->cmdNameLen = (VOS_UINT16)VOS_StrLen(atCmdStr);

    memResult = memcpy_s(cmdName->cmdName, AT_CMD_NAME_LEN + 1, (data + pos), cmdName->cmdNameLen);
    TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_NAME_LEN + 1, cmdName->cmdNameLen);

    cmdName->cmdName[cmdName->cmdNameLen] = '\0';

    pos += cmdName->cmdNameLen;

    pos += (VOS_UINT16)VOS_StrLen("=");

    return pos;
}

VOS_UINT32 AT_HandleSimLockDataWriteCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    errno_t                     memResult;
    VOS_INT8                    ret;
    VOS_UINT16                  cmdlen;
    VOS_UINT16                  pos;
    VOS_UINT8                  *dataPara         = VOS_NULL_PTR;
    AT_SIMLOCKDATAWRITE_SetReq *simlockDataWrite = VOS_NULL_PTR;
    VOS_UINT32                  result;
    AT_ParseCmdNameType         atCmdName;
    VOS_CHAR                   *atCmdStr = "^SIMLOCKDATAWRITE";

    (VOS_VOID)memset_s(&atCmdName, sizeof(atCmdName), 0x00, sizeof(atCmdName));

    /* 为提高AT解析性能，在入口处判断命令长度是否为AT^SIMLOCKDATAWRITE设置命令的长度，若不是则直接退出 */
    if ((VOS_StrLen("AT^SIMLOCKDATAWRITE=") + AT_SIMLOCKDATAWRITE_PARA_LEN) != len) {
        return AT_FAILURE;
    }

    cmdlen = (VOS_UINT16)VOS_StrLen("AT^SIMLOCKDATAWRITE=");
    /*lint -save -e516 */
    dataPara = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, cmdlen);
    /*lint -restore */
    if (dataPara == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_HandleSimLockDataWriteCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }
    (VOS_VOID)memset_s(dataPara, cmdlen, 0x00, cmdlen);

    /* 拷贝命令名，供后续比较使用 */
    memResult = memcpy_s(dataPara, cmdlen, data, cmdlen);
    TAF_MEM_CHK_RTN_VAL(memResult, cmdlen, cmdlen);

    /* AT命令头字符转大写 */
    At_UpString(dataPara, cmdlen);

    /* 待处理的字符串头部不是"AT^SIMLOCKDATAWRITE="直接返回AT_FAILURE */
    ret = VOS_StrNiCmp((VOS_CHAR *)dataPara, "AT^SIMLOCKDATAWRITE=", cmdlen);
    if (ret != 0) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(indexNum, (VOS_UINT8 *)"^SIMLOCKDATAWRITE", AT_EXTEND_CMD_TYPE);

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos = AT_GetAtCmdParametersLen(data, atCmdStr, &atCmdName);

    /* 申请参数解析缓存结构 */
    simlockDataWrite = (AT_SIMLOCKDATAWRITE_SetReq *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(AT_SIMLOCKDATAWRITE_SetReq));
    if (simlockDataWrite == VOS_NULL_PTR) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dataPara);
        /*lint -restore */
        At_FormatResultData(indexNum, AT_ERROR);
        return AT_SUCCESS;
    }
    (VOS_VOID)memset_s(simlockDataWrite, sizeof(AT_SIMLOCKDATAWRITE_SetReq), 0x00, sizeof(AT_SIMLOCKDATAWRITE_SetReq));

    /* 保存参数 */
    memResult = memcpy_s(simlockDataWrite->categoryData, AT_SIMLOCKDATAWRITE_PARA_LEN, (data + pos),
                         (VOS_SIZE_T)(len - pos));
    TAF_MEM_CHK_RTN_VAL(memResult, AT_SIMLOCKDATAWRITE_PARA_LEN, (VOS_SIZE_T)(len - pos));

    /* 设置命令类型，操作类型和参数个数 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    g_atCmdFmtType          = AT_EXTEND_CMD_TYPE;

    AT_PR_LOGI("enter");

    result = AT_SetSimlockDataWritePara(indexNum, simlockDataWrite);
    if (result != AT_WAIT_ASYNC_RETURN) {
        AT_PR_LOGI("return OK");

        At_FormatResultData(indexNum, result);
    }
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, dataPara);
    PS_MEM_FREE(WUEPS_PID_AT, simlockDataWrite);
    /*lint -restore */
    return AT_SUCCESS;
}

/*
 * 功能描述: 获取并检查命令中的逗号位置和个数
 */
LOCAL VOS_UINT32 AT_GetAtCmdCommaNumAndPosition(VOS_UINT8 indexNum, VOS_UINT16 pos, VOS_UINT16 len,
    const VOS_UINT8 *data, VOS_UINT16 *firstCommaPos)
{
    VOS_UINT16 loop;
    VOS_UINT16 commaCnt = 0;

    for (loop = pos; loop < len; loop++) {
        if (*(data + loop) == ',') {
            commaCnt++;

            /* 记录下第一个逗号的位置 */
            if ((*firstCommaPos) == 0) {
                (*firstCommaPos) = loop + 1;
            }
        }
    }

    /* 若逗号个数不为1，则AT命令结果返回失败 */
    if (commaCnt != 1) {
        AT_WARN_LOG("AT_GetAtCmdCommaNumAndPosition: usCommaCnt != 1!");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    return AT_FAILURE;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 AT_HandleApSndApduCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_CHAR *atCmdStr = "^CISA";
    VOS_UINT16 cmdlen;
    VOS_UINT16 pos;
    VOS_UINT16 firstCommaPos = 0;
    VOS_UINT16 firstParaLen;
    VOS_UINT16 secondParaLen;
    VOS_UINT32 lengthValue;
    VOS_UINT32 result;
    VOS_UINT8  headChar;
    VOS_UINT8  tailChar;

    /* CISA命令只能从AP侧接收处理，其它端口不可以 */
    if (AT_IsApPort(indexNum) != VOS_TRUE) {
        return AT_FAILURE;
    }

    cmdlen = (VOS_UINT16)VOS_StrLen("AT^CISA=");

    /* 待处理的字符串头部不是"AT^CISA="直接返回AT_FAILURE */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CISA=", cmdlen) != 0) {
        return AT_FAILURE;
    }

    /* 不支持AT^CISA=?命令 */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CISA=?", VOS_StrLen("AT^CISA=?")) == 0) {
        At_FormatResultData(indexNum, AT_ERROR);

        return AT_SUCCESS;
    }

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos = AT_GetAtCmdParametersLen(data, atCmdStr, &(g_atParseCmd.cmdName));

    /* 获取并检查命令中的逗号位置和个数 */
    if (AT_GetAtCmdCommaNumAndPosition(indexNum, pos, len, data, &firstCommaPos) == AT_SUCCESS) {
        AT_WARN_LOG("AT_HandleApSndApduCmd: usCommaCnt != 1!");
        return AT_SUCCESS;
    }

    /* 第二个参数是由引号包含起来的 */
    headChar = *(data + firstCommaPos);
    tailChar = *(data + len - VOS_StrLen("\""));
    if ((headChar != '"') || (tailChar != '"')) {
        AT_WARN_LOG("AT_HandleApSndApduCmd: <command> not a string");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    /* 计算两个参数的长度 */
    firstParaLen  = (firstCommaPos - pos) - (VOS_UINT16)VOS_StrLen(",");
    secondParaLen = len - firstCommaPos - (VOS_UINT16)(AT_DOUBLE_LENGTH * VOS_StrLen("\""));

    if (atAuc2ul(data + cmdlen, firstParaLen, &lengthValue) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleApSndApduCmd: <length> value invalid");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }
    /* 设置命令类型，操作类型和参数个数 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    g_atCmdFmtType          = AT_EXTEND_CMD_TYPE;

    /* 在扩展命令表中匹配此命令 */
    result = atMatchCmdName(indexNum, g_atParseCmd.cmdFmtType);

    if (result != ERR_MSP_SUCCESS) {
        At_FormatResultData(indexNum, result);

        return AT_SUCCESS;
    }

    result = AT_SetCISAPara(indexNum, lengthValue, (data + firstCommaPos + VOS_StrLen("\"")), secondParaLen);

    if (result != AT_WAIT_ASYNC_RETURN) {
        At_FormatResultData(indexNum, result);
    }

    return AT_SUCCESS;
}
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_HandleApXsmsSndCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16 cmdlen;
    VOS_UINT16 pos;
    VOS_UINT16 firstCommaPos = 0;
    VOS_UINT16 firstParaLen;
    VOS_UINT16 secondParaLen;
    VOS_UINT32 lengthValue;
    VOS_UINT32 result;
    VOS_UINT8  headChar;
    VOS_UINT8  tailChar;
    VOS_CHAR *atCmdStr = "^CCMGS";

    cmdlen = (VOS_UINT16)VOS_StrLen("AT^CCMGS=");

    /* 待处理的字符串头部不是"AT^CCMGS="直接返回AT_FAILURE */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CCMGS=", cmdlen) != 0) {
        return AT_FAILURE;
    }

    /* 不支持AT^CCMGS=?命令 */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CCMGS=?", VOS_StrLen("AT^CCMGS=?")) == 0) {
        At_FormatResultData(indexNum, AT_OK);

        return AT_SUCCESS;
    }

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos = AT_GetAtCmdParametersLen(data, atCmdStr, &(g_atParseCmd.cmdName));

    /* 获取并检查命令中的逗号位置和个数 */
    if (AT_GetAtCmdCommaNumAndPosition(indexNum, pos, len, data, &firstCommaPos) == AT_SUCCESS) {
        AT_WARN_LOG("AT_HandleApXsmsSndCmd: usCommaCnt != 1!");
        return AT_SUCCESS;
    }

    /* 第二个参数是由引号包含起来的 */
    headChar = *(data + firstCommaPos);
    tailChar = *(data + len - VOS_StrLen("\""));
    if ((headChar != '"') || (tailChar != '"')) {
        AT_WARN_LOG("AT_HandleApXsmsSndCmd: <PDU> not a string");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    /* 计算两个参数的长度 */
    firstParaLen  = (firstCommaPos - pos) - (VOS_UINT16)VOS_StrLen(",");
    secondParaLen = len - firstCommaPos - (VOS_UINT16)(AT_DOUBLE_LENGTH * VOS_StrLen("\""));

    if (atAuc2ul(data + cmdlen, firstParaLen, &lengthValue) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleApXsmsSndCmd: <length> value invalid");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }
    /* 设置命令类型，操作类型和参数个数 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    g_atCmdFmtType          = AT_EXTEND_CMD_TYPE;

    /* 在扩展命令表中匹配此命令 */
    result = atMatchCmdName(indexNum, g_atParseCmd.cmdFmtType);

    if (result != ERR_MSP_SUCCESS) {
        At_FormatResultData(indexNum, result);

        return AT_SUCCESS;
    }

    result = AT_SetCcmgsPara(indexNum, lengthValue, (data + firstCommaPos + VOS_StrLen("\"")), secondParaLen);

    if (result != AT_WAIT_ASYNC_RETURN) {
        At_FormatResultData(indexNum, result);
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 获取逗号个数和位置
 */
LOCAL VOS_UINT16 AT_GetApXsmsWriteCmdCommaNumAndPosition(VOS_UINT16 pos, VOS_UINT16 len, VOS_UINT8 *data, VOS_UINT16 *firstCommaPos, VOS_UINT16 *secondCommaPos)
{
    VOS_UINT16 commaCnt = 0;
    VOS_UINT16 loop;

    for (loop = pos; loop < len; loop++) {
        if (*(data + loop) == ',') {
            commaCnt++;

            /* 记录下第一个逗号的位置 */
            if ((*firstCommaPos) == 0) {
                (*firstCommaPos) = loop + 1;
            }

            /* 记录下第二个逗号的位置 */
            if (((*secondCommaPos) == 0) && (commaCnt == AT_VALID_COMMA_NUM)) {
                (*secondCommaPos) = loop + 1;
            }
        }
    }

    return commaCnt;
}

/*
 * 功能描述: 检查第二个和第三个参数
 */
LOCAL VOS_UINT32 AT_CheckApXsmsWriteCmdPara(VOS_UINT8 indexNum, VOS_UINT16 commaCnt, VOS_UINT16 len, VOS_UINT16 secondCommaPos, VOS_UINT8 *data)
{
    VOS_UINT8  headChar;
    VOS_UINT8  tailChar;

    /* 若逗号个数不为2，则AT命令结果返回失败 */
    if (commaCnt != AT_VALID_COMMA_NUM) {
        AT_WARN_LOG("AT_CheckApXsmsWriteCmdPara: usCommaCnt != 1!");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    /* 第三个参数是由引号包含起来的 */
    headChar = *(data + secondCommaPos);
    tailChar = *(data + len - VOS_StrLen("\""));
    if ((headChar != '"') || (tailChar != '"')) {
        AT_WARN_LOG("AT_CheckApXsmsWriteCmdPara: <PDU> not a string");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    return AT_FAILURE;
}

VOS_UINT32 AT_HandleApXsmsWriteCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16 cmdlen;
    VOS_UINT16 pos;
    VOS_UINT16 commaCnt = 0;
    VOS_UINT16 firstCommaPos = 0;
    VOS_UINT16 secondCommaPos = 0;
    VOS_UINT16 firstParaLen;
    VOS_UINT16 secondParaLen;
    VOS_UINT16 thirdParaLen;
    VOS_UINT32 lengthValue;
    VOS_UINT32 statValue;
    VOS_UINT32 result;
    VOS_CHAR *atCmdStr = "^CCMGW";

    cmdlen = (VOS_UINT16)VOS_StrLen("AT^CCMGW=");

    /* 待处理的字符串头部不是"AT^CCMGW="直接返回AT_FAILURE */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CCMGW=", cmdlen) != 0) {
        return AT_FAILURE;
    }

    /* 不支持AT^CCMGS=?命令 */
    if (VOS_StrNiCmp((VOS_CHAR *)data, "AT^CCMGW=?", VOS_StrLen("AT^CCMGW=?")) == 0) {
        At_FormatResultData(indexNum, AT_OK);

        return AT_SUCCESS;
    }

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    pos = AT_GetAtCmdParametersLen(data, atCmdStr, &(g_atParseCmd.cmdName));

    /* 获取命令中的逗号位置和个数 */
    commaCnt = AT_GetApXsmsWriteCmdCommaNumAndPosition(pos, len, data, &firstCommaPos, &secondCommaPos);

    if (AT_CheckApXsmsWriteCmdPara(indexNum, commaCnt, len, secondCommaPos, data) == AT_SUCCESS) {
        return AT_SUCCESS;
    }
    /* 计算三个参数的长度 */
    firstParaLen  = (firstCommaPos - pos) - (VOS_UINT16)VOS_StrLen(",");
    secondParaLen = secondCommaPos - firstCommaPos - (VOS_UINT16)VOS_StrLen(",");
    thirdParaLen  = len - secondCommaPos - (VOS_UINT16)(AT_DOUBLE_LENGTH * VOS_StrLen("\""));

    /* 解析参数1<len>的值 */
    if (atAuc2ul(data + cmdlen, firstParaLen, &lengthValue) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleApXsmsSndCmd: <length> value invalid");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    /* 解析参数2<stat>的值 */
    if (atAuc2ul(data + firstCommaPos, secondParaLen, &statValue) == AT_FAILURE) {
        AT_WARN_LOG("AT_HandleApXsmsSndCmd: <length> value invalid");

        At_FormatResultData(indexNum, AT_CME_INCORRECT_PARAMETERS);

        return AT_SUCCESS;
    }

    /* 设置命令类型，操作类型和参数个数 */
    g_atParseCmd.cmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    g_atCmdFmtType          = AT_EXTEND_CMD_TYPE;

    /* 在扩展命令表中匹配此命令 */
    result = atMatchCmdName(indexNum, g_atParseCmd.cmdFmtType);

    if (result != ERR_MSP_SUCCESS) {
        At_FormatResultData(indexNum, result);

        return AT_SUCCESS;
    }

    result = AT_SetCcmgwPara(indexNum, lengthValue, statValue, (data + secondCommaPos + VOS_StrLen("\"")),
                             thirdParaLen);

    if (result != AT_WAIT_ASYNC_RETURN) {
        At_FormatResultData(indexNum, result);
    }

    return AT_SUCCESS;
}
#endif

VOS_UINT32 At_HandleApModemSpecialCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    /* 获取当前产品形态 */
    systemAppConfig = AT_GetSystemAppConfigAddr();

    if ((*systemAppConfig != SYSTEM_APP_ANDROID)
#if (FEATURE_PHONE_SC == FEATURE_ON)
        && (*systemAppConfig != SYSTEM_APP_MP)
#endif
        ) {
        return AT_FAILURE;
    }

    /* 处理AT^FACAUTHPUBKEYEX=<index>,<total>,<pubkey>设置命令(参数<pubkey>超长) */
    if (AT_HandleFacAuthPubKeyExCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    /* 处理AT^IDENTIFYEND=<Rsa>设置命令(参数<Rsa>超长) */
    if (AT_HandleApModemSpecialCmdProc(indexNum, data, len, "^IDENTIFYEND") == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    /* 处理AT^IDENTIFYOTAEND=<OtaRsa>设置命令(参数<OtaRsa>超长) */
    if (AT_HandleApModemSpecialCmdProc(indexNum, data, len, "^IDENTIFYOTAEND") == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    /* 处理AT^PHONEPHYNUM=<type>,<PhynumRsa>,<Hmac>设置命令(参数<PhynumRsa>超长) */
    if (AT_HandleApModemSpecialCmdProc(indexNum, data, len, "^PHONEPHYNUM") == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    /* 处理AT^SIMLOCKDATAWRITEEX=<layer>,<total>,<index>,<simlock_data>,[hmac]设置命令(参数<simlock_data>超长) */
    if (AT_HandleSimLockDataWriteExCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    /* 处理AT^SIMLOCKOTADATAWRITE=<layer>,<total>,<index>,<simlock_data>,[hmac]设置命令(参数<simlock_data>超长) */
    if (AT_HandleSimLockOtaDataWriteCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }

#if (FEATURE_SC_NETWORK_UPDATE == FEATURE_ON)
    /* 处理AT^SIMLOCKNWDATAWRITE=<layer>,<total>,<index>,<simlock_data>,[hmac]设置命令(参数<simlock_data>超长) */
    if (AT_HandleSimLockNWDataWriteCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }
#endif

    /* 处理AT^SIMLOCKDATAWRITE=<simlock_data_write>设置命令(参数<simlock_data_write>超长) */
    if (AT_HandleSimLockDataWriteCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* 处理AT^CISA=<length>,<command> */
    if (AT_HandleApSndApduCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (AT_HandleApXsmsSndCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }

    if (AT_HandleApXsmsWriteCmd(indexNum, data, len) == AT_SUCCESS) {
        return AT_SUCCESS;
    }
#endif

    /* 删除对^EOPLMN设置命令的特殊解析 */

    return AT_FAILURE;
}

TAF_UINT32 At_CheckUsimStatus(TAF_UINT8 *cmdName, VOS_UINT8 indexNum)
{
    TAF_UINT32       rst;
    ModemIdUint16    modemId;
    VOS_UINT32       getModemIdRslt;
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;
    AT_ModemSmsCtx  *smsCtx      = VOS_NULL_PTR;

    NAS_NVIM_FollowonOpenspeedFlag quickStartFlg;

    quickStartFlg.quickStartSta = AT_QUICK_START_DISABLE;
    modemId                     = MODEM_ID_0;

    getModemIdRslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (getModemIdRslt != VOS_OK) {
        AT_ERR_LOG("At_CheckUsimStatus:Get Modem Id fail!");
        return AT_ERROR;
    }

    smsCtx      = AT_GetModemSmsCtxAddrFromModemId(modemId);
    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);


    /* 快速开机模式，不检查 PIN 状态 */
    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_FOLLOW_ON_OPEN_SPEED_FLAG, (VOS_UINT8 *)(&quickStartFlg),
                          sizeof(NAS_NVIM_FollowonOpenspeedFlag)) != NV_OK) {
        quickStartFlg.quickStartSta = AT_QUICK_START_DISABLE;
    }

    if (quickStartFlg.quickStartSta == AT_QUICK_START_ENABLE) {
        return AT_SUCCESS;
    }

    /* SMS相关命令掉卡流程需要在业务模块内部处理，此处仅处理CMGL，其他命令的处理待确认 */
    if (VOS_StrCmp((TAF_CHAR *)cmdName, "+CMGL") == 0) {
        if (smsCtx->cpmsInfo.usimStorage.totalRec != 0) {
            return AT_SUCCESS;
        }
    }

    switch (usimInfoCtx->cardStatus) {
        case USIMM_CARDAPP_SERVIC_AVAILABLE:
            rst = AT_SUCCESS;
            break;
        case USIMM_CARDAPP_SERVIC_SIM_PIN:
            rst = AT_CME_SIM_PIN_REQUIRED;
            break;
        case USIMM_CARDAPP_SERVIC_SIM_PUK:
            rst = AT_CME_SIM_PUK_REQUIRED;
            break;
        case USIMM_CARDAPP_SERVIC_UNAVAILABLE:
        case USIMM_CARDAPP_SERVIC_NET_LCOK:
        case USIMM_CARDAPP_SERVICE_IMSI_LOCK:
            rst = AT_CME_SIM_FAILURE;
            break;
        case USIMM_CARDAPP_SERVIC_ABSENT:
            rst = AT_CME_SIM_NOT_INSERTED;
            break;

        case USIMM_CARDAPP_SERVIC_BUTT:

            rst = AT_CME_SIM_BUSY;
            break;

        default:
            rst = AT_ERROR;
            break;
    }

    return rst;
}

VOS_VOID At_SetCmdSubMode(AT_ClientIdUint16 clientId, AT_CmdSubModeUint8 subMode)
{
    DMS_PortIdUint16        portId;
    DMS_PortModeUint8       mode;

    portId  = g_atClientTab[clientId].portNo;
    mode    = DMS_PORT_GetMode(portId);

    if  (mode != DMS_PORT_MODE_CMD &&
         mode != DMS_PORT_MODE_ONLINE_CMD) {
        AT_ERR_LOG1("At_SetCmdSubMode mode abnormal", mode);
        return;
    }

    g_parseContext[clientId].mode = subMode;
}

AT_CmdSubModeUint8 At_GetCmdSubMode(AT_ClientIdUint16 clientId)
{
    return g_parseContext[clientId].mode;
}

TAF_UINT32 AT_CountDigit(TAF_UINT8 *data, TAF_UINT32 len, TAF_UINT8 Char, TAF_UINT32 indexNum)
{
    TAF_UINT8 *tmp    = TAF_NULL_PTR;
    TAF_UINT8 *check  = TAF_NULL_PTR;
    TAF_UINT32 count  = 0;
    TAF_UINT32 chkLen = 0;
    TAF_UINT32 tmpLen = 0;

    tmp = data;

    /* 检查输入参数 */
    if ((*tmp == Char) || (*((tmp + len) - 1) == Char)) {
        return 0;
    }

    check = data;
    while (tmpLen++ < len) {
        /* 判断是否是数字或者是特定字符 */
        if (((*check >= '0') && (*check <= '9')) || ((*check == Char) && (*(check + 1) != Char))) {
            check++;
        } else {
            return 0;
        }
    }

    while (chkLen++ < len) {
        if (*tmp == Char) {
            count++;
            if (count == indexNum) {
                return chkLen;
            }
        }
        tmp++;
    }

    return 0;
}

/*
 * 功能描述: COPS设置命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcNetWorkTimerOut(VOS_UINT8 indexNum)
{
    return AT_CME_NETWORK_TIMEOUT;
}

/*
 * 功能描述: PC进行PPP拨号命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcPppOrgTimerOut(VOS_UINT8 indexNum)
{
    AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_REQ);
    /* 返回命令模式 */
    DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);

    /* 送NO CARRIER消息 */
    return AT_NO_CARRIER;
}

/*
 * 功能描述: ATD命令PPP拨号命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcAtdIpCallTimerOut(VOS_UINT8 indexNum)
{
    AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
    return AT_ERROR;
}

/*
 * 功能描述: 数据业务拨号断开命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcPsCallEndTimerOut(VOS_UINT8 indexNum)
{
    /* 返回命令模式 */
    DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);
    return AT_ERROR;
}

/*
 * 功能描述: 设置TMODE命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcTmodeTimerOut(VOS_UINT8 indexNum)
{
    return AT_ERROR;
}

/*
 * 功能描述: ABORT命令超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcAbortCmdTimerOut(VOS_UINT8 indexNum)
{
    return AT_ABORT;
}

/*
 * 功能描述: 未知错误超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcUnkonwnTimerOut(VOS_UINT8 indexNum)
{
    return AT_CMS_UNKNOWN_ERROR;
}

/*
 * 功能描述: 读电话本超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcCpbrTimerOut(VOS_UINT8 indexNum)
{
    AT_UartCtx          *uartCtx = VOS_NULL_PTR;

    uartCtx = AT_GetUartCtxAddr();

    if (AT_IsHsuartBaseUser(indexNum) == VOS_TRUE) {
        uartCtx->wmLowFunc = VOS_NULL_PTR;
    }

    return AT_ERROR;
}

/*
 * 功能描述: 语音模式设置超时处理
 */
LOCAL AT_RreturnCodeUint32 AT_ProcVmsetTimerOut(VOS_UINT8 indexNum)
{
    AT_InitVmSetCtx();
    return AT_ERROR;
}

/*
 * 功能描述: 和命令相关的计时器，超时处理函数表
 */
LOCAL AT_PS_CMD_TIMEOUT_FUNC AT_GetCmdTimerOutResultFunc(AT_CmdCurOptUint32 cmdCurrentOpt)
{
    const AT_PS_CmdTimeoutResult  *cmdTimeoutFuncTblPtr = VOS_NULL_PTR;
    AT_PS_CMD_TIMEOUT_FUNC         cmdTimeoutFunc       = VOS_NULL_PTR;
    VOS_UINT32                          cnt;

    cmdTimeoutFuncTblPtr = AT_GET_CMD_TIMEOUT_RSLT_FUNC_TBL_PTR();

    for (cnt = 0; cnt < AT_GET_CMD_TIMEOUT_RSLT_FUNC_TBL_SIZE(); cnt++) {
        if (cmdCurrentOpt == cmdTimeoutFuncTblPtr[cnt].cmdCurrentOpt) {
            cmdTimeoutFunc = cmdTimeoutFuncTblPtr[cnt].cmdTimeoutFunc;
            break;
        }
    }

    return cmdTimeoutFunc;
}

VOS_VOID At_InterTimerOutProc(VOS_UINT8 indexNum)
{
    AT_PS_CMD_TIMEOUT_FUNC cmdTimeoutFunc = VOS_NULL_PTR;
    AT_RreturnCodeUint32 result;

    result  = AT_FAILURE;

    cmdTimeoutFunc = AT_GetCmdTimerOutResultFunc(g_atClientTab[indexNum].cmdCurrentOpt);

    /* 如果处理函数存在则调用 */
    if (cmdTimeoutFunc != VOS_NULL_PTR) {
        result = cmdTimeoutFunc(indexNum);
    } else {
        AT_ERR_LOG1("At_InterTimerOutProc: not find func, cmdCurrentOpt is ", g_atClientTab[indexNum].cmdCurrentOpt);
        result = AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
    /* usMagic 小于 1024即可 */
    g_atClientTab[indexNum].magic = 0;
    g_atClientTab[indexNum].canAbort = 0;
    if (indexNum == g_current_user_at) {
        g_atClientTab[AT_CLIENT_ID_APP].isWaitAts = 0;
        g_current_user_at = VOS_NULL_PARA;
    }
#endif
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_DockHandleCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    errno_t    memResult;
    VOS_UINT16 length = 0;
    VOS_UINT8  cmdTmp[AT_CMDTMP_MAX_LEN + 1] = {0};
    VOS_UINT8  i;

    if (len > AT_CMDTMP_MAX_LEN) {
        return AT_FAILURE;
    }

    /* 将该AT命令从应用转发给E5 */
    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if (g_atClientTab[i].userType == AT_APP_USER) {
            (VOS_VOID)memset_s(cmdTmp, sizeof(cmdTmp), 0x00, sizeof(cmdTmp));
            if (len > 0) {
                memResult = memcpy_s(cmdTmp, sizeof(cmdTmp), data, len);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cmdTmp), len);
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%s", g_atCrLf, cmdTmp, g_atCrLf);
            At_SendResultData(i, g_atSndCodeAddress, length);
            return AT_SUCCESS;
        }
    }

    return AT_FAILURE;
}

/*
 * Description: 命令名匹配
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_MatchCmdName(TAF_UINT16 indexNum, TAF_UINT32 cmdType)
{
    /*   */
    return AT_FAILURE;
}

VOS_UINT32 AT_HandleDockSetCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32 result;

    result = AT_ParseSetDockCmd(data, len);
    if (result != AT_SUCCESS) {
        return AT_FAILURE;
    }

    if (At_MatchCmdName(indexNum, g_atCmdFmtType) == AT_FAILURE) {
        At_FormatResultData(indexNum, AT_CMD_NOT_SUPPORT);
        return AT_SUCCESS;
    }

    AT_SaveCmdElementInfo(indexNum, (VOS_UINT8 *)"^DOCK", AT_EXTEND_CMD_TYPE);

    result = At_SetParaCmd(indexNum);

    At_FormatResultData(indexNum, result);

    return AT_SUCCESS;
}

VOS_VOID AT_RcvTiS0Expired(REL_TimerMsgBlock *msg)
{
    AT_ModemCcCtx    *ccCtx = VOS_NULL_PTR;
    MN_CALL_SupsParam callMgmtParam;
    TAF_Ctrl          ctrl;
    VOS_UINT8         clientIndex;
    VOS_UINT16        reqClientId;
    ModemIdUint16     modemId;

    /* 根据timer name获取属于哪个index */
    clientIndex = (VOS_UINT8)(msg->name >> 12);

    ccCtx = AT_GetModemCcCtxAddrFromClientId(clientIndex);

    (VOS_VOID)memset_s(&callMgmtParam, sizeof(callMgmtParam), 0x00, sizeof(callMgmtParam));

    /* 目前只有voice支持自动接听功能，calltype 固定填为voice */
    callMgmtParam.callType    = MN_CALL_TYPE_VOICE;
    callMgmtParam.callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH;

    callMgmtParam.callId = (MN_CALL_ID_T)msg->para;

    /*
     * 来电ClientId为广播ClientId 0x3fff或者0x7fff，
     * AT在处理incoming事件时，判断已启动自动接听，启动定时器时ulName中带的是Client Index即0x3f或者0x7f
     * 所以此处调用MN_CALL_Sups时需要填ClientId
     */
    if (clientIndex == AT_BROADCAST_CLIENT_INDEX_MODEM_0) {
        reqClientId = AT_BROADCAST_CLIENT_ID_MODEM_0;
    } else if (clientIndex == AT_BROADCAST_CLIENT_INDEX_MODEM_1) {
        reqClientId = AT_BROADCAST_CLIENT_ID_MODEM_1;
    } else {
        reqClientId = AT_BROADCAST_CLIENT_ID_MODEM_2;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = reqClientId;
    ctrl.opId     = 0;

    if (AT_GetModemIdFromClient(reqClientId, &modemId) != VOS_OK) {
        return;
    }

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &callMgmtParam, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callMgmtParam), modemId) !=
        VOS_OK) {
        AT_WARN_LOG("AT_ProcTimerS0: S0 Answer Fail");
    }
    AT_PR_LOGH("AT_RcvTiS0Expired: CallId = %d", callMgmtParam.callId);
    ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
    ccCtx->s0TimeInfo.timerName  = 0;
}

VOS_VOID AT_RcvTiShutDownExpired(REL_TimerMsgBlock *tmrMsg)
{
#if (VOS_WIN32 == VOS_OS_VER || OSA_CPU_CCPU == VOS_OSA_CPU)
    AT_SysbootShutdown((sysboot_shutdown_reason_e)0);
#else
    AT_SysbootShutdown();
#endif
}

AT_PS_INTERNAL_TIMEOUT_FUNC AT_GetInternalTimeOutResultFunc(VOS_UINT32 interTimerName)
{
    const AT_PS_InternalTimeoutResult  *interTimeoutFuncTblPtr = VOS_NULL_PTR;
    AT_PS_INTERNAL_TIMEOUT_FUNC         interTimeoutFunc       = VOS_NULL_PTR;
    VOS_UINT32                          cnt;

    interTimeoutFuncTblPtr = AT_GET_INTERNAL_TIMEOUT_RSLT_FUNC_TBL_PTR();

    for (cnt = 0; cnt < AT_GET_INTERNAL_TIMEOUT_RSLT_FUNC_TBL_SIZE(); cnt++) {
        if (interTimerName == interTimeoutFuncTblPtr[cnt].interTimerName) {
            interTimeoutFunc = interTimeoutFuncTblPtr[cnt].internalTimeoutFunc;
            break;
        }
    }

    return interTimeoutFunc;
}

TAF_VOID At_TimeOutProc(REL_TimerMsgBlock *msg)
{
    /* 局部变量声明及初始化 */
    AT_PS_INTERNAL_TIMEOUT_FUNC interTimeoutFunc = VOS_NULL_PTR;
    VOS_UINT8                   indexNum;

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_TimeOutProc pMsg is NULL");
        return;
    }

    indexNum = (VOS_UINT8)msg->name;
    if ((msg->name & 0x00000f00) == AT_INTERNAL_PROCESS_TYPE) {
        interTimeoutFunc = AT_GetInternalTimeOutResultFunc(msg->name & 0x000000ff);

        /* 如果处理函数存在则调用 */
        if (interTimeoutFunc != VOS_NULL_PTR) {
            interTimeoutFunc(msg);
        } else {
            AT_ERR_LOG1("At_TimeOutProc: not find timeout func, Msg is ", msg->name);
        }
        return;
    } else {
        indexNum = (VOS_UINT8)msg->name;
        if (indexNum >= AT_MAX_CLIENT_NUM) {
            AT_WARN_LOG("At_TimeOutProc FAILURE");
            return;
        }

        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CURRENT_OPT_BUTT) {
            AT_WARN_LOG("At_TimeOutProc AT_CMD_CURRENT_OPT_BUTT");
            return;
        }

        AT_LOG1("At_TimeOutProc ucIndex:", indexNum);
        AT_LOG1("At_TimeOutProc g_atClientTab[ucIndex].CmdCurrentOpt:", g_atClientTab[indexNum].cmdCurrentOpt);

        At_InterTimerOutProc(indexNum);
    }
}
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT8* At_GetStrContent(TAF_UINT32 type)
{
    if (type < AT_STRING_BUTT) {
        return g_atStringTab[type].text;
    } else {
        return g_atStringTab[AT_STRING_BUTT].text;
    }
}

/* PC工程中AT从A核移到C核, At_sprintf有重复定义,故在此处添加条件编译宏 */

/*
 * 功能描述: 检查入参合法性
 */
LOCAL VOS_INT32 AT_CheckFormatReportString(VOS_CHAR *headAddr, VOS_CHAR *currAddr)
{
    if ((headAddr == VOS_NULL_PTR) || (currAddr == VOS_NULL_PTR)) {
        AT_ERR_LOG("AT_CheckFormatReportString: Parameter of the function is null.");
        return VOS_FALSE;
    }

    if (currAddr < headAddr) {
        AT_ERR_LOG("AT_CheckFormatReportString: addr error.");
        return VOS_FALSE;
    }
    return VOS_TRUE;
}

/*
 * 功能描述: 检查格式化字符串的类型
 */
LOCAL VOS_VOID AT_ProcFormatReportType(VOS_CHAR **data, VOS_UINT32 *signFlag, VOS_UINT32 *len1, VOS_UINT32 *len2)
{
    (*len1) = 0;
    (*len2) = 0;
    (*data)++;

    if ((**data) == '+' && *(*data + 1) == 'd') {
        (*data)++;
        (*signFlag) = VOS_TRUE;
        (*len1)     = 0; /* 有符号整型打印符号 */
    }

    if ((**data) == '0') {
        (*data)++;
        (*len1) = 0; /* 指定的宽度，或填充 */
    }

    /* 目前，只支持小于等于9位的宽度 */
    if (((**data) >= '0') && ((**data) <= '9')) {
        (*len1) = (VOS_UINT32)(VOS_INT32)((**data) - '0'); /* 指定的宽度 */
        (*data)++;
    }
}

/*
 * 功能描述: 字符串类型的处理
 */
LOCAL VOS_VOID AT_ProcFormatTypeForString(VOS_CHAR *s , VOS_INT32 *length)
{
    VOS_CHAR  *sc       = NULL;

    if (s == NULL) {
        (*length) += (VOS_INT32)strlen("(null)");
        return;
    }

    for (sc = s; *sc != '\0'; ++sc) {
        (*length)++;
    }
    return;
}

/*
 * 功能描述: 整数类型的处理
 */
LOCAL VOS_VOID AT_ProcFormatTypeForInteger(VOS_INT32 numSrc, VOS_UINT32 len1, VOS_INT32 *length,
    VOS_UINT32 *len2, VOS_UINT32 *signFlag)
{
    VOS_INT32  numTemp  = numSrc; /* 转换后的数据 */

    (*length)++;
    (*len2)++; /* 实际的宽度 */

    /* 有符号数十进制输出，当前数字大于9或小于-9时，需要通过除10计算数字宽度 */
    while ((numTemp > AT_DEC_MAX_NUM) || (numTemp < AT_DEC_SIGNED_MIN_NUM)) {
        numTemp = numTemp / 10;
        (*length)++;
        (*len2)++; /* 实际的宽度 */
    }

    if ((*len2) < len1) {
        (*length) += (VOS_INT32)(len1 - (*len2));
    }
    /* 如果ulLen2 >= ulLen1,且lNumSrc<0，则要加上负号的一个空间 */
    else if (numSrc < 0 || (*signFlag) == VOS_TRUE) {
        (*length)++;
        (*signFlag) = VOS_FALSE;
    } else {
    }
}

/*
 * 功能描述: 无符号整数类型的处理
 */
LOCAL VOS_VOID AT_ProcFormatTypeForUnInteger(VOS_UINT32 num, VOS_UINT32 len1, VOS_INT32 *length, VOS_UINT32 *len2)
{
    (*length)++;
    (*len2)++; /* 实际的宽度 */
    /* 无符号数十进制输出，当前数字大于9时，需要通过除10计算数字宽度 */
    while (num > AT_DEC_MAX_NUM) {
        num = num / 10;
        (*length)++;
        (*len2)++; /* 实际的宽度 */
    }

    if ((*len2) < len1) {
        (*length) += (VOS_INT32)(len1 - (*len2));
    }
}

/*
 * 功能描述: 十六进制类型的处理
 */
LOCAL VOS_VOID AT_ProcFormatTypeForHex(VOS_UINT32 num, VOS_UINT32 len1, VOS_INT32 *length, VOS_UINT32 *len2)
{
    (*length)++;
    (*len2)++; /* 实际的宽度 */

    while (num > 0x0f) {
        /* 十六进制输出，当前数字大于0x0f时，需要通过除16计算数字宽度 */
        num = num / 16;
        (*length)++;
        (*len2)++; /* 实际的宽度 */
    }

    if ((*len2) < len1) {
        (*length) += (VOS_INT32)(len1 - (*len2));
    }
}

/*lint -e713 -e507 -e530*/
VOS_INT32 AT_FormatReportString(VOS_INT32 maxLength, VOS_CHAR *headAddr, VOS_CHAR *currAddr, const VOS_CHAR *fmt, ...)
{
    VOS_INT32  length   = 0;
    VOS_UINT32 num      = 0;
    VOS_INT32  numSrc   = 0; /* 原始数据 */
    VOS_UINT32 signFlag = VOS_FALSE;
    VOS_CHAR  *data     = NULL;
    VOS_CHAR  *s        = NULL;
    va_list    args;
    VOS_UINT32 len1 = 0; /* ulLen1:%后设置的长度; */
    VOS_UINT32 len2 = 0; /* ulLen2:实际读取的长度 */

    if (AT_CheckFormatReportString(headAddr, currAddr) != VOS_TRUE) {
        return 0;
    }

    va_start(args, fmt);
    for (data = (VOS_CHAR *)fmt; *data != '\0'; ++data) {
        if (*data != '%') {
            length++; /* 加1 */
            continue;
        }

        AT_ProcFormatReportType(&data, &signFlag, &len1, &len2);

        /* 目前，只会用到c,d,s,X,四种，其它一律认为出错 */
        switch (*data) {
            case 'c':
                length++;
                break;

            case 's':
                s = va_arg(args, VOS_CHAR *);
                AT_ProcFormatTypeForString(s, &length);
                break;

            case 'd':
                numSrc  = va_arg(args, VOS_INT32);
                AT_ProcFormatTypeForInteger(numSrc, len1, &length, &len2, &signFlag);
                break;

            case 'u':
                num = va_arg(args, VOS_UINT32);
                AT_ProcFormatTypeForUnInteger(num, len1, &length, &len2);
                break;

            case 'X':
            case 'x':
                num = va_arg(args, VOS_UINT32);
                AT_ProcFormatTypeForHex(num, len1, &length, &len2);
                break;

            default:
                va_end(args);
                return 0;
        }
    }
    va_end(args);

    if (currAddr - headAddr + length >= (VOS_INT32)(maxLength)) {
        return 0;
    }

    va_start(args, fmt);

    if (length != vsnprintf_s(currAddr, (VOS_SIZE_T)(maxLength - (currAddr - headAddr)),
                              (VOS_SIZE_T)(maxLength - (currAddr - headAddr) - 1), fmt, args)) {
        va_end(args);
        return 0;
    }

    va_end(args);

    return length;
}

/*lint +e713 +e507 +e530*/

TAF_VOID AT_MapCallTypeModeToClccMode(MN_CALL_TypeUint8 callType, VOS_UINT8 *clccMode)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
        case MN_CALL_TYPE_PSAP_ECALL:
            *clccMode = AT_CLCC_PARA_MODE_VOICE;
            break;
        case MN_CALL_TYPE_FAX:
            *clccMode = AT_CLCC_PARA_MODE_FAX;
            break;
        default:
            *clccMode = AT_CLCC_PARA_MODE_DATA;
            break;
    }
}

AT_CmdCurOptUint32 At_GetMnOptType(MN_MMI_OperationTypeUint8 mnOptType)
{
    AT_CmdCurOptUint32  cmdCurrentOpt = AT_CMD_INVALID;
    VOS_UINT32          loop;
    VOS_UINT32          itemsNum;

    itemsNum = sizeof(g_atMnOptTypeConvertAtCmdTypeTab) / sizeof(AT_MnOptTypeConvertAtCmdOptType);

    for (loop = 0; loop < itemsNum; loop++) {
        if ((VOS_UINT8)g_atMnOptTypeConvertAtCmdTypeTab[loop].mnOptType == mnOptType) {
            cmdCurrentOpt = (AT_CmdCurOptUint32)g_atMnOptTypeConvertAtCmdTypeTab[loop].atCmdType;
            break;
        }
    }

    return cmdCurrentOpt;
}

VOS_UINT32 At_UnlockSimLock(VOS_UINT8 indexNum, VOS_UINT32 paraCnt, AT_ParseParaType *paralist)
{
    AT_ParseParaType       *pwdPara = VOS_NULL_PTR;
    DRV_AGENT_SimlockSetReq simlockReq;
    errno_t                 memResult;

    simlockReq.pwdLen = 0;

    if (paraCnt == AT_SIMLOCK_VALID_PARA_NUM) {
        pwdPara = (AT_ParseParaType *)(paralist + 1);

        if (pwdPara->paraLen == TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX) {
            if (At_CheckNumString(pwdPara->para, TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX) != AT_FAILURE) {
                simlockReq.pwdLen = TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX;

                memResult = memcpy_s(simlockReq.pwd, sizeof(simlockReq.pwd), pwdPara->para,
                                     TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(simlockReq.pwd), TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
                simlockReq.pwd[simlockReq.pwdLen] = 0;
            }
        }
    }

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                               DRV_AGENT_SIMLOCK_SET_REQ, (VOS_UINT8 *)&simlockReq, sizeof(simlockReq),
                               I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        /* 设置当前操作模式 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DRV_AGENT_SIMLOCK_SET_REQ;
        (VOS_VOID)memset_s(simlockReq.pwd, sizeof(simlockReq.pwd), 0x00, sizeof(simlockReq.pwd));
        /* 由于SIMLOCK特殊处理，这里即使返回等待异步消息解析器也不会起定时器 */
        if (At_StartTimer(AT_SET_PARA_TIME, indexNum) != AT_SUCCESS) {
            At_FormatResultData(indexNum, AT_ERROR);
            return VOS_ERR;
        }

        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;

        return VOS_OK;
    }

    (VOS_VOID)memset_s(simlockReq.pwd, sizeof(simlockReq.pwd), 0x00, sizeof(simlockReq.pwd));
    At_FormatResultData(indexNum, AT_ERROR);
    return VOS_ERR;
}

/*
 * 功能描述: 获取xml的操作类型
 */
LOCAL VOS_UINT32 At_GetXmlProcType(const TAF_UINT8 *data, VOS_UINT16 *tmpLen, AT_XmlProcTypeUint8 *xmlProcType, MTA_AT_CposOperateTypeUint32 *cposOpType)
{
    /* 收到"Ctrl-Z"字符，发送本条消息 */
    if (data[(*tmpLen) - 1] == AT_CTRL_Z) {
        (*tmpLen) -= 1;
        (*xmlProcType) = AT_XML_PROC_TYPE_FINISH;
        (*cposOpType)  = MTA_AT_CPOS_SEND;
    }
    /* 收到"CR"字符，继续等待输入 */
    else if (data[(*tmpLen) - 1] == AT_CR) {
        (*xmlProcType) = AT_XML_PROC_TYPE_CONTINUE;
    }
    /* 收到"ESC"字符，取消本条消息发送 */
    else if (data[(*tmpLen) - 1] == AT_ESC) {
        (*tmpLen) -= 1;
        (*xmlProcType) = AT_XML_PROC_TYPE_FINISH;
        (*cposOpType)  = MTA_AT_CPOS_CANCEL;
    } else {
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_ProcXmlText(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len)
{
    errno_t                      memResult;
    VOS_UINT16                   tmpLen;
    AT_XmlProcTypeUint8          xmlProcType;
    AT_MTA_CposReq              *cposSetReq = VOS_NULL_PTR;
    VOS_UINT32                   result;
    MTA_AT_CposOperateTypeUint32 cposOpType;
    AT_ModemAgpsCtx             *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(indexNum);

    /* 局部变量初始化 */
    tmpLen      = len;
    xmlProcType = AT_XML_PROC_TYPE_BUTT;
    cposOpType  = MTA_AT_CPOS_BUTT;

    if (At_GetXmlProcType(data, &tmpLen, &xmlProcType, &cposOpType) == AT_ERROR) {
        return AT_ERROR;
    }

    /* 把缓冲区的字符加到XML码流中 */
    if (cposOpType != MTA_AT_CPOS_CANCEL) {
        /* XML码流长度判断 */
        if (((agpsCtx->xml.xmlTextCur - agpsCtx->xml.xmlTextHead) + tmpLen) > AT_XML_MAX_LEN) {
            return AT_ERROR;
        }

        if (tmpLen > 0) {
            memResult = memcpy_s(agpsCtx->xml.xmlTextCur,
                                 AT_XML_MAX_LEN - (unsigned int)(agpsCtx->xml.xmlTextCur - agpsCtx->xml.xmlTextHead),
                                 data, tmpLen);
            TAF_MEM_CHK_RTN_VAL(memResult,
                                AT_XML_MAX_LEN - (unsigned int)(agpsCtx->xml.xmlTextCur - agpsCtx->xml.xmlTextHead),
                                tmpLen);
        }

        agpsCtx->xml.xmlTextCur += tmpLen;
    }

    /* 下发XML码流或继续等待输入 */
    if (xmlProcType == AT_XML_PROC_TYPE_FINISH) {
        cposSetReq = (AT_MTA_CposReq *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(AT_MTA_CposReq));
        if (cposSetReq == VOS_NULL_PTR) {
            AT_ERR_LOG("At_ProcXmlText: Memory malloc failed!");
            return AT_ERROR;
        }
        (VOS_VOID)memset_s(cposSetReq, sizeof(AT_MTA_CposReq), 0x00, sizeof(AT_MTA_CposReq));

        /* 填写消息结构 */
        cposSetReq->cposOpType = cposOpType;

        /* 当用户输入了Ctrl-Z才将码流发给MTA */
        if (cposOpType == MTA_AT_CPOS_SEND) {
            cposSetReq->xmlLength = (VOS_UINT32)(agpsCtx->xml.xmlTextCur - agpsCtx->xml.xmlTextHead);
            if (cposSetReq->xmlLength > 0) {
                memResult = memcpy_s(cposSetReq->xmlText, MTA_CPOS_XML_MAX_LEN, agpsCtx->xml.xmlTextHead,
                                     cposSetReq->xmlLength);
                TAF_MEM_CHK_RTN_VAL(memResult, MTA_CPOS_XML_MAX_LEN, cposSetReq->xmlLength);
            }
        }

        /* 发送到MTA模块 */
        result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                        ID_AT_MTA_CPOS_SET_REQ, (VOS_UINT8 *)cposSetReq, sizeof(AT_MTA_CposReq),
                                        I0_UEPS_PID_MTA);

        /* 释放XML码流结构 */
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, cposSetReq);
        /*lint -restore */

        if (result != TAF_SUCCESS) {
            AT_ERR_LOG("At_ProcXmlText: AT_FillAndSndAppReqMsg fail.");
            return AT_ERROR;
        }

        /* 设置AT模块实体的状态为等待异步返回 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPOS_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_WAIT_XML_INPUT;
}

/* 删除^EOPLMN设置命令的参数解析函数，使用通用AT参数解析器解析 */
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

TAF_UINT32 At_CheckCurrRatModeIsCL(VOS_UINT8 indexNum)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    getModemIdRslt;
    VOS_UINT8    *isCLMode = VOS_NULL_PTR;

    getModemIdRslt = AT_GetModemIdFromClient(indexNum, &modemId);

    /* 获取modemid失败，默认不拦截相应的AT命令 */
    if (getModemIdRslt != VOS_OK) {
        AT_ERR_LOG("At_CurrRatModeIsCL:Get Modem Id fail!");
        return VOS_FALSE;
    }

    isCLMode = AT_GetModemCLModeCtxAddrFromModemId(modemId);

    if (*isCLMode == VOS_TRUE) {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
}
#endif

VOS_UINT32 AT_ConvertMtaResult(MTA_AT_ResultUint32 result)
{
    AT_RreturnCodeUint32 returnCode;

    switch (result) {
        case MTA_AT_RESULT_NO_ERROR:
            returnCode = AT_OK;
            break;
        case MTA_AT_RESULT_INCORRECT_PARAMETERS:
            returnCode = AT_CME_INCORRECT_PARAMETERS;
            break;
        case MTA_AT_RESULT_NETWORK_TIMEOUT:
            returnCode = AT_CME_NETWORK_TIMEOUT;
            break;
        case MTA_AT_RESULT_ERROR:
        default:
            returnCode = AT_ERROR;
            break;
    }

    return returnCode;
}

VOS_UINT32 AT_GetSeconds(VOS_VOID)
{
    VOS_UINT32 ulsecond;
    VOS_UINT32 ultimelow    = 0;
    VOS_UINT32 ultimehigh   = 0;
    VOS_UINT64 ulltimeCount = 0;

    if (AT_GetTimerAccuracyTimestamp(&ultimehigh, &ultimelow) == ERR_MSP_SUCCESS) {
        ulltimeCount = ((VOS_UINT64)ultimehigh << 32) | ((VOS_UINT64)ultimelow);
        ulltimeCount = ulltimeCount / SLICE_TO_SECOND_UINT;
    }

    /* 将U64的slice转换为秒之后，只需要取低32位即可。低32位的秒能统计的时间已经达到136年 */
    ulsecond = (VOS_UINT32)(ulltimeCount & SLICE_TO_MAX_SECOND);

    return ulsecond;
}

VOS_VOID At_AdjustLocalDate(TIME_ZONE_Time *uinversalTime, VOS_INT8 adjustValue, TIME_ZONE_Time *localTime)
{
    VOS_UINT8 day;

    /* 调整日期 */
    day = (VOS_UINT8)(uinversalTime->day + adjustValue);

    if (day == 0) {
        /* 月份减一 */
        if (uinversalTime->month == 1) {
            /* 调整为上一年的12月份,年份减一 */
            localTime->month = 12;

            if (uinversalTime->year == 0) {
                /* 如果是2000年，调整为1999年 */
                localTime->year = 99;
            } else {
                localTime->year = uinversalTime->year - 1;
            }
        } else {
            localTime->month = uinversalTime->month - 1;
            localTime->year  = uinversalTime->year;
        }

        /* 日期调整为上个月的最后一天, */
        localTime->day = At_GetDaysForEachMonth(localTime->year, localTime->month);
    } else if (day > At_GetDaysForEachMonth(uinversalTime->year, uinversalTime->month)) {
        /* 日期调整为下个月一号 */
        localTime->day = 1;

        /* 月份加一 当前月份为12 */
        if (uinversalTime->month == 12) {
            /* 调整为下一年的1月份,年份加一 */
            localTime->month = 1;
            localTime->year  = uinversalTime->year + 1;
        } else {
            localTime->month = uinversalTime->month + 1;
            localTime->year  = uinversalTime->year;
        }
    } else {
        localTime->day   = day;
        localTime->month = uinversalTime->month;
        localTime->year  = uinversalTime->year;
    }
}

VOS_VOID AT_GetLiveTime(NAS_MM_InfoInd *aTtime, NAS_MM_InfoInd *newTime, VOS_UINT32 nwSecond)
{
    TIME_ZONE_Time newTimeInfo = {0};
    VOS_UINT32     nowSecond;
    VOS_UINT32     seconds;
    VOS_UINT32     adjustDate;
    VOS_UINT32     indexNum;
    errno_t        memResult = 0;

    /* 全局变量中保存有时间信息 */
    if ((aTtime->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        memResult = memcpy_s(newTime, sizeof(NAS_MM_InfoInd), aTtime, sizeof(NAS_MM_InfoInd));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(NAS_MM_InfoInd), sizeof(NAS_MM_InfoInd));

        if (nwSecond == 0) {
            return;
        }

        nowSecond = AT_GetSeconds();

        /* 如果ulNowSecond 超过了slice表示的最大值则进会反转，反转之后需要加上最大值 */
        if (nowSecond >= nwSecond) {
            seconds = nowSecond - nwSecond;
        } else {
            /* 参考其余溢出流程实现, +1的原因是经过0的计数 */
            seconds = SLICE_TO_MAX_SECOND - nwSecond + nowSecond + 1;
        }

        if (seconds != 0) {
            memResult = memcpy_s(&newTimeInfo, sizeof(TIME_ZONE_Time), &aTtime->universalTimeandLocalTimeZone,
                                 sizeof(TIME_ZONE_Time));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TIME_ZONE_Time), sizeof(TIME_ZONE_Time));

            newTimeInfo.second += (VOS_UINT8)(seconds % AT_MINUTE_TO_SECONDS);                          /* 秒 */
            newTimeInfo.minute += (VOS_UINT8)((seconds / AT_MINUTE_TO_SECONDS) % AT_MINUTE_TO_SECONDS); /* 分 */
            newTimeInfo.hour +=
                (VOS_UINT8)((seconds / (AT_MINUTE_TO_SECONDS * AT_HOUR_TO_MINUTES)) % AT_DAY_TO_HOURS); /* 时 */
            adjustDate = (seconds / (AT_MINUTE_TO_SECONDS * AT_HOUR_TO_MINUTES * AT_DAY_TO_HOURS));     /* 天 */

            if (newTimeInfo.second >= AT_MINUTE_TO_SECONDS) {
                newTimeInfo.minute += newTimeInfo.second / AT_MINUTE_TO_SECONDS; /* 分 */
                newTimeInfo.second = newTimeInfo.second % AT_MINUTE_TO_SECONDS;  /* 实际的秒数 */
            }

            if (newTimeInfo.minute >= AT_HOUR_TO_MINUTES) {
                newTimeInfo.hour += newTimeInfo.minute / AT_HOUR_TO_MINUTES;  /* 小时 */
                newTimeInfo.minute = newTimeInfo.minute % AT_HOUR_TO_MINUTES; /* 实际的分数 */
            }

            if ((adjustDate > 0) || (newTimeInfo.hour >= AT_DAY_TO_HOURS)) {
                adjustDate += (newTimeInfo.hour / AT_DAY_TO_HOURS);    /* 天数 */
                newTimeInfo.hour = newTimeInfo.hour % AT_DAY_TO_HOURS; /* 实际的小时数 */

                memResult = memcpy_s(&newTime->universalTimeandLocalTimeZone, sizeof(TIME_ZONE_Time), &newTimeInfo,
                                     sizeof(TIME_ZONE_Time));
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TIME_ZONE_Time), sizeof(TIME_ZONE_Time));

                for (indexNum = 0; indexNum < adjustDate; indexNum++) {
                    At_AdjustLocalDate(&newTimeInfo, 1, &newTime->universalTimeandLocalTimeZone);
                    memResult = memcpy_s(&newTimeInfo, sizeof(TIME_ZONE_Time), &newTime->universalTimeandLocalTimeZone,
                                         sizeof(TIME_ZONE_Time));
                    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TIME_ZONE_Time), sizeof(TIME_ZONE_Time));
                }
            } else {
                memResult = memcpy_s(&newTime->universalTimeandLocalTimeZone, sizeof(TIME_ZONE_Time), &newTimeInfo,
                                     sizeof(TIME_ZONE_Time));
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TIME_ZONE_Time), sizeof(TIME_ZONE_Time));
            }
        }
    }
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT8 AT_IsModemLoopOn(VOS_VOID)
{
    return g_modemLoop;
}

VOS_VOID AT_WriteModemLoop(VOS_UINT8 value)
{
    g_modemLoop = value;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
/*
 * 功能描述: 收到"Ctrl-Z"字符，发送消息处理
 */
LOCAL VOS_UINT32 At_ProcStreamTypeFinish(VOS_UINT8 clientId, AT_ModemUePolicyCtx *uePolicyCtx)
{
    TAF_PS_5GUePolicyInfo *uePolicyInfo   = VOS_NULL_PTR;
    errno_t                memResult;
    VOS_UINT32             allocLen;

    allocLen     = sizeof(TAF_PS_5GUePolicyInfo) + uePolicyCtx->uePolicyLen / 2;
    uePolicyInfo = (TAF_PS_5GUePolicyInfo *)PS_MEM_ALLOC(WUEPS_PID_AT, allocLen);

    if (uePolicyInfo == VOS_NULL_PTR) {
        AT_ERR_LOG("At_ProcStreamTypeFinish: Memory malloc failed!");
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(uePolicyInfo, allocLen, 0x00, allocLen);

    /* 填写消息结构 */
    uePolicyInfo->protocalVer     = uePolicyCtx->protocalVer;
    uePolicyInfo->pti             = uePolicyCtx->pti;
    uePolicyInfo->uePolicyMsgType = uePolicyCtx->uePolicyMsgType;
    uePolicyInfo->classMarkLen    = uePolicyCtx->classMarkLen;
    memResult = memcpy_s(uePolicyInfo->classMark, TAF_PS_UE_POLICY_CLASSMARK_MAX_LEN, uePolicyCtx->classMark,
                         TAF_PS_UE_POLICY_CLASSMARK_MAX_LEN);

    TAF_MEM_CHK_RTN_VAL(memResult, TAF_PS_UE_POLICY_CLASSMARK_MAX_LEN, TAF_PS_UE_POLICY_CLASSMARK_MAX_LEN);

    uePolicyInfo->ueOsIdLen = uePolicyCtx->ueOsIdLen;
    memResult = memcpy_s(uePolicyInfo->ueOsIdInfo, TAF_PS_UE_OS_ID_INFO_MAX_LEN, uePolicyCtx->ueOsIdInfo,
                         TAF_PS_UE_OS_ID_INFO_MAX_LEN);

    TAF_MEM_CHK_RTN_VAL(memResult, TAF_PS_UE_OS_ID_INFO_MAX_LEN, TAF_PS_UE_OS_ID_INFO_MAX_LEN);

    uePolicyInfo->infoLen = uePolicyCtx->uePolicyLen / 2;
    if (At_AsciiString2HexSimple(uePolicyInfo->content, uePolicyCtx->uePolicyHead,
                                 (VOS_UINT16)uePolicyCtx->uePolicyLen) != AT_SUCCESS) {
        PS_MEM_FREE(WUEPS_PID_AT, uePolicyInfo);
        return AT_ERROR;
    }

    /* 调用DSM提供的设置接口 */
    if (TAF_PS_SetCsUePolicy(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[clientId].clientId), 0,
                             uePolicyInfo) != VOS_OK) {
        PS_MEM_FREE(WUEPS_PID_AT, uePolicyInfo);
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, uePolicyInfo);

    g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_CSUEPOLICY_SET;
    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 At_ProcUePolicyInfo(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length)
{
    errno_t                memResult;
    VOS_UINT16             tmpLen         = length - 1;
    AT_StreamProcTypeUint8 streamProcType = AT_STREAM_PROC_TYPE_BUTT;
    AT_ModemUePolicyCtx   *uePolicyCtx    = VOS_NULL_PTR;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);

    /* 收到"Ctrl-Z"字符，发送本条消息 */
    if (data[tmpLen] == AT_CTRL_Z) {
        streamProcType = AT_STREAM_PROC_TYPE_FINISH;
    }
    /* 收到"CR"字符，继续等待输入 */
    else if (data[tmpLen] == AT_CR) {
        streamProcType = AT_STREAM_PROC_TYPE_CONTINUE;
    }
    /* 收到"ESC"字符，取消本条消息发送 */
    else if (data[tmpLen] == AT_ESC) {
        streamProcType = AT_STREAM_PROC_TYPE_CANCEL;
    } else {
        return AT_ERROR;
    }

    /* 把缓冲区的字符加到码流中 */
    if (streamProcType != AT_STREAM_PROC_TYPE_CANCEL) {
        /* 码流长度判断 */
        if ((VOS_UINT32)((uePolicyCtx->uePolicyCur - uePolicyCtx->uePolicyHead) + tmpLen) > uePolicyCtx->uePolicyLen) {
            AT_ERR_LOG("At_ProcUePolicyInfo: UE POLICY stream too long!");
            return AT_ERROR;
        }

        if (tmpLen > 0) {
            memResult = memcpy_s(uePolicyCtx->uePolicyCur,
                                 uePolicyCtx->uePolicyLen -
                                     (unsigned int)(uePolicyCtx->uePolicyCur - uePolicyCtx->uePolicyHead),
                                 data, tmpLen);
            TAF_MEM_CHK_RTN_VAL(memResult,
                                uePolicyCtx->uePolicyLen -
                                    (unsigned int)(uePolicyCtx->uePolicyCur - uePolicyCtx->uePolicyHead),
                                tmpLen);
        }

        uePolicyCtx->uePolicyCur += tmpLen;
    }

    /* 下发码流或继续等待输入 */
    if (streamProcType == AT_STREAM_PROC_TYPE_FINISH) {
        if (At_ProcStreamTypeFinish(clientId, uePolicyCtx) == AT_ERROR) {
            return AT_ERROR;
        } else {
            return AT_WAIT_ASYNC_RETURN;
        }
    } else if (streamProcType == AT_STREAM_PROC_TYPE_CANCEL) {
        return AT_OK;
    } else {
        return AT_WAIT_UE_POLICY_INPUT;
    }
}

LOCAL VOS_UINT32 At_ProcSendImsUrsp(VOS_UINT8 clientId)
{
    AT_ModemUePolicyCtx   *uePolicyCtx  = VOS_NULL_PTR;
    AT_IMSA_ImsUrspSetReq *urspSetReq   = VOS_NULL_PTR;
    VOS_UINT32             allocLen;
    VOS_UINT32             rst;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);

    allocLen   = sizeof(AT_IMSA_ImsUrspSetReq) + uePolicyCtx->imsUrspLen / 2;
    urspSetReq = (AT_IMSA_ImsUrspSetReq *)PS_MEM_ALLOC(WUEPS_PID_AT, allocLen);

    if (urspSetReq == VOS_NULL_PTR) {
        AT_ERR_LOG("At_ProcSendImsUrsp: Memory malloc failed!");
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(urspSetReq, allocLen, 0x00, allocLen);

    /* 填写消息结构 */
    urspSetReq->version = (VOS_UINT16)uePolicyCtx->imsUrspVer;

    urspSetReq->length = (VOS_UINT16)(uePolicyCtx->imsUrspLen / 2);
    if (At_AsciiString2HexSimple(urspSetReq->data, uePolicyCtx->imsUrspHead,
                                 (VOS_UINT16)uePolicyCtx->imsUrspLen) != AT_SUCCESS) {
        PS_MEM_FREE(WUEPS_PID_AT, urspSetReq);
        return AT_ERROR;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[clientId].clientId, g_atClientTab[clientId].opId,
              ID_AT_IMSA_IMS_URSP_SET_REQ, (VOS_UINT8 *)(&urspSetReq->version),
              (VOS_UINT32)(sizeof(urspSetReq->version) + sizeof(urspSetReq->length) + urspSetReq->length),
              PS_PID_IMSA);

    if (rst != TAF_SUCCESS) {
        PS_MEM_FREE(WUEPS_PID_AT, urspSetReq);
        AT_ERR_LOG("At_ProcSendImsUrsp: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, urspSetReq);

    g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_IMSURSP_SET;

    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 At_ProcImsUrspInfo(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length)
{
    AT_ModemUePolicyCtx   *uePolicyCtx  = VOS_NULL_PTR;
    errno_t                memRslt;
    VOS_UINT16             tmpLen         = length - 1;
    AT_StreamProcTypeUint8 streamProcType = AT_STREAM_PROC_TYPE_BUTT;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);

    /* 收到"Ctrl-Z"字符，发送本条消息 */
    if (data[tmpLen] == AT_CTRL_Z) {
        streamProcType = AT_STREAM_PROC_TYPE_FINISH;
    }
    /* 收到"CR"字符，继续等待输入 */
    else if (data[tmpLen] == AT_CR) {
        streamProcType = AT_STREAM_PROC_TYPE_CONTINUE;
    }
    /* 收到"ESC"字符，取消本条消息发送 */
    else if (data[tmpLen] == AT_ESC) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }

    /* 把缓冲区的字符加到码流中 */
    if ((VOS_UINT32)((uePolicyCtx->imsUrspCur - uePolicyCtx->imsUrspHead) + tmpLen) > uePolicyCtx->imsUrspLen) {
        AT_ERR_LOG("At_ProcImsUrspInfo: ims ursp stream too long!");
        return AT_ERROR;
    }

    if (tmpLen > 0) {
        memRslt = memcpy_s(uePolicyCtx->imsUrspCur,
                      uePolicyCtx->imsUrspLen - (unsigned int)(uePolicyCtx->imsUrspCur - uePolicyCtx->imsUrspHead),
                      data, tmpLen);
        TAF_MEM_CHK_RTN_VAL(memRslt,
           uePolicyCtx->imsUrspLen - (unsigned int)(uePolicyCtx->imsUrspCur - uePolicyCtx->imsUrspHead), tmpLen);
    }

    uePolicyCtx->imsUrspCur += tmpLen;

    /* 下发码流或继续等待输入 */
    if (streamProcType == AT_STREAM_PROC_TYPE_FINISH) {
        /* 发送码流 */
        return At_ProcSendImsUrsp(clientId);
    } else {
        return AT_WAIT_IMSA_URSP_INPUT;
    }
}

VOS_VOID At_ProcUePolicyInfoStream(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length, AT_ParseContext *clientCxt)
{
    VOS_UINT32           result;
    AT_ModemUePolicyCtx *uePolicyCtx = VOS_NULL_PTR;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);
    result      = At_ProcUePolicyInfo(clientId, data, length);

    /* 处理成功，输出提示符">" */
    if (result == AT_WAIT_UE_POLICY_INPUT) {
        At_FormatResultData(clientId, result);
        return;
    }

    /* 清空缓存区 */
    PS_MEM_FREE(WUEPS_PID_AT, uePolicyCtx->uePolicyHead); /* 清空 */
    (VOS_VOID)memset_s(uePolicyCtx, sizeof(AT_ModemUePolicyCtx), 0x00, sizeof(AT_ModemUePolicyCtx));

    At_SetCmdSubMode(clientId, AT_NORMAL_MODE); /* 置回命令状态 */

    if (result == AT_WAIT_ASYNC_RETURN) {
        if (clientCxt->cmdElement != NULL) {
            if (At_StartTimer(clientCxt->cmdElement->setTimeOut, clientId) != AT_SUCCESS) {
                AT_ERR_LOG("At_ProcImsUrspStream():ERROR:Start Timer Failed");
            }

            clientCxt->clientStatus = AT_FW_CLIENT_STATUS_PEND;
        }
        return;
    }

    At_FormatResultData(clientId, result);
}

VOS_VOID At_ProcImsUrspStream(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length, AT_ParseContext *clientCxt)
{
    VOS_UINT32           result;
    AT_ModemUePolicyCtx *uePolicyCtx = VOS_NULL_PTR;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);

    result = At_ProcImsUrspInfo(clientId, data, length);

    /* 处理成功，输出提示符">" */
    if (result == AT_WAIT_IMSA_URSP_INPUT) {
        At_FormatResultData(clientId, result);
        return;
    }

    /* 清空缓存区 */
    PS_MEM_FREE(WUEPS_PID_AT, uePolicyCtx->imsUrspHead); /* 清空 */
    (VOS_VOID)memset_s(uePolicyCtx, sizeof(AT_ModemUePolicyCtx), 0x00, sizeof(AT_ModemUePolicyCtx));

    At_SetCmdSubMode(clientId, AT_NORMAL_MODE); /* 置回命令状态 */

    if (result == AT_WAIT_ASYNC_RETURN) {
        if (clientCxt->cmdElement != NULL) {
            if (At_StartTimer(clientCxt->cmdElement->setTimeOut, clientId) != AT_SUCCESS) {
                AT_ERR_LOG("At_ProcImsUrspStream():ERROR:Start Timer Failed");
            }

            clientCxt->clientStatus = AT_FW_CLIENT_STATUS_PEND;
        }
        return;
    }

    At_FormatResultData(clientId, result);
}

#endif


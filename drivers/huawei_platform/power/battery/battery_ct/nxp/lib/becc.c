/*
 * becc.c
 *
 * Binary ECC operations
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/string.h>

#include "becc.h"

#define BYTES_OF_WORD 32
#define BECC_LEN 6
#define BECC_STEP_SIZE 4

static const uint16_t lutsq[] = {
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
	0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
	0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
	0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
	0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
	0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
	0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
	0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
	0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
	0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
	0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
	0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
	0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
	0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
	0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
	0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
	0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

static const bitstr_t isotimesamatrix[] = {
	{ 0x73B65D25, 0x2F5BCA6D, 0x491A71F6,
	 0x9B5CED97, 0x3D8BD6F2, 0x0000034E },
	{ 0xDC7775D8, 0x9121A5C2, 0x0074DF3A,
	 0xB1A99783, 0x94612DE7, 0x00000174 },
	{ 0x01B05513, 0xCC6876AD, 0xEEF8E4ED,
	 0x8F39602E, 0x59752D21, 0x00000792 },
	{ 0xC03D5169, 0x5344C911, 0xCF144527,
	 0x108D8A60, 0x77B3308A, 0x000006F9 },
	{ 0x8E8A8FDE, 0xE927D441, 0x73DFF824,
	 0xBBD4CE07, 0x7E90D109, 0x000007C8 },
	{ 0x8BB78B90, 0xE9625E0A, 0xDD4EF3D3,
	 0xEC0CD814, 0xA9393D60, 0x00000079 },
	{ 0xD40E1BE1, 0x0F6918C6, 0x1B02C91D,
	 0x6BE2D5E9, 0x983BA1AA, 0x00000198 },
	{ 0x68E0EC2B, 0xF11B9B2C, 0x6ADBDDBF,
	 0x57C4413D, 0x95B59097, 0x00000777 },
	{ 0x03F7A0FF, 0x321E5F90, 0xB184D27E,
	 0xE6CCD125, 0x57688144, 0x000003FD },
	{ 0xD2831B2B, 0xD17D770E, 0x3A70D418,
	 0xBFCBA2A4, 0x54658D77, 0x000006D0 },
	{ 0x670E6E73, 0x5F0E2954, 0x43955684,
	 0x2EBAAE6F, 0xFF8F5922, 0x000000F6 },
	{ 0x65B6E9E2, 0xEBA9EB13, 0xD777F643,
	 0x1C5E4089, 0xE4AB7825, 0x00000369 },
	{ 0x8C2038A0, 0x90DA633E, 0xD099F0FF,
	 0xDA654ECA, 0x58EB03A1, 0x0000020E },
	{ 0x0B7CF3B3, 0xE3909553, 0x1206CCA7,
	 0xFE557D25, 0x8F2388EF, 0x000002D3 },
	{ 0xBB63CDCF, 0x03571531, 0xAF15446F,
	 0x32F96D36, 0x5B71C8B8, 0x000001EF },
	{ 0x79E73B38, 0x3D4A10AF, 0x22611A14,
	 0x857EE3D8, 0xA6C07501, 0x000001C7 },
	{ 0xD268B57C, 0x5F5E6420, 0x8967C9CF,
	 0x5BC04C13, 0x79869421, 0x0000003A },
	{ 0xA77A3553, 0xF7D604AE, 0x1A3AF4B3,
	 0xF48F0BAA, 0xB75195B9, 0x00000701 },
	{ 0x9754EC96, 0x08ACD19D, 0x33C79E2A,
	 0xB90D7956, 0xDB388CD4, 0x00000722 },
	{ 0xB41D11CE, 0xD6C35320, 0x3EA3CC30,
	 0x28BE4701, 0xB0C8D9A0, 0x00000506 },
	{ 0xDEA304EC, 0xE989DF83, 0x3905E1D8,
	 0x7CC0DD86, 0x93BC4887, 0x0000013B },
	{ 0xD7E7AC66, 0x9AB966EB, 0x827B4E18,
	 0x2638C155, 0x9B803CEE, 0x00000215 },
	{ 0x182E63E6, 0x2AD2110C, 0xCDA8D382,
	 0x6C32EA52, 0x553413C2, 0x00000597 },
	{ 0xBADAD79C, 0xAAD1C1B4, 0xBC77ACB9,
	 0x27394F01, 0x087B03BF, 0x00000140 },
	{ 0x598F640D, 0xC2261FCA, 0xA7E72E92,
	 0x5426A37B, 0x55085CFF, 0x000003E1 },
	{ 0xE29CBD50, 0x731494B7, 0x985A0AFF,
	 0xFF037BC7, 0x7AA49D31, 0x00000486 },
	{ 0xD32025C5, 0x7362FF25, 0x42E38B83,
	 0x1D57D46B, 0xDA52EF29, 0x0000023C },
	{ 0x58576FEC, 0x6BF5AB10, 0xC390216C,
	 0x23019A89, 0x2EFAA532, 0x0000018B },
	{ 0xE7D88AB5, 0xB9328F60, 0x4B671D8A,
	 0xBDC9C070, 0x9A9A1E08, 0x00000311 },
	{ 0x0734A55D, 0x006A55DB, 0x0A8398F8,
	 0xCCBEBC33, 0x17DA1620, 0x000003DC },
	{ 0x7D4590D3, 0xEFDE90AB, 0x010D22B6,
	 0xA2A29DFA, 0x09EC0B64, 0x0000042B },
	{ 0xE56A144F, 0xF5F1F50F, 0xBEC71F57,
	 0x368CDC69, 0x5C93E7DD, 0x000001A5 },
	{ 0x25AD5C62, 0x54420B01, 0xBB20602A,
	 0x425AE30C, 0xF049D2FC, 0x00000435 },
	{ 0x6FAFE069, 0xBF9CDD9C, 0xC6C199E2,
	 0x9C23B638, 0x75B31A0F, 0x000003B3 },
	{ 0x9C907894, 0x4EB458F9, 0xC8608209,
	 0xDD6E1C5A, 0x5F9060E5, 0x000005CE },
	{ 0xAFE59B41, 0x74D4312E, 0x500EB1B5,
	 0x616D02BF, 0xD15ACCBF, 0x00000042 },
	{ 0x7390A1EE, 0x4F33E877, 0x8331D7E4,
	 0x21F61793, 0x43F421A7, 0x0000003D },
	{ 0x7230F32A, 0x10D255B4, 0x59F65C74,
	 0x375D7CCB, 0xCC345C02, 0x0000013E },
	{ 0x5161AB9D, 0xEF8646F1, 0x893B7E59,
	 0xFC8F9039, 0x52FAA360, 0x00000405 },
	{ 0xE2062D90, 0xD39F6EC7, 0x9098E100,
	 0xE8C4E6CA, 0x721AC800, 0x000004E6 },
	{ 0x47C0EC30, 0x541B2F28, 0xA7D9F20E,
	 0xAD998318, 0x5403F329, 0x00000231 },
	{ 0xF5B1C356, 0xF5DA79F6, 0x86AEEE0F,
	 0x72393D9A, 0x60F27E20, 0x000004BF },
	{ 0x267811C6, 0xF80E1572, 0x1495A241,
	 0x0794B693, 0x925C88C2, 0x00000767 },
	{ 0x18DC01CC, 0xC89550AE, 0x13771A46,
	 0xB07ABD44, 0xE903AD1F, 0x0000009B },
	{ 0x319F5F39, 0xE0DF9DC7, 0x67014101,
	 0xC1A21567, 0x5305E65C, 0x000004E8 },
	{ 0xD29F9454, 0xC77B997F, 0x84927866,
	 0x007B064A, 0x0E3453A7, 0x00000288 },
	{ 0x4478EB57, 0xFD665BA4, 0xA1CB4671,
	 0xE466E9AC, 0xEC70E52D, 0x0000071D },
	{ 0x39F47E32, 0xF45CCFC6, 0xA2F8CB64,
	 0x6AD3E1DB, 0x4385D993, 0x0000035A },
	{ 0xBCF893AB, 0xC7C8D1E6, 0x91D38CC1,
	 0xFA6242F4, 0x2CFEE251, 0x00000615 },
	{ 0x458EF42A, 0x06D20A7A, 0x294CE627,
	 0xC6449351, 0x22E2BB78, 0x000005E0 },
	{ 0x03E97853, 0x14B2FCBE, 0x4F20BB65,
	 0xBAA250A1, 0x086C51CD, 0x00000364 },
	{ 0x03A5EF0F, 0xFF2AD19D, 0xB21B5A0F,
	 0xA49A776D, 0x2961E368, 0x000004BC },
	{ 0x9DB704CE, 0x76FF7746, 0xB4E019F7,
	 0xCB134F02, 0xE245E10E, 0x000004A7 },
	{ 0xEA222BE3, 0x2C98F8D2, 0x95C279BD,
	 0x6EE663F4, 0x0C9C0546, 0x0000025A },
	{ 0x493B10E9, 0x512D3DC9, 0xBF54AFE1,
	 0xA786E026, 0x2C81B68A, 0x00000100 },
	{ 0x50A2ED2D, 0xD9108710, 0x4D1C5D01,
	 0xFF43B6AE, 0x7AE729B6, 0x0000071E },
	{ 0xA1024CE7, 0x5D3E1006, 0x19DFC3AF,
	 0x27117E3C, 0x2353E30E, 0x000004B7 },
	{ 0x02DFCBC3, 0x0E587C0A, 0xE76409BA,
	 0xDC65EE2C, 0xD7A3D6AA, 0x000000C8 },
	{ 0xB5F518CD, 0xFBCDAE05, 0xE8193C48,
	 0xDEAAF3B8, 0x69C2F494, 0x0000008F },
	{ 0x2E8CFCC9, 0xBD332671, 0x7D5B2833,
	 0x44E30B21, 0x8EB430D6, 0x0000003A },
	{ 0xBDD05160, 0x919C3A7A, 0xF708DD9D,
	 0xED1FC309, 0x0E846B1F, 0x000002F1 },
	{ 0x395C62F8, 0xB5852235, 0x97A11B87,
	 0x3BC8A592, 0xBD90E5F9, 0x00000765 },
	{ 0xC83DEECA, 0x1A0A2F16, 0x344E051C,
	 0xF3750319, 0x0B5F994E, 0x000002B7 },
	{ 0x8BB170B0, 0xAD486E54, 0x8D38CB3F,
	 0x372F5290, 0xE8A2C299, 0x0000076B },
	{ 0x0B19EDA0, 0x298A91F1, 0x38396993,
	 0x7FCF5888, 0x36AA80CD, 0x000004AA },
	{ 0x4741832A, 0x547CC772, 0xB30CCDA0,
	 0x9DD1D61B, 0x465BDB72, 0x000004D3 },
	{ 0xB90F178F, 0xD1F8F3C4, 0x183998D5,
	 0x8E988DAB, 0x4BE9775D, 0x000000AB },
	{ 0x2254CBC4, 0x729B4B8E, 0xB664D562,
	 0x9218A3DE, 0x817F76FF, 0x00000410 },
	{ 0x7CCA5D12, 0x9B847A57, 0xF1ED7FF2,
	 0x855810C0, 0xC00CA61F, 0x0000048A },
	{ 0x5C7981C7, 0x2A9DAB1B, 0xB85B3255,
	 0xE6D4E460, 0x46386E47, 0x00000094 },
	{ 0xFAA36051, 0xDE710598, 0x563F9076,
	 0xB48E3976, 0xED2B6D95, 0x0000027C },
	{ 0xB296B55D, 0x3132F800, 0xBA1BD2FB,
	 0x5D37CD63, 0x2D5FDE63, 0x00000126 },
	{ 0x31B950F5, 0xB2D473AD, 0xB8E1BD7F,
	 0x46192E70, 0x53175E13, 0x00000721 },
	{ 0xE9489D22, 0x66C29571, 0x8610CDF5,
	 0xFDC2B214, 0x985952A3, 0x00000337 },
	{ 0x58B73D3E, 0x3A72FF5A, 0x7D1FB933,
	 0x7EBCED87, 0x9BF2B7DB, 0x00000534 },
	{ 0x035A4B29, 0x463BA5B6, 0x4F157736,
	 0x38DFA72A, 0x142FED9A, 0x00000439 },
	{ 0xB9290D74, 0x1BB82505, 0xCB0DF659,
	 0x52918189, 0x15DCB7C9, 0x000007E5 },
	{ 0x7BA46909, 0xADD6E1DB, 0x5E679C2B,
	 0x13E84E34, 0x0587A14A, 0x00000533 },
	{ 0x0D9D3148, 0x021A979A, 0x6742CA0F,
	 0xFE2AAAD7, 0xD421A615, 0x0000034A },
	{ 0xC3AB9E2D, 0x51BFCD90, 0xF8EF04D7,
	 0x82EEA57C, 0xABFEA7A6, 0x000000E8 },
	{ 0x7C5DD2AF, 0x3D3C41B0, 0x8F0865AC,
	 0x5BFECDEF, 0xFF04A95F, 0x000001E3 },
	{ 0x8B7B666C, 0x9BF85604, 0x97E48D2D,
	 0x9AA16FD4, 0x17872CB3, 0x00000497 },
	{ 0x2D8906C8, 0xB864FB29, 0xF6460284,
	 0xA8FC71DB, 0xFE5EA464, 0x0000067C },
	{ 0xA2890DCD, 0xE308E0AD, 0xDE1BA884,
	 0xBF64EEB3, 0xBA1BB1E5, 0x0000076C },
	{ 0x30F1325C, 0x3209F6CC, 0x7D8985B9,
	 0xF830EB52, 0xE6AC9FCE, 0x00000133 },
	{ 0xDB957DB9, 0xC27169C0, 0x39276833,
	 0x4F4E54FF, 0xD100DBD9, 0x00000577 },
	{ 0x020434A3, 0xE88DAF33, 0xBAF1D896,
	 0x239753F1, 0x83C6988E, 0x00000517 },
	{ 0xBCD68815, 0xE5E0F8AD, 0xFB83D50A,
	 0x37061C3F, 0xC325EDBD, 0x000002CE },
	{ 0x93E6D3F5, 0xD1C17272, 0xC7AEFD2D,
	 0x66261A37, 0x1A832E94, 0x00000281 },
	{ 0x5DE19C40, 0x9925EF43, 0xC9DB5AE6,
	 0x4522B611, 0x8E800203, 0x0000016E },
	{ 0x4C88021D, 0xB9E44383, 0xA5E5D613,
	 0xCA17806E, 0xB5401F2E, 0x000003AC },
	{ 0x4F784FDF, 0x0F7EF6AB, 0xFFA1B041,
	 0x92F95BEA, 0x6E3C588C, 0x000001A4 },
	{ 0xCB4DC52E, 0x93CBC938, 0x595AC8C7,
	 0x03AC8813, 0xB09E1AE3, 0x000002C7 },
	{ 0xB5F3B338, 0x5D16E046, 0xE661A7DA,
	 0xDB147F06, 0x2AE28747, 0x000000D6 },
	{ 0x7DEDD4CF, 0x34AFC53F, 0x5E83AA5B,
	 0x2C9E32A2, 0x50653774, 0x000007EA },
	{ 0x5195C3B7, 0x8DECA63B, 0x965F37D3,
	 0x3B107EE5, 0xFB200F05, 0x000006A0 },
	{ 0xB659306B, 0xC4C3519A, 0xA636F702,
	 0x9E5EE67F, 0x68DAFFAB, 0x0000030D },
	{ 0xD477118A, 0x8CBFAF7F, 0xD19C7BDF,
	 0x71403F4F, 0x12D0E04B, 0x00000271 },
	{ 0x2134C923, 0x461964F2, 0xA17D4E17,
	 0x0295D03B, 0xD694204A, 0x000002D0 },
	{ 0xF8AFB126, 0x5CC7C776, 0xB4B7D263,
	 0x43664CD8, 0xF28AAC09, 0x0000069A },
	{ 0x81320AA4, 0x94977409, 0x43FE0599,
	 0xAF9A770B, 0x4D6F3F76, 0x00000620 },
	{ 0x77EC5CEE, 0x36C3D574, 0x842600F9,
	 0x3842B9C6, 0xB69971C4, 0x00000225 },
	{ 0xCA7CF4D0, 0x6DD7E2F2, 0x5374E209,
	 0xA1D63C01, 0xB9640E6C, 0x000003EF },
	{ 0xEF94A19A, 0xDF03AD89, 0x57503F35,
	 0xBB167ACE, 0x6EE832D0, 0x000006DA },
	{ 0x1089A65F, 0x25E61BB1, 0x39E29A64,
	 0x3AE97C71, 0x2169F98E, 0x0000060A },
	{ 0x43225DC5, 0xA318C7CD, 0x19DA9E45,
	 0x3AA8F568, 0xCB0FBCE4, 0x0000023E },
	{ 0xE71D763E, 0xC78284FE, 0x81CA554A,
	 0xDFF07DDA, 0x9D63ECFD, 0x00000006 },
	{ 0xC79F06A1, 0x829880CD, 0x1BBD51E6,
	 0xBC8E5CFB, 0xF2F0891D, 0x00000313 },
	{ 0xC389BBF7, 0x12CB8115, 0xAFA23F68,
	 0xB388B2E9, 0x66757449, 0x00000332 },
	{ 0x27B71C60, 0x421CFD1C, 0x5A493817,
	 0x0C685141, 0x917ABC8D, 0x0000001B },
	{ 0x8AC60A30, 0x10FCF3C8, 0x475C33F7,
	 0xACCC7A1F, 0x9979054B, 0x000001D8 },
	{ 0x7EE88134, 0xE37EF07C, 0xE76F0D34,
	 0x22D79A0B, 0x95CE16EA, 0x00000079 },
	{ 0xAB55283A, 0x2136AE81, 0xCADF8964,
	 0x7C10249A, 0x528568EE, 0x0000035F },
	{ 0x26945107, 0xDD0A02EC, 0x27B00C4F,
	 0x0EA89461, 0xC13B2694, 0x00000528 },
	{ 0x9E4A83B3, 0xD26E8AC0, 0x1E2EC7C6,
	 0x1A48D6BB, 0x2AC1E758, 0x00000638 },
	{ 0x50E8653C, 0xDECAF735, 0x8E8BBFE8,
	 0x6670D198, 0xCFF52D08, 0x0000019C },
	{ 0xF94E8D41, 0x8DCEE1F8, 0x65E96E99,
	 0x6222C827, 0x8D783A58, 0x00000341 },
	{ 0x36879CE7, 0x0B342046, 0xA149B18B,
	 0x11B290E8, 0x52F82738, 0x00000414 },
	{ 0x9062FF63, 0x81B008A4, 0x6A3DDF8B,
	 0x9E43983E, 0x0C488D2C, 0x000003CD },
	{ 0x362509AD, 0xEAFCE872, 0x0E341FA8,
	 0x6617AC39, 0x540EE683, 0x00000681 },
	{ 0x593BA2E5, 0x3AFA51F4, 0x21432620,
	 0xA9C3F4E2, 0x5BD3C2C6, 0x000000C0 },
	{ 0x01C158BE, 0x39985461, 0x63EE3ABB,
	 0xE96570D7, 0x092D6112, 0x00000792 },
	{ 0x758C23F9, 0x1958D629, 0xCE3903EE,
	 0x636A1C63, 0x8B47C5FA, 0x00000409 },
	{ 0x0284FDFF, 0x9AC230B8, 0x35450772,
	 0x915C34F3, 0x1AA7CA13, 0x00000318 },
	{ 0x6D21B307, 0xB77505B4, 0x53A77172,
	 0xE6AC1128, 0x0019204B, 0x0000005E },
	{ 0x02ACDB3E, 0xABC33E2F, 0x3BEE289B,
	 0xD093AC58, 0xC39381F7, 0x00000375 },
	{ 0x710C1F24, 0xDEA231C9, 0x3F623AA5,
	 0x7D0622D1, 0xD0C7203A, 0x000005E8 },
	{ 0xA0699E35, 0x4947DC3F, 0x47ABC726,
	 0x627178A6, 0xAC20A090, 0x0000037F },
	{ 0x1815AC00, 0x4EA32C5D, 0x5A19FFBB,
	 0x3542075D, 0x146B4D1E, 0x000005CB },
	{ 0x59E1DC9B, 0x85C6C08A, 0x37F409EB,
	 0x932330E9, 0x1A699BF8, 0x00000410 },
	{ 0x79E8F5A1, 0x613A2295, 0x8BE1467E,
	 0x727698FC, 0x52F9C81E, 0x000002F2 },
	{ 0x6E1AFBA1, 0xCD2575C8, 0x9CBB70D2,
	 0x9902D767, 0xBD9102E2, 0x0000039E },
	{ 0xDDF47D40, 0x290C1A38, 0x47621FD8,
	 0xCA656C95, 0x43E0FBA0, 0x000005C3 },
	{ 0xE9F0B47D, 0xAE24D488, 0xA337D082,
	 0xCACD1773, 0xC188E373, 0x00000760 },
	{ 0x6F98E786, 0x2D0CD1BB, 0x4D808544,
	 0xFD96C900, 0xDB11302C, 0x000004BA },
	{ 0x850E715F, 0x8E5AAAF4, 0x6D1D0DB3,
	 0x92A83DB2, 0xDF82BC42, 0x000000F5 },
	{ 0x110BE53A, 0x7D14204E, 0x29551CA7,
	 0x5A51C010, 0xB7DFD99E, 0x00000653 },
	{ 0x588A2E76, 0x088E6815, 0x505497DA,
	 0xB17E04C4, 0x4485E777, 0x0000010E },
	{ 0xC549EC43, 0x724A8757, 0x14C0964F,
	 0x6AE24E72, 0x8DA6B109, 0x000001CD },
	{ 0xE2D500D1, 0x7777C3FA, 0x70989F0A,
	 0x8D5FDAB2, 0x286B45DA, 0x00000416 },
	{ 0xB2C727D0, 0x4A94B876, 0x93F4570C,
	 0x2DC983C0, 0x61221C33, 0x000007DF },
	{ 0x3CAD4AA8, 0xB7BDF93B, 0xBCE7A73E,
	 0xEBA61390, 0xFA0B02E7, 0x00000717 },
	{ 0xD4ECE885, 0xE3BA4F67, 0xAA9C5F21,
	 0x8028B362, 0xFB0E570E, 0x000001C5 },
	{ 0x40C38CCA, 0x01AD8E52, 0x2F040C49,
	 0x2C72B5C6, 0x595D5344, 0x00000246 },
	{ 0xB01DEC4F, 0x2920DEC6, 0x72C91144,
	 0x6FC09A78, 0xEDF390BE, 0x000006A9 },
	{ 0x348C0C22, 0x64AEA4AD, 0x99CFA844,
	 0x6CFC3E8F, 0x59F569EB, 0x0000032B },
	{ 0x72181780, 0x51DCF180, 0x13FF5E2E,
	 0x5B4E0080, 0xD853F6D2, 0x00000380 },
	{ 0x2055290F, 0x2EEF74BD, 0x2C7B3187,
	 0x926F8D28, 0xC040EB62, 0x0000046B },
	{ 0x52F7EA61, 0xBE5A8E78, 0x944C769D,
	 0x637E6CFB, 0xCFE38BC4, 0x000001FF },
	{ 0x2FAFA72F, 0x1D51984A, 0x5D38D1D9,
	 0xBF786FF8, 0xF0A55A9D, 0x00000328 },
	{ 0x9C925AB0, 0x2FEC92F4, 0x5946514E,
	 0x60DE634E, 0x9D1F219F, 0x0000011B },
	{ 0x74CEC280, 0x532FCC9A, 0xB77CC920,
	 0xAE33EB13, 0x1A015E5C, 0x00000363 },
	{ 0x592B58A5, 0x31780578, 0x52A04DCC,
	 0x250DA9BC, 0xDFBC1436, 0x00000079 },
	{ 0xB56C94D9, 0xF8E8307B, 0x561FA6DF,
	 0x971691A3, 0x968E7600, 0x0000073F },
	{ 0xF6566EC2, 0x98424E81, 0xD95AD13E,
	 0x073B754E, 0xC1F8288E, 0x000006B7 },
	{ 0x1A0834C7, 0xDC6F92DB, 0xAB2C0171,
	 0xDD93AB7F, 0x4DCBD500, 0x0000038B },
	{ 0x23A2D6F6, 0x989D98CB, 0xCBCE4641,
	 0xB31AE54D, 0xD2648333, 0x000000AE },
	{ 0xC87E70E8, 0xDD761B0F, 0x9333DD1B,
	 0xFCEA4D73, 0x289A8225, 0x00000486 },
	{ 0xBE0AE84E, 0x8ECD8222, 0x0562464A,
	 0x09B85E02, 0x2A18FB08, 0x0000067F },
	{ 0x18F20FA8, 0x73A9C81E, 0x3170E08D,
	 0xBD176F8F, 0xC26DA6D7, 0x000001F5 },
	{ 0xDBBC0AB4, 0xEBDF4E2E, 0x94B0D5EB,
	 0xA48E5290, 0x3C31C40F, 0x000000E0 },
	{ 0xCCAF4E55, 0xA52301AB, 0x562A77C6,
	 0xA9F7A821, 0x7328E64F, 0x00000666 },
	{ 0x760932AA, 0x81729EFA, 0x36F59608,
	 0x0E073090, 0x48FB00C9, 0x00000430 }
};

/* bitstr_parse(kx, "34e3d8bd6f29b5ced97491a71f62f5bca6d73b65d24"); */
/* 0x0000034E,0x3D8BD6F2,0x9B5CED97,0x491A71F6,0x2F5BCA6D,0x73B65D24 */
static const bitstr_t epif_kx = {
	0x73B65D24, 0x2F5BCA6D, 0x491A71F6,
	0x9B5CED97, 0x3D8BD6F2, 0x0000034E
};

/* ecc_poly: 00000008,00000000,00000000,00000000,00000000,000000C9 */
/*         : t^163 + t^7 + t^6 + t^3 + t^0 */
static const bitstr_t ecc_poly = {
	0x000000C9, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000008
};

/* some basic bit-manipulation routines that act on these vectors follow */
#define WORD_GETBIT(x, idx)	(((x) >> (idx)) & 1)
#define BITSTR_GETBIT(x, idx)	((x[(idx) / BYTES_OF_WORD] >>	\
				 ((idx) % BYTES_OF_WORD)) & 1)
#define BITSTR_SETBIT(x, idx)	(x[(idx) / BYTES_OF_WORD] |=	\
				 1 << ((idx) % BYTES_OF_WORD))
#define BITSTR_CLRBIT(x, idx)	(x[(idx) / BYTES_OF_WORD] &=	\
				 ~(1 << ((idx) % BYTES_OF_WORD)))

#define BITSTR_CLEAR(x)		memset((x), 0, sizeof(bitstr_t))

#define BITSTR_SWAP(x, y)	\
do {				\
	bitstr_t h;		\
	BECC_BITSTR_COPY(h, x);	\
	BECC_BITSTR_COPY(x, y);	\
	BECC_BITSTR_COPY(y, h);	\
} while (0)

#define BITSTR_IS_EQUAL(x, y)	(!memcmp(x, y, sizeof(bitstr_t)))
#define FIELD_ADD1(x)		(x[0] ^= 1)

#define FIELD_SET1(x)						\
do {								\
	x[0] = 1;						\
	memset(x + 1, 0, sizeof(bitstr_t) - BECC_STEP_SIZE);	\
} while (0)

#define POINT_IS_ZERO(x, y)	(bitstr_is_clear(x) && bitstr_is_clear(y))
#define POINT_SET_ZERO(x, y)	\
do {				\
	BITSTR_CLEAR(x);	\
	BITSTR_CLEAR(y);	\
} while (0)

/* return the number of the highest one-bit + 1 */
static uint32_t bitstr_sizeinbits(const bitstr_t x);
/* left-shift by 'count' digits */
static void bitstr_lshift(bitstr_t x, const bitstr_t y, uint32_t count);
static uint32_t field_is1(const bitstr_t x);
/* check if y^2 + x*y = x^3 + *x^2 + coeff_b holds */
static void lambda_add(bitstr_t x3, bitstr_t l3, bitstr_t z3,
		const bitstr_t x1, const bitstr_t l1, const bitstr_t z1);
static void lambda_double(bitstr_t x, bitstr_t l, bitstr_t z);

/* return the number of the highest one-bit + 1 */
static uint32_t bitstr_sizeinbits(const bitstr_t x)
{
	uint32_t i;
	uint32_t mask;

	for (x += NUMWORDS, i = BYTES_OF_WORD * NUMWORDS; i > 0 && !*--x;
	     i -= BYTES_OF_WORD)
		;
	if (i) {
		for (mask = 1UL << (BYTES_OF_WORD - 1); !(*x & mask);
		     mask >>= 1, i--)
			;
	}
	return i;
}

/* left-shift by 'count' digits */
static void bitstr_lshift(bitstr_t x, const bitstr_t y, uint32_t count)
{
	uint32_t i;
	uint32_t offs = BECC_STEP_SIZE * (count / BYTES_OF_WORD);

	memmove((char *)x + offs, y, sizeof(bitstr_t) - offs);
	memset(x, 0, offs);
	count %= BYTES_OF_WORD;
	if (count) {
		for (i = NUMWORDS - 1; i > 0; i--)
			x[i] = (x[i] << count) |
				(x[i - 1] >> (BYTES_OF_WORD - count));
		x[0] <<= count;
	}
}

static uint32_t field_is1(const bitstr_t x)
{
	uint32_t i;

	if (*x++ != 1)
		return 0;
	for (i = 1; i < NUMWORDS && !*x++; i++)
		;
	return i == NUMWORDS;
}

/* nist Curve: B-163 */
static void red_nistb163(bitstr_t z, uint32_t *x)
{
	uint32_t i;
	uint32_t t;

	for (i = 10; i >= BECC_LEN; i--) {
		t = x[i];
		x[i - 6] = x[i - 6] ^ (t << 29);
		x[i - 5] = x[i - 5] ^ (t << 4) ^ (t << 3) ^ t ^ (t >> 3);
		x[i - 4] = x[i - 4] ^ (t >> 28) ^ (t >> 29);
	}
	t = x[5] >> 3;
	z[0] = x[0] ^ (t << 7) ^ (t << 6) ^ (t << 3) ^ t;
	z[1] = x[1] ^ (t >> 25) ^ (t >> 26);
	z[5] = x[5] & (uint32_t)0x7;
	z[2] = x[2];
	z[3] = x[3];
	z[4] = x[4];
}

static void red_epif(bitstr_t z, uint32_t *x)
{
	uint32_t i, j;

	for (i = 342; i >= 171; i--) {
		if (((x[i >> 5]) & (1 << (i & 31)))) {
			j = i - 101;
			(x[j >> 5]) ^= (1 << (j & 31));
			j = i - 171;
			(x[j >> 5]) ^= (1 << (j & 31));
		}
	}
	z[0] = x[0];
	z[1] = x[1];
	z[2] = x[2];
	z[3] = x[3];
	z[4] = x[4];
	z[5] = x[5] & 0x7FF;
}

/* Implementation of the point addition and point doubling
 * of elliptic curve points represented in Lambda-coordinates.
 * See:
 * Thomaz Oliveira, Julio Lopez, Diego F. Aranha, Francisco Rodroguez-Henroquez
 * Two is the fastest prime: lambda coordinates for binary elliptic curves.
 * J. Cryptographic Engineering (JCE) 4(1):3-17 (2014)
 * https://eprint.iacr.org/2013/131.pdf
 *
 * We work on an elliptic curve
 * E : y^2+x*y=x^3+a2*x^2+a6
 * defined over a finite field of 2^n elements.
 * Here we assume a2=1.
 * A projective point in Lambda coordinates(X,L,Z) represents
 * an affine point (x,y) as
 * x = X/Z
 * y/x = (L-X)/Z
 * --> y = X(L-X)/Z^2
 */
/* add two points together (x3, l3, z3) := (x3, l3, z3) + (x1, l1, z1) */
static void lambda_add(bitstr_t x3, bitstr_t l3, bitstr_t z3,
		const bitstr_t x1, const bitstr_t l1, const bitstr_t z1)
{
	bitstr_t la, lb, lc, ld;

	becc_field_mult(la, l1, z3, false);
	becc_field_mult(lb, l3, z1, false);
	becc_field_add(la, la, lb);
	becc_field_mult(lb, x1, z3, false);
	becc_field_mult(x3, x3, z1, false);
	becc_field_add(lc, lb, x3);
	becc_field_square(lc, lc);
	becc_field_mult(lb, la, lb, false);
	becc_field_mult(ld, la, x3, false);
	becc_field_mult(x3, la, lc, false);
	becc_field_mult(la, x3, z3, false);
	becc_field_mult(x3, lb, ld, false);
	becc_field_add(lb, ld, lc);
	becc_field_add(l3, l1, z1);
	becc_field_square(lc, lb);
	becc_field_mult(ld, la, l3, false);
	becc_field_add(l3, lc, ld);
	becc_field_mult(z3, la, z1, false);
}

static void lambda_double(bitstr_t x, bitstr_t l, bitstr_t z)
{
	bitstr_t la, lb, lc;

	becc_field_square(la, z);
	becc_field_square(lb, l);
	becc_field_mult(l, l, z, false);
	becc_field_add(lc, lb, l);
	becc_field_add(lb, lc, la);
	becc_field_mult(lc, x, z, false);
	becc_field_square(x, lb);
	becc_field_mult(z, lb, la, false);
	becc_field_square(la, lc);
	becc_field_mult(lb, lb, l, false);
	becc_field_add(la, la, x);
	becc_field_add(la, la, lb);
	becc_field_add(l, la, z);
}

void becc_bin2elem(bitstr_t e, const uint8_t *buf, const uint32_t bufsize)
{
	uint32_t j;
	uint32_t x;
	const uint8_t *p = buf + bufsize - 1;
	const uint8_t *q = buf - 1;

	if (!e || !buf)
		return;
	for (j = 0; j < BECC_LEN; j++) {
		x = 0;
		if (p != q)
			x = ((uint32_t)*(p--));
		if (p != q)
			x |= (((uint32_t)*(p--)) << 8);
		if (p != q)
			x |= (((uint32_t)*(p--)) << 16);
		if (p != q)
			x |= (((uint32_t)*(p--)) << 24);
		e[j] = x;
	}
}

void becc_elem2bin(uint8_t *buf, const uint32_t bufsize, const bitstr_t e)
{
	uint32_t j;
	uint32_t x;
	uint8_t *d = buf + bufsize - 1;
	const uint8_t *q = buf - 1;

	if (!buf || !e)
		return;
	for (j = 0; j < BECC_LEN; j++) {
		x = e[j];
		if (d != q)
			*(d--) = ((uint8_t)(x & 0xFF));
		x >>= 8;
		if (d != q)
			*(d--) = ((uint8_t)(x & 0xFF));
		x >>= 8;
		if (d != q)
			*(d--) = ((uint8_t)(x & 0xFF));
		x >>= 8;
		if (d != q)
			*(d--) = ((uint8_t)(x & 0xFF));
	}
	while (d != q)
		*(d--) = 0x00;
}

void becc_to_epif(bitstr_t destination, const bitstr_t x_value)
{
	bitstr_t la;
	uint32_t i = 0;

	if (!destination || !x_value)
		return;
	BITSTR_CLEAR(la);
	for (i = 0; i < DEGREE; i++) {
		if (BITSTR_GETBIT(x_value, i))
			becc_field_add(la, la, isotimesamatrix[i]);
	}
	becc_field_add(la, la, epif_kx);
	BECC_BITSTR_COPY(destination, la);
}

void becc_point_mult(bitstr_t x, bitstr_t y, const bitstr_t exp)
{
	bitstr_t a, l, b, z;
	int32_t i;

	if (!x || !y || !exp)
		return;
	/* Convert to Lambda coordinates */
	BECC_BITSTR_COPY(a, x);
	/* l = y/x+x */
	becc_field_invert(l, x);
	becc_field_mult(l, l, y, false);
	becc_field_add(l, l, x);
	FIELD_SET1(b);
	BECC_BITSTR_COPY(y, l);
	BECC_BITSTR_COPY(z, b);
	for (i = bitstr_sizeinbits(exp) - 2; i >= 0; i--) {
		lambda_double(a, l, b);
		if (BITSTR_GETBIT(exp, i))
			lambda_add(a, l, b, x, y, z);
	}
	/* (a/b, a(l-a)/b^2) */
	becc_field_invert(z, b);
	becc_field_mult(x, a, z, false);
	becc_field_square(z, z);
	becc_field_add(l, l, a);
	becc_field_mult(a, a, l, false);
	becc_field_mult(y, z, a, false);
}

void becc_field_invert(bitstr_t z, const bitstr_t x)
{
	bitstr_t la, lb, lc, ld;
	int32_t i;

	if (!z || !x)
		return;
	BECC_BITSTR_COPY(la, x);
	BECC_BITSTR_COPY(lb, ecc_poly);
	BITSTR_CLEAR(lc);
	FIELD_SET1(z);
	while (!field_is1(la)) {
		i = bitstr_sizeinbits(la) - bitstr_sizeinbits(lb);
		if (i < 0) {
			BITSTR_SWAP(la, lb);
			BITSTR_SWAP(lc, z);
			i = -i;
		}
		bitstr_lshift(ld, lb, i);
		becc_field_add(la, la, ld);
		bitstr_lshift(ld, lc, i);
		becc_field_add(z, z, ld);
	}
}

/* Lopez-Dahab Binary Field Multiplication (Performance Optimized)
 * using right-to-left comb method with window width w=4 (Algorithm 5)
 * "High-Speed Software Multiplication in F(2^m)", IndoCrypt 2000
 * by Julio Lopez and Ricardo Dahab
 */
void becc_field_mult(bitstr_t z, const bitstr_t x, const bitstr_t y,
		     const bool epif)
{
	uint32_t c[11] = {0};
	uint32_t P16[16][BECC_LEN], r0, r1, r2, r3, u;
	uint32_t j, k, l;

	if (!z || !x || !y)
		return;
	/* each 4 bit nibble has 16 (0, 1, ..., 15) possible values,
	 * precompute array P16 = {y*0, y*1, y*2, y*3, ..., y*15},
	 * containing multiplication of y with each possible nibble
	 */
	for (u = 0, k = 0; k < BECC_LEN; k++) {
		r0 = y[k];
		r1 = (r0 << 1) | (u >> 31);
		r2 = (r0 << 2) | (u >> 30);
		r3 = (r0 << 3) | (u >> 29);
		P16[0][k] = 0;                  /* y * (0x0=0b0000=0) */
		P16[1][k] = r0;                 /* y * (0x1=0b0001=1) */
		P16[2][k] = r1;                 /* y * (0x2=0b0010=z) */
		P16[3][k] = r1 ^ r0;            /* y * (0x3=0b0011=z+1) */
		P16[4][k] = r2;                 /* y * (0x4=0b0100=z^2) */
		P16[5][k] = r2 ^ r0;            /* y * (0x5=0b0101=z^2+1) */
		P16[6][k] = r2 ^ r1;            /* y * (0x6=0b0110=z^2+z) */
		P16[7][k] = r2 ^ r1 ^ r0;       /* y * (0x7=0b0111=z^2+z+1) */
		P16[8][k] = r3;                 /* y * (0x8=0b1000=z^3) */
		P16[9][k] = r3 ^ r0;            /* y * (0x9=0b1001=z^3+1) */
		P16[10][k] = r3 ^ r1;           /* y * (0xA=0b1010=z^3+z) */
		P16[11][k] = r3 ^ r1 ^ r0;      /* y * (0xB=0b1011=z^3+z+1) */
		P16[12][k] = r3 ^ r2;           /* y * (0xC=0b1100=z^3+z^2) */
		P16[13][k] = r3 ^ r2 ^ r0;      /* y * (0xD=0b1101=z^3+z^2+1) */
		P16[14][k] = r3 ^ r2 ^ r1;      /* y * (0xE=0b1110=z^3+z^2+z) */
		/* y * (0xF=0b1111=z^3+z^2+z+1) */
		P16[15][k] = r3 ^ r2 ^ r1 ^ r0;
		u = r0;
	}
	/* decrement through each nibble of a 32 bit word */
	for (j = (BYTES_OF_WORD - BECC_STEP_SIZE); j >= BECC_STEP_SIZE;
	     j -= BECC_STEP_SIZE) {
		for (k = 0; k < BECC_LEN; k++) {
	/* take nibble value from each word of x and multiple with y */
			u = (x[k] >> j) & 15;
			for (l = 0; l < BECC_LEN; l++)
			/* add product result to c */
				(c[l + k]) ^= (P16[u][l]);
		}
	/* except for bottom nibble, left shift product in c by a nibble */
		for (l = 10; l > 0; l--)
			c[l] = (c[l] << BECC_STEP_SIZE) |
			       (c[l - 1] >> (BYTES_OF_WORD - BECC_STEP_SIZE));
		c[0] <<= BECC_STEP_SIZE;
	}
	/* do last bottom nibble (j = 0) */
	for (k = 0; k < BECC_LEN; k++) {
	/* take nibble from each word of x and multiple with y */
		u = (x[k]) & 15;
		for (l = 0; l < BECC_LEN; l++)
		/* add product result to c */
			(c[l + k]) ^= (P16[u][l]);
	}
	/* preform binary field reduction of product
	 * to get it down to 163 bit or 171 bit curve
	 */
	if (epif)
		red_epif(z, c);
	else
		red_nistb163(z, c);
}

void becc_field_square(bitstr_t z, const bitstr_t x)
{
	int32_t i;
	uint32_t c[11] = {0};

	if (!z || !x)
		return;
	for (i = 0; i < BECC_LEN; i++) {
		uint32_t b0 = (x[i] & 0xFF);
		uint32_t b1 = ((x[i] >> 8) & 0xFF);
		uint32_t b2 = ((x[i] >> 16) & 0xFF);
		uint32_t b3 = ((x[i] >> 24) & 0xFF);

		c[2 * i] = ((lutsq[b1] << 16UL) | lutsq[b0]);
		if (i != 5)
			c[2 * i + 1] = ((lutsq[b3] << 16UL) | lutsq[b2]);
	}
	red_nistb163(z, c);
}

/* field addition */
void becc_field_add(bitstr_t z, const bitstr_t x, const bitstr_t y)
{
	uint32_t i;

	if (!x || !y || !z)
		return;
	for (i = 0; i < NUMWORDS; i++)
		*(z++) = (*(x++) ^ *(y++));
}

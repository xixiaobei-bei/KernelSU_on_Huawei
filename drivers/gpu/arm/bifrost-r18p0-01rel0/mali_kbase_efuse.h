/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: gpu efuse configuration
 * Author : gpu
 * Create : 2019/12/20
 */

#ifndef _GPU_EFUSE_H
#define _GPU_EFUSE_H

#define GPU_FAILED_CORES_EFUSE_OFFSET 1627
#define GPU_FAILED_CORES_EFUSE_SIZE 16

#define LITE_CHIP_EFUSE_OFFSET 3438
#define LITE_CHIP_EFUSE_SIZE 1

#define MODEM_LITE_CHIP_EFUSE_OFFSET 3439
#define MODEM_LITE_CHIP_EFUSE_SIZE 1

#define EFUSE_STACK_MASK_BIT 0xFF
#define EFUSE_STACK_MASK_BIT_SHIFT 8

typedef struct{
	u32 efuse_fail_bit;
	u32 clip_core_mask;
	u32 clip_core_mask_plus;
} shader_present_struct;

// Fail core identified in efuse
#define EFUSE_BIT0_FAIL                0x1
#define EFUSE_BIT1_FAIL                0x2
#define EFUSE_BIT2_FAIL                0x4
#define EFUSE_BIT3_FAIL                0x8
#define EFUSE_BIT4_FAIL                0x10
#define EFUSE_BIT5_FAIL                0x20
#define EFUSE_BIT6_FAIL                0x40
#define EFUSE_BIT7_FAIL                0x80
#define EFUSE_BIT8_FAIL                0x100
#define EFUSE_BIT9_FAIL                0x200
#define EFUSE_BIT10_FAIL               0x400
#define EFUSE_BIT11_FAIL               0x800
#define EFUSE_BIT12_FAIL               0x1000
#define EFUSE_BIT13_FAIL               0x2000
#define EFUSE_BIT14_FAIL               0x4000
#define EFUSE_BIT15_FAIL               0x8000

// The location of core in the core_mask
#define CLIP_BIT0_CORE                  0xFFFE
#define CLIP_BIT1_CORE                  0xFFFD
#define CLIP_BIT2_CORE                  0xFFFB
#define CLIP_BIT3_CORE                  0xFFF7
#define CLIP_BIT4_CORE                  0xFFEF
#define CLIP_BIT5_CORE                  0xFFDF
#define CLIP_BIT6_CORE                  0xFFBF
#define CLIP_BIT7_CORE                  0xFF7F
#define CLIP_BIT8_CORE                  0xFEFF
#define CLIP_BIT9_CORE                  0xFDFF
#define CLIP_BIT10_CORE                 0xFBFF
#define CLIP_BIT11_CORE                 0xF7FF
#define CLIP_BIT12_CORE                 0xEFFF
#define CLIP_BIT13_CORE                 0xDFFF
#define CLIP_BIT14_CORE                 0xBFFF
#define CLIP_BIT15_CORE                 0x7FFF

#define CLIP_BIT0_CORE_PLUS             0xFFFC
#define CLIP_BIT1_CORE_PLUS             0xFFF9
#define CLIP_BIT2_CORE_PLUS             0xFFF9
#define CLIP_BIT3_CORE_PLUS             0xFFF3
#define CLIP_BIT4_CORE_PLUS             0xFFCF
#define CLIP_BIT5_CORE_PLUS             0xFF9F
#define CLIP_BIT6_CORE_PLUS             0xFF9F
#define CLIP_BIT7_CORE_PLUS             0xFF3F
#define CLIP_BIT8_CORE_PLUS             0xFCFF
#define CLIP_BIT9_CORE_PLUS             0xF9FF
#define CLIP_BIT10_CORE_PLUS            0xF9FF
#define CLIP_BIT11_CORE_PLUS            0xF3FF
#define CLIP_BIT12_CORE_PLUS            0xCFFF
#define CLIP_BIT13_CORE_PLUS            0x9FFF
#define CLIP_BIT14_CORE_PLUS            0x9FFF
#define CLIP_BIT15_CORE_PLUS            0x3FFF

#define LITE_CHIP_CORE_MASK             0x3FFF

int get_gpu_efuse_cfg(struct kbase_device * const kbdev, u32 *shader_present_mask, u32 *lite_chip_mask);


#endif

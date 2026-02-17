/*-----------------------------------------------------------------------
// Copyright (c) 2017, Hisilicon Tech. Co., Ltd. All rights reserved.
//
// This software and the associated documentation are confidential and
// proprietary to Synopsys, Inc. Your use or disclosure of this software
// is subject to the terms and conditions of a written license agreement
// between you, or your company, and Synopsys, Inc.
//
// The entire notice above must be reproduced on all authorized copies.
//
//-----------------------------------------------------------------------
//
// Project:
//
// CAL.
//
// Description:
//
//
//
//-----------------------------------------------------------------------*/

#ifndef _SYNOPSYS_STD_DEF_H_
#define _SYNOPSYS_STD_DEF_H_

#ifndef __KERNEL__

/**
 * \file
 * \ingroup SysDef
 * \brief Standard C Include Files
 * \copydoc sys_include_files
 */

/**
 * \defgroup sys_include_files Standard C Include Files
 * \ingroup SysDef
 * The System Component uses the following includes:
 \code
 */
#include <stddef.h>   /* C89 compliant - if this is not available then add your definition here */
#include <stdint.h>   /* C99 compliant - if this is not available then add your definition here */
#include <stdarg.h>   /* C89 compliant -  if this is not available then add your definition here */
/**
  \endcode
 *
 * If these definitions are not available, the elliptic_std_def.h file should be updated with
 * definitions for the following:
 * - int8_t
 * - int16_t
 * - int32_t
 * - uint8_t
 * - uint16_t
 * - uint32_t
 *
 * The library is designed to operate on a 32-bit system.
 */

#else
#include <linux/types.h>
#include <linux/kernel.h>
#endif

#endif


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

#ifndef _SYNOPSYS_SYSTEM_TYPES_H_
#define _SYNOPSYS_SYSTEM_TYPES_H_

#include "elliptic_std_def.h"

/**
 * \file
 * \brief Defines the System Macros.
 * \ingroup SysDef
 */

/**
 * \addtogroup SystemAPI
 * @{
 * \cond INCLUDE_SYSTEM_API_CAL
 */

/* System types definitions. */
#define ELP_STATUS                      int16_t   /**< \brief Macro for system API return value. */
/**
 * \endcond
 * @}
 */

/**
 * \addtogroup TranslationAPI
 * @{
 * \cond INCLUDE_DATA_TRANSLATION_API_CAL
 */
#define ELP_SYSTEM_INFO                 int16_t   /**< \brief Macro for system API return information. */
/**
 * \endcond
 * @}
 */

/**
 * @{
 * \cond NOT_DEFINED_IN_DOXYGEN
 */
#define ELP_SYSTEM_BIG_ENDIAN           0
#define ELP_SYSTEM_LITLLE_ENDIAN        1

/* PRNG definitions. */
#define PRINT_FUNC      printf_ptr
#define PRINT_FUNC_DECL int32_t (*PRINT_FUNC)(const void *str, ...)
#define PRNG_FUNC       prng
#define PRNG_FUNC2      prng2
#define PRNG_FUNC_DECL  uint8_t (* PRNG_FUNC)  (void *, void *, uint8_t)
#define PRNG_FUNC2_DECL uint32_t (* PRNG_FUNC2)(void *, void *, uint32_t)

typedef struct
{
   void *prng_inst;
   PRNG_FUNC_DECL;
} elp_std_prng_info;

typedef struct
{
   void *prng_inst;
   PRNG_FUNC2_DECL;
} elp_std_prng_info2;

#define ELP_STD_PRNG_INFO  elp_std_prng_info
#define ELP_STD_PRNG_INFO2 elp_std_prng_info2

/**
 * \endcond
 * @}
 */

#endif


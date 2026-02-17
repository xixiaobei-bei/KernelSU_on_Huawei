/*-----------------------------------------------------------------------
//Copyright (c) 2017, Hisilicon Tech. Co., Ltd. All rights reserved.
//
// This software and the associated documentation are confidential and
// proprietary to Synopsys, Inc. Your use or disclosure of this software
// is subject to the terms and conditions of a written license agreement
// between you, or your company, and Synopsys, Inc.
//
//  The entire notice above must be reproduced on all authorized copies.
//
//-----------------------------------------------------------------------
//
// Project:
//
// Host Library.
//
// Description:
//
// Driver interface error code definitions.
//
//-----------------------------------------------------------------------*/

#ifndef _HOSTLIB_DRIVER_ERRORS_H_
#define _HOSTLIB_DRIVER_ERRORS_H_

/**
 * \file
 * \ingroup HL_Driver
 * \brief Error codes for the Host Library Driver
 */

/**
 * \defgroup HostLibErrors General Library Errors
 * \ingroup HL_Driver
 *
 * The following are error code definitions produced
 * by the API library.
 *
 * @{
 *
 */

#define HL_DRIVER_SUCCESS                0
#define HL_DRIVER_FAILED               (-1)
#define HL_DRIVER_NO_MEMORY            (-2)
#define HL_DRIVER_NO_ACCESS            (-3)
#define HL_DRIVER_INVALID_PARAM        (-4)
#define HL_DRIVER_TOO_MANY_ESM_DEVICES (-5)
#define HL_DRIVER_USER_DEFINED_ERROR   (-6) // anything beyond this error code is user defined

// End of APIErrors group
/**
* @}
*/

/**
* @}
*/

#endif

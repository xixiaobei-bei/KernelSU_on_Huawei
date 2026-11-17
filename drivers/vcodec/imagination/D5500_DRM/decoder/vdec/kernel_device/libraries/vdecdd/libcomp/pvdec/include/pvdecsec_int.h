/*!
 *****************************************************************************
 *
 * @File       pvdecsec_int.h
 * @Title      Low-level PVDECSEC interface component
 * @Description    This file contains the interface to communicate with an MSVDX core.
 * ---------------------------------------------------------------------------
 *
 * Copyright (c) Imagination Technologies Ltd.
 * 
 * The contents of this file are subject to the MIT license as set out below.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * Alternatively, the contents of this file may be used under the terms of the 
 * GNU General Public License Version 2 ("GPL")in which case the provisions of
 * GPL are applicable instead of those above. 
 * 
 * If you wish to allow use of your version of this file only under the terms 
 * of GPL, and not to allow others to use your version of this file under the 
 * terms of the MIT license, indicate your decision by deleting the provisions 
 * above and replace them with the notice and other provisions required by GPL 
 * as set out in the file called "GPLHEADER" included in this distribution. If 
 * you do not delete the provisions above, a recipient may use your version of 
 * this file under the terms of either the MIT license or GPL.
 * 
 * This License is also included in this distribution in the file called 
 * "MIT_COPYING".
 *
 *****************************************************************************/

#if !defined (__PVDECSECINT_H__)
#define __PVDECSECINT_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "img_defs.h"
#include "hwctrl_int.h"
#include "vxd.h"

/*!
******************************************************************************

 @Function              PVDECSEC_Create

******************************************************************************/
extern IMG_RESULT
PVDECSEC_Create(
    const VDECDD_sDdDevConfig * psDdDevConfig,
    VXD_sCoreProps            * psCoreProps,
    HWCTRL_sRendec            * psRendec,
    IMG_HANDLE                * phPvdecContext,
    HWCTRL_sFunctions         * psVxdFuncs
);

/*!
******************************************************************************

 @Function              PVDECSEC_Initialise

******************************************************************************/
extern IMG_RESULT
PVDECSEC_Initialise(
    const IMG_HANDLE        hVxd,
    VDECFW_sCoreInitData  * psInitConfig,
    const IMG_BOOL          bSecureStream
);

/*!
******************************************************************************

 @Function              PVDECSEC_DeInitialise

******************************************************************************/
extern IMG_RESULT
PVDECSEC_DeInitialise(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              PVDECSEC_FlushMmuCache

******************************************************************************/
extern IMG_RESULT
PVDECSEC_FlushMmuCache(
    const IMG_HANDLE  hVxd
);

#if defined(__cplusplus)
}
#endif

#endif /* __PVDECSECINT_H__ */

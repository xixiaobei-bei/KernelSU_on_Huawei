/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 1998-2014. All rights reserved.
* File name: securec.h
* Decription:
*             the user of this secure c library should include this header file
*             in you source code. This header file declare all supported API
*             prototype of the library, such as memcpy_s, strcpy_s, wcscpy_s,
*             strcat_s, strncat_s, sprintf_s, scanf_s, and so on.
* History:
*     1. Date:
*         Author:
*         Modification:
********************************************************************************
*/

#ifndef __SECUREC_H__5D13A042_DC3F_4ED9_A8D1_882811274C27
#define __SECUREC_H__5D13A042_DC3F_4ED9_A8D1_882811274C27

/* If you need high performance, enable the WITH_PERFORMANCE_ADDONS macro! */
#define WITH_PERFORMANCE_ADDONS

#include "securectype.h" /*lint !e537*/
#include <stdarg.h>

/* If stack size on some embedded platform is limited, you can define the following macro
*  which will put some variables on heap instead of stack.
#define STACK_SIZE_LESS_THAN_1K
*/

/* for performance consideration, the following macro will call the corresponding API
* of libC for memcpy, memmove and memset
*/
#define CALL_LIBC_COR_API

/* codes should run under the macro COMPATIBLE_LINUX_FORMAT in unknow system on default,
   and strtold. The function
   strtold is referenced first at ISO9899:1999(C99), and some old compilers can
   not support these functions. Here provides a macro to open these functions:

       SECUREC_SUPPORT_STRTOLD  -- if defined, strtold will   be used
*/

/* define error code */
#ifndef errno_t
typedef int errno_t;
#endif

/* success */
#define EOK (0)

/* invalid parameter */
#ifdef EINVAL
#undef EINVAL
#endif
#define EINVAL           (22)
#define EINVAL_AND_RESET (22 | 0X80)
/* invalid parameter range */
#ifdef ERANGE
#undef ERANGE /* to avoid redefinition */
#endif
#define ERANGE           (34)
#define ERANGE_AND_RESET (34 | 0X80)

/* A wide-character code has been detected that does not correspond to a
* valid character, or a byte sequence does not form a valid wide-character code
*/
#ifdef EILSEQ
#undef EILSEQ
#endif
#define EILSEQ (42)

#ifdef EOVERLAP_AND_RESET
#undef EOVERLAP_AND_RESET
#endif
/* Once the buffer overlap is detected, the dest buffer must be reseted! */
#define EOVERLAP_AND_RESET (54 | 0X80)

/* if you need export the function of this library in Win32 dll, use __declspec(dllexport) */

/* return SecureC Version */
void getHwSecureCVersion(char *verStr, int bufSize, unsigned short *verNumber);

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
/* memset function */
static inline errno_t memset_s(void *dest, size_t destMax, int c, size_t count)
{
    if (destMax == 0 || count > destMax || dest == NULL) {
        return -EINVAL;
    }

    memset(dest, c, count);
    return EOK;
}

/* memcpy function */
static inline errno_t memcpy_s(void *dest, size_t destMax, const void *src, size_t count)
{
    if (destMax == 0 || count > destMax || dest == NULL || src == NULL) {
        return -EINVAL;
    }

    memcpy(dest, src, count);
    return EOK;
}

/* memmove */
static inline errno_t memmove_s(void *dest, size_t destMax, const void *src, size_t count)
{
    if (destMax == 0 || count > destMax || dest == NULL || src == NULL) {
        return -EINVAL;
    }

    memmove(dest, src, count);
    return EOK;
}

static inline int vsnprintf_s(char *strDest, size_t destMax, size_t count, const char *format, va_list arglist)
{
    int ret;
    size_t tempCount = count;

    if (format == NULL || strDest == NULL || destMax == 0) {
        return -EINVAL;
    }

    if (tempCount > destMax) {
        tempCount = destMax;
    }

    ret = vsnprintf(strDest, tempCount, format, arglist);
    return ret;
}

static int snprintf_s(char *strDest, size_t destMax, size_t count, const char *format, ...)
{
    int ret;
    va_list arglist;

    va_start(arglist, format);
    ret = vsnprintf_s(strDest, destMax, count, format, arglist);
    va_end(arglist);

    return ret;
}

/* strcpy */
static inline errno_t strcpy_s(char *strDest, size_t destMax, const char *strSrc)
{
    if (destMax == 0 || strDest == NULL || strSrc == NULL) {
        return -EINVAL;
    }

    if (strlen(strSrc) + 1 > destMax) {
        return -EINVAL;
    }
    strcpy(strDest, strSrc);
    return EOK;
}

/* strncpy */
static inline errno_t strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count)
{
    size_t srcStrLen;

    if (destMax == 0 || strDest == NULL || strSrc == NULL) {
        return -EINVAL;
    }

    srcStrLen = strlen(strSrc);
    if ((size_t)(TWO_MIN(srcStrLen, count) + 1) > destMax) {
        return -EINVAL;
    }

    strncpy(strDest, strSrc, count);
    return EOK;
}

/* strcat */
static inline errno_t strcat_s(char *strDest, size_t destMax, const char *strSrc)
{
    if (destMax == 0 || strDest == NULL || strSrc == NULL) {
        return -EINVAL;
    }

    strcat(strDest, strSrc);
    return EOK;
}

/* strncat */
static inline errno_t strncat_s(char *strDest, size_t destMax, const char *strSrc, size_t count)
{
    if (destMax == 0 || strDest == NULL || strSrc == NULL) {
        return -EINVAL;
    }

    strncat(strDest, strSrc, count);
    return EOK;
}

static inline char *strtok_s(char *strToken, const char *strDelimit, char **context)
{
    char *token = NULL;
    const char *ctl = NULL;

    /* validate delimiter and string context */
    if (context == NULL || strDelimit == NULL) {
        return NULL;
    }

    /* valid input string and string pointer from where to search */
    if (strToken == NULL && (*context) == NULL) {
        return NULL;
    }

    /* If string is null, continue searching from previous string position stored in context */
    if (strToken == NULL) {
        strToken = *context;
    }

    /* Find beginning of token (skip over leading delimiters). Note that
    * there is no token if this loop sets string to point to the terminal null.
 */
    while (*strToken != 0) {
        ctl = strDelimit;
        while (*ctl != 0 && *ctl != *strToken) {
            ++ctl;
        }
        /* don't find any delimiter in string header, break the loop */
        if (*ctl == 0) {
            break;
        }
        ++strToken;
    }

    token = strToken; /* point to updated position */

    /* Find the rest of the token. If it is not the end of the string,
    * put a null there.
 */
    for (; *strToken != 0; strToken++) {
        if (*ctl != 0) { /* find a delimiter */
            *strToken++ = 0; /* set string termintor */
            break;
        }
    }

    /* record string position for next search in the context */
    *context = strToken;

    /* Determine if a token has been found. */
    if (token == strToken) {
        return NULL;
    } else {
        return token;
    }
}
#else

/* strcpy */
errno_t strcpy_s(char *strDest, size_t destMax, const char *strSrc);

/* memmove */
errno_t memmove_s(void *dest, size_t destMax, const void *src, size_t count);

int vsnprintf_s(char *strDest, size_t destMax, size_t count, const char *format, va_list arglist) SECUREC_ATTRIBUTE(4, 0);

/* snprintf */
#if defined(__KERNEL__) || defined(_PRE_LINUX_TEST)
int snprintf_s(char *strDest, size_t destMax, size_t count, const char *format, ...) SECUREC_ATTRIBUTE(4, 5);
#else  // for ut
#define snprintf_s _snprintf_s
#endif

/* sprintf */
int sprintf_s(char *strDest, size_t destMax, const char *format, ...) SECUREC_ATTRIBUTE(3, 4);

/* memset function */
errno_t memset_s(void *dest, size_t destMax, int c, size_t count);
/* memcpy function */
errno_t memcpy_s(void *dest, size_t destMax, const void *src, size_t count);

/* strncpy */
errno_t strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);

/* strcat */
errno_t strcat_s(char *strDest, size_t destMax, const char *strSrc);
/* strncat */
errno_t strncat_s(char *strDest, size_t destMax, const char *strSrc, size_t count);

/* strtok */
char *strtok_s(char *strToken, const char *strDelimit, char **context);
#endif
#if 0
/* wmemcpy */
errno_t wmemcpy_s(wchar_t *dest, size_t destMax, const wchar_t *src, size_t count);

/* memmove */
errno_t wmemmove_s(wchar_t *dest, size_t destMax, const wchar_t *src, size_t count);

errno_t wcscpy_s(wchar_t *strDest, size_t destMax, const wchar_t *strSrc);

errno_t wcsncpy_s(wchar_t *strDest, size_t destMax, const wchar_t *strSrc, size_t count);

errno_t wcscat_s(wchar_t *strDest, size_t destMax, const wchar_t *strSrc);

errno_t wcsncat_s(wchar_t *strDest, size_t destMax, const wchar_t *strSrc, size_t count);

wchar_t *wcstok_s(wchar_t *strToken, const wchar_t *strDelimit, wchar_t **context);

/* sprintf */
int sprintf_s(char *strDest, size_t destMax, const char *format, ...) SECUREC_ATTRIBUTE(3, 4);

int swprintf_s(wchar_t *strDest, size_t destMax, const wchar_t *format, ...);

/* vsprintf */
int vsprintf_s(char *strDest, size_t destMax, const char *format, va_list argptr) SECUREC_ATTRIBUTE(3, 0);

int vswprintf_s(wchar_t *strDest, size_t destMax, const wchar_t *format, va_list argptr);

/* scanf */
int scanf_s(const char *format, ...);

int wscanf_s(const wchar_t *format, ...);

/* vscanf */
int vscanf_s(const char *format, va_list arglist);

int vwscanf_s(const wchar_t *format, va_list arglist);

/* fscanf */
int fscanf_s(FILE *stream, const char *format, ...);

int fwscanf_s(FILE *stream, const wchar_t *format, ...);

/* vfscanf */
int vfscanf_s(FILE *stream, const char *format, va_list arglist);

int vfwscanf_s(FILE *stream, const wchar_t *format, va_list arglist);

/* sscanf */
int sscanf_s(const char *buffer, const char *format, ...);

int swscanf_s(const wchar_t *buffer, const wchar_t *format, ...);

/* vsscanf */
int vsscanf_s(const char *buffer, const char *format, va_list argptr);

int vswscanf_s(const wchar_t *buffer, const wchar_t *format, va_list arglist);

/* gets */
char *gets_s(char *buffer, size_t destMax);

#endif

errno_t strncpy_error(char *strDest, size_t destMax, const char *strSrc, size_t count);
errno_t strcpy_error(char *strDest, size_t destMax, const char *strSrc);

#if defined(WITH_PERFORMANCE_ADDONS)
/* those functions are used by macro */
errno_t memset_sOptTc(void *dest, size_t destMax, int c, size_t count);
errno_t memcpy_sOptTc(void *dest, size_t destMax, const void *src, size_t count);

/* strcpy_sp is a macro, NOT a function in performance optimization mode. */
#define strcpy_sp(dest, destMax, src) /*lint -save -e506 -e1055 */ (( __builtin_constant_p((destMax)) && __builtin_constant_p((src))) ?  \
    STRCPY_SM((dest), (destMax), (src)):strcpy_s((dest), (destMax), (src)) ) /*lint -restore */

/* strncpy_sp is a macro, NOT a function in performance optimization mode. */
#define strncpy_sp(dest, destMax, src, count) /*lint -save -e506 -e1055 */ ((__builtin_constant_p((count)) &&  __builtin_constant_p((destMax)) && __builtin_constant_p((src))) ?  \
    STRNCPY_SM((dest), (destMax), (src), (count)):strncpy_s((dest), (destMax), (src), (count)) ) /*lint -restore */

/* strcat_sp is a macro, NOT a function in performance optimization mode. */
#define strcat_sp(dest, destMax, src) /*lint -save -e506 -e1055 */ (( __builtin_constant_p((destMax)) && __builtin_constant_p((src))) ?  \
    STRCAT_SM((dest), (destMax), (src)):strcat_s((dest), (destMax), (src)) ) /*lint -restore */

/* strncat_sp is a macro, NOT a function in performance optimization mode. */
#define strncat_sp(dest, destMax, src, count) /*lint -save -e506 -e1055 */ ((__builtin_constant_p((count)) &&  __builtin_constant_p((destMax)) && __builtin_constant_p((src))) ?  \
    STRNCAT_SM((dest), (destMax), (src), (count)):strncat_s((dest), (destMax), (src), (count)) ) /*lint -restore */

/* memcpy_sp is a macro, NOT a function in performance optimization mode. */
#define memcpy_sp(dest, destMax, src, count) /*lint -save -e506 -e1055 */ (__builtin_constant_p((count)) ? (MEMCPY_SM((dest), (destMax),  (src), (count))) :  \
       (__builtin_constant_p((destMax)) ? (((size_t)(destMax) > 0 && (((UINT64T)(destMax) & (UINT64T)(-2)) < SECUREC_MEM_MAX_LEN)) ? memcpy_sOptTc((dest), (destMax), (src), (count)) : ERANGE ):memcpy_s((dest), (destMax), (src), (count)))) /*lint -restore */

/* memset_sp is a macro, NOT a function in performance optimization mode. */
#define memset_sp(dest, destMax, c, count) /*lint -save -e506 -e1055 */ (__builtin_constant_p((count)) ? (MEMSET_SM((dest), (destMax),  (c), (count))) :  \
       (__builtin_constant_p((destMax)) ? (((size_t)(destMax) > 0 && (((UINT64T)(destMax) & (UINT64T)(-2)) < SECUREC_MEM_MAX_LEN)) ? memset_sOptTc((dest), (destMax), (c), (count)) : ERANGE ):memset_s((dest), (destMax), (c), (count)))) /*lint -restore */

#endif

#endif /* __SECUREC_H__5D13A042_DC3F_4ED9_A8D1_882811274C27 */



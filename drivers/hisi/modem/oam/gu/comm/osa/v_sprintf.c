/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: v_sprintf.c                                                     */
/*                                                                           */
/* Author:                                                                   */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date:                                                                     */
/*                                                                           */
/* Description: copy this file from Dopra                                    */
/*                                                                           */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification:                                                          */
/*                                                                           */
/*****************************************************************************/


#include "v_IO.h"

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767 modify:x51137; review:h59254; cause:print log */
#define    THIS_FILE_ID        PS_FILE_ID_V_SPRINTF_C
/*lint +e767 modify:x51137; review:h59254; */


/*****************************************************************************
 Function   : _Second2DateTime
 Description: Convert the number of seconds to the dates
 Input      : ulSec      -- The seconds set by the user
              pstRetDate -- The current date, in the format of Y/M/D (31-16/15-8/7-0)
              pstRetTime -- The current time, in the format of h/m/s (31-16/15-8/7-0)
 Output     : None
 Return     : None
 ******************************************************************************/
VOS_VOID _Second2DateTime( VOS_UINT32 ulSec,
                          VOS_UINT32 *pulRetDate,
                          VOS_UINT32 *pulRetTime )
{
    VOS_UINT32 dmsize[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    VOS_UINT32 ulYear;
    VOS_UINT32 ulMonth;
    VOS_UINT32 ulTempSec;
    VOS_UINT32 ulHour, ulMin;
    VOS_UINT32 ulTemp = ulSec;

    if( VOS_NULL_PTR == pulRetDate )
    {
        return;
    }

    if( VOS_NULL_PTR == pulRetTime )
    {
        return;
    }

    /*year*/
    for( ulYear = 1970;; )
    {
        ulTempSec = ( VOS_IsLeap( ulYear ) ? 366 : 365 ) * 24 * 60 * 60;
        if( ulTemp >= ulTempSec )
        {
            ulYear ++;
            ulTemp -= ulTempSec;
        }
        else
        {
            break;
        }
    }

    /*month*/
    for( ulMonth = 1;; )
    {
        ulTempSec = (VOS_UINT32 )( ( VOS_IsLeap( ulYear ) && ( ulMonth == 2 ) ) ? \
                    ( dmsize[ulMonth] + 1 ) : dmsize[ulMonth] ) * 24 * 60 * 60;
        if( ulTemp >= ulTempSec )
        {
            ulMonth ++;
            ulTemp -= ulTempSec;
        }
        else
        {
            break;
        }
    }
    *pulRetDate = ( ulYear << 16 ) | ( ulMonth<<8 ) | \
        ( ulTemp / ( 24 * 60 * 60 ) + 1 );

    ulTemp %= ( 24 * 60 * 60 );

    ulHour = ulTemp/ ( 60 * 60 );
    ulTemp %= ( 60 * 60 );

    ulMin = ulTemp / 60;
    ulTemp %= 60;

    *pulRetTime = ( ulHour << 16 ) | ( ulMin << 8 ) | ulTemp;

    return;
}

/**************************************************************************
 Function    : _C_formatter
 Description :

 Calls       :
 Called By   :
 Input       :
 Output      :
 Return      :
 **************************************************************************/
VOS_INT32 _C_formatter(const VOS_CHAR *format,
                VOS_VOID        put_one_char(VOS_UINT8, VOS_VOID *,VOS_CHAR *),
                VOS_VOID       *secret_pointer,
                va_list     ap,     VOS_CHAR * str)
{
    VOS_CHAR                format_flag;
    VOS_INT32                 precision;
    VOS_INT32                 length, mask, nr_of_bits, n;
    VOS_INT32                 field_width;
    VOS_CHAR                flag_char, left_adjust;
    VOS_UINT_PTR           u_address;


    VOS_INT32                num;
    VOS_CHAR               *buf_pointer;
    VOS_CHAR               *ptr, *hex;
    VOS_CHAR                zeropad;
    VOS_CHAR                buf[FRMWRI_BUFSIZE];
    VOS_UINT32              ulDate = 0;/* added by wuxiaoqian 2k/09/20 for "%t"*/
    VOS_UINT32              ulTime = 0;/* added by wuxiaoqian 2k/09/20 for "%t"*/

    for (;;)    /* Until full format string read */
    {
        while ((format_flag = *format++) != '%')        /* Until '%' or '\0' */
        {
            if (!format_flag)
            {
                *(str+*((VOS_INT32 *)secret_pointer))='\0';
                /*nr_of_chars计数有错,当\n时,实际是2个字符,nr_of_chars只记了一次*/
                return *((VOS_INT32 *)secret_pointer);
            }
            put_one_char ((VOS_UINT8)format_flag, secret_pointer,str);

        }   /* while () */

        if (*format == '%')             /* %% prints as % */
        {
            format++;
            put_one_char('%',secret_pointer,str);

            continue;
        }   /* if () */

        flag_char = 0;
        left_adjust = 0;
        /*=====================================*/
        /* check for leading -, + or ' 'flags  */
        /*=====================================*/
        for (;;)
        {
            if (*format == '+' || *format == ' ')
            {
                flag_char = *format++;
            }
            else if (*format == '-')
            {
                left_adjust++;
                format++;
            }
            else
            {
                break;
            }
        }   /* for (;;) */

        /*======================================*/
        /* a '0' may follow the flag character  */
        /* this is the requested fill character */
        /*======================================*/
        if (*format == '0')
        {
            zeropad = 1;
            format++;
        }
        else
        {
            zeropad = 0;
        }

        /*===================================*/
        /* Optional field width (may be '*') */
        /*===================================*/
        if (*format == '*')
        {
            /*lint -e147 -e586*/
            field_width = va_arg(ap, VOS_INT32);
            /*lint +e147 +e586*/
            if (field_width < 0)
            {
                field_width = (VOS_CHAR)(-field_width);
                left_adjust = (VOS_CHAR)(!left_adjust);
            }   /* if () */
            format++;
        }
        else
        {
            field_width = 0;
            while (*format >= '0' && *format <= '9')
            {
                field_width = field_width * 10 + (*format++ - '0');
            }
        }   /* if () */

        /*=============================*/
        /* Optional precision (or '*') */
        /*=============================*/
        if (*format=='.')
        {
            if (*++format == '*')
            {
                /*lint -e147 -e586*/
                precision = va_arg(ap, VOS_INT32);
                /*lint +e147 +e586*/
                format++;
            }
            else
            {
                precision = 0;
                while (*format >= '0' && *format <= '9')
                {
                    precision = precision * 10 + (*format++ - '0');
                }
            }   /* if () */
        }
        else
        {
            precision = -1;
        }

        /*======================================================*/
        /* At this point, "left_adjust" is nonzero if there was */
        /* a sign, "zeropad" is 1 if there was a leading zero   */
        /* and 0 otherwise, "field_width" and "precision"       */
        /* contain numbers corresponding to the digit strings   */
        /* before and after the decimal point, respectively,    */
        /* and "flag_char" is either 0 (no flag) or contains    */
        /* a plus or space character. If there was no decimal   */
        /* point, "precision" will be -1.                       */
        /*======================================================*/

        /*========================*/
        /* Optional "l" modifier? */
        /*========================*/
        if (*format == 'l')
        {
            length = 1;
            format++;
        }
        else
            if (*format == 'F')
            {
                length = 1;
                format++;
            }
            else
            {
                length = 0;
            }

        /*===================================================*/
        /* At exit from the following switch, we will emit   */
        /* the characters starting at "buf_pointer" and      */
        /* ending at "ptr"-1                                 */
        /*===================================================*/
        switch (format_flag = *format++)
        {
        case 'c':
            /*lint -e147 -e586*/
            buf[0] = (VOS_CHAR)va_arg(ap, VOS_INT32);
            /*lint +e147 +e586*/
            ptr = buf_pointer = &buf[0];
            if (buf[0])
            {
                ptr++;
            }
            break;

        case 's':
            /*lint -e147 -e586*/
            if ((buf_pointer = va_arg(ap,VOS_CHAR *)) == NULL_PTR)
            {
                /*lint +e147 +e586*/
                buf_pointer = "(null pointer)";
            }
            if (precision < 0)
            {
                precision = 10000;
            }
            for (n=0; *buf_pointer++ && n < precision; n++)
            {
                ;
            }
            ptr = --buf_pointer;
            buf_pointer -= n;
            break;

        case 'o':
        case 'p':
        case 'X':
        case 'x':
            if (format_flag == 'p')
            { /* huyong ??? ---- %p和标准的输出不同 */
                if (length)
                {
                    /*lint -e147 -e586*/
                    u_address = (VOS_UINT_PTR)va_arg(ap,VOS_CHAR *);
                    /*lint +e147 +e586*/
                }
                else
                {
                    /*lint -e147 -e586*/
                    u_address = (VOS_UINT_PTR)va_arg(ap,VOS_CHAR *);
                    /*lint +e147 +e586*/
                }
            }
            else if (length)
            {
                /*lint -e147 -e586*/
                u_address = va_arg(ap,VOS_UINT32);
                /*lint +e147 +e586*/
            }
            else
            {
                /*lint -e147 -e586*/
                u_address = (VOS_UINT_PTR)va_arg(ap,VOS_INT32);
                /*lint +e147 +e586*/
            }

            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];
            hex = "0123456789ABCDEF";
            if (format_flag == 'o')
            {
                mask = 0x7;
                nr_of_bits = 3;
            }
            else
            {
                if (format_flag == 'x')
                {
                    hex = "0123456789abcdef";
                }
                mask = 0xf;
                nr_of_bits = 4;
            }   /* if () */

            do
            {
                *--buf_pointer = *(hex + ((VOS_INT32) u_address & mask));
            }
            while ( ( u_address >>= nr_of_bits ) != 0 );

            if (precision < 0)          /* "precision" takes precedence */
            {
                if (zeropad)
                {
                    precision = field_width;
                }
            }
            while (precision > ptr - buf_pointer)
            {
                *--buf_pointer = '0';
            }
            break;

        case 'd':
        case 'i':
        case 'u':
            if (length)
            {
                /*lint -e147 -e586*/
                num = va_arg(ap,VOS_INT32);
                /*lint +e147 +e586*/
            }
            else
            {
                /*lint -e147 -e586*/
                n = va_arg(ap,VOS_INT32);
                /*lint +e147 +e586*/
                num = (VOS_INT32) n;
            }   /* if () */
            if ( format_flag != 'u' && num < 0  )
            {
                n = 1;
                flag_char = '-';
                u_address = (VOS_UINT32)-num;
            }
            else
            {
                n = (VOS_INT)(flag_char != 0);
                u_address = (VOS_UINT32)num;
            }   /* if () */

            /*=======================*/
            /* now convert to digits */
            /*=======================*/
            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];
            do
            {
                *--buf_pointer = (VOS_CHAR)((u_address % 10) + '0');
            }
            while ( (u_address /= 10) != 0 );

            if (precision < 0)        /* "precision" takes precedence */
            {
                if (zeropad)
                {
                    precision = field_width - n;
                }
            }
            while (precision > ptr - buf_pointer)
            {
                *--buf_pointer = '0';
            }
            break;

        case 'f':
            format_flag = 0;
            /*fall though*/
        case 'e':
            /*fall though*/
        case 'E':
            ptr = buf_pointer = "FLOATS? wrong formatter installed!";
            while (*ptr)
            {
                ptr++;
            }
            break;
        /******************************************************************
        * added by wuxiaoqian 2k/09/20                  *
        * output Datetime, the input is a VOS_UINT32 (sec from 1970/1/1)*
        ******************************************************************/
        case 'T':
            /*lint -e147 -e586*/
            num = va_arg(ap,VOS_INT32);
            /*lint +e147 +e586*/
            _Second2DateTime((VOS_UINT32)num, &ulDate, &ulTime);

            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];

            /* second */
            u_address = ulTime & 0xff;
            /*do
                *--buf_pointer = (CHAR)(( ulong % 10) + '0');
            while ( (ulong /= 10) != 0 );*/
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = ':';

            /* minute */
            u_address = ( ulTime >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = ':';

            /* hour */
            u_address = ulTime >> 16;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = ' ';

            /* date */
            u_address = ulDate & 0xff;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = '/';

            /* month */
            u_address = ( ulDate >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = '/';

            /* year */
            u_address = ulDate >> 16;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            break;

        case 't':
            /*lint -e147 -e586*/
            num = va_arg(ap,VOS_INT32);
            /*lint +e147 +e586*/
            _Second2DateTime((VOS_UINT32)num, &ulDate, &ulTime);

            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];

            /* date */
            u_address = ulDate & 0xff;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = '/';

            /* month */
            u_address = ( ulDate >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            *--buf_pointer = '/';

            /* year */
            u_address = ulDate >> 16;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');
            u_address /= 10;
            *--buf_pointer = (VOS_CHAR)(( u_address % 10) + '0');

            break;

        case '\0':              /* Really bad place to find NUL in */
        format--;
        /*lint -e616 */
        default:
        /*lint +e616 */
        /*=======================*/
        /* Undefined conversion! */
        /*=======================*/
        /*lint -e585 */
        ptr = buf_pointer = "???";
        /*lint +e585 */
        ptr += 4;
        break;
        }   /* switch () */

        /*===========================================================*/
        /* This part emittes the formatted string to "put_one_char". */
        /*===========================================================*/
        if ((length = ptr - buf_pointer) > field_width)
        {
            n = 0;
        }
        else
        {
            n = field_width - length - (VOS_INT)(flag_char != 0);
        }

        /*=================================*/
        /* emit any leading pad characters */
        /*=================================*/
        if (!left_adjust)
        {
            while (--n >= 0)
            {
                put_one_char(' ', secret_pointer,str);

            }   /* while () */
        }

        /*===============================*/
        /* emit flag characters (if any) */
        /*===============================*/
        if (flag_char)
        {
            put_one_char((VOS_UINT8)flag_char, secret_pointer,str);

        }   /* if () */

        /*========================*/
        /* emit the string itself */
        /*========================*/
        while (--length >= 0)
        {
            put_one_char((VOS_UINT8)(*buf_pointer++), secret_pointer,str);

        }   /* while () */

        /*================================*/
        /* emit trailing space characters */
        /*================================*/
        if (left_adjust)
        {
            while (--n >= 0)
            {
                put_one_char(' ', secret_pointer,str);

            }   /* while () */
        }
    }   /* for (;;) */


}   /* _C_formatter(,,,) */

/*****************************************************************************
 Function   : comio
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID comio(VOS_UINT8 output, VOS_VOID *number,VOS_CHAR * strbuf)
{
    /* (*((VOS_INT32 *) number))++;*/

    /* Modified by Zym 2000/8/4 */
    /*if (output == '\n')
    {
        *(strbuf+*((VOS_INT32 *)number))=('\r');
        (*((VOS_INT32*)number))++;
        if( (*((VOS_INT32 *)number)) == (Str_Length-1))  *((VOS_INT32 *)number)-=1;
    }*/

    *(strbuf+*((VOS_INT32 *)number))=(VOS_INT8)output;
    (*((VOS_INT32 *)number))++;
    if( (*((VOS_INT32 *)number)) == (Str_Length-1))  *((VOS_INT32 *)number)-=1;
}


/*****************************************************************************
 Function   : exponent
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
/*lint -e578 */
char *exponent(char *p,int exp, unsigned char fmtch)
{
/*lint +e578 */

     register char *t;
     char expbuf[MAXEXP];

     *p++ =(char)fmtch;
     if (exp < 0) {
         exp = -exp;
         *p++ = '-';
     }
     else
         *p++ = '+';
     t = expbuf + MAXEXP;
     if (exp > 9) {
         do {
             *--t = (char)(tochar(exp % 10));
         } while ((exp /= 10) > 9);
         *--t = (char)(tochar(exp));
         /*lint -e722 */
         for (; t < expbuf + MAXEXP; *p++ = *t++)
         /*lint +e722 */
         {

         }
     }
     else {
         *p++ = '0';
         *p++ = (char)(tochar(exp));
     }
     return(p);
}

/*****************************************************************************
 Function   : ANSI_vsprintf
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT32 ANSI_vsprintf(VOS_CHAR *out_buf, const VOS_CHAR *fmt0, va_list argp)
{
    VOS_INT                 lChar;
    VOS_INT                 ltemp;
    VOS_INT                 lBase;
    VOS_INT                 lDprec;
    VOS_INT                 lFieldSize;
    VOS_INT                 lFlags;
    VOS_INT                 lFpprec;
    VOS_INT                 lPrec;
    VOS_INT                 lRealSize;
    VOS_INT                 lConverSize;
    VOS_INT                 lFormatWidth;
    VOS_UINT_PTR            ulLong;
    VOS_CHAR                ucPrefixSign;
    VOS_CHAR                *pucDigs;
    VOS_CHAR                ucBuf[BUF];
    VOS_CHAR                *pcBuf;
    VOS_CHAR                *pucOutBuf=out_buf;
    VOS_UCHAR               *pucFmt;
    VOS_INT32               num;
    VOS_UINT32              ulTimelong;

    VOS_CHAR               *buf_pointer;

    VOS_UINT32              ulDate = 0;             /* added by wuxiaoqian 2k/09/20 for "%t"*/
    VOS_UINT32              ulTime = 0;             /* added by wuxiaoqian 2k/09/20 for "%t"*/

    pucFmt = (VOS_UCHAR *)fmt0;
    pucDigs = "0123456789abcdef";
    /*lint -e801 -e445 */
    for (;; ++pucFmt)
    {
            for (; ((lChar = *pucFmt)!=0) && lChar != '%'; ++pucFmt)
            {
                *pucOutBuf++ =(VOS_CHAR) lChar;
            }
            if (!lChar)
            {
                goto    lvspret;
            }

            lFlags = 0; lDprec = 0; lFpprec = 0; lFormatWidth = 0;
            lPrec = -1;
            ucPrefixSign = '\0';

rflag:  switch (*++pucFmt) {
        case ' ':
            if (!ucPrefixSign)
            {
                ucPrefixSign = ' ';
            }
            goto rflag;
        case '#':
            lFlags |= ALT;
            goto rflag;
        case '*':
            /*lint -e147 -e586*/
            if ((lFormatWidth = va_arg(argp, VOS_INT)) >= 0)
            {
                /*lint +e147 +e586*/
                goto rflag;
            }
            lFormatWidth = -lFormatWidth;
        /*lint -e616 */
        case '-':
        /*lint +e616 */
            lFlags |= LADJUST;
            goto rflag;
        case '+':
            ucPrefixSign = '+';
            goto rflag;
        case '.':
            if (*++pucFmt == '*')
            {
                /*lint -e147 -e586*/
                ltemp = va_arg(argp, VOS_INT);
                /*lint +e147 +e586*/
            }
            else
            {
                ltemp = 0;
                while (isascii(*pucFmt) && isdigit(*pucFmt))
                {
                    ltemp = 10 * ltemp + todigit(*pucFmt++);
                }
                --pucFmt;
            }
            lPrec = ltemp < 0 ? -1 : ltemp;
            goto rflag;
        case '0':
            lFlags |= ZEROPAD;
            goto rflag;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ltemp = 0;
            do {
                ltemp = 10 * ltemp + todigit(*pucFmt);
            } while (isascii(*++pucFmt) && isdigit(*pucFmt));
            lFormatWidth = ltemp;
            --pucFmt;
            goto rflag;
        case 'L':
            lFlags |= LONGDBL;
            goto rflag;
        case 'h':
            lFlags |= SHORTINT;
            goto rflag;
        case 'l':
            lFlags |= LONGINT;
            goto rflag;
        case 'c':
            /*lint -e147 -e586*/
            *(pcBuf = ucBuf) = (VOS_CHAR)va_arg(argp, VOS_INT);
            /*lint +e147 +e586*/
            lConverSize = 1;
            ucPrefixSign = '\0';
            goto pforw;
        case 'D':
            lFlags |= LONGINT;
        /*lint -e616 */
        case 'd':
        /*lint +e616 */
        case 'i':
            /*lint -e147 -e586*/
            ARG(int);
            /*lint +e147 +e586*/
            if ((VOS_INT32)ulLong < 0)
            {
                ulLong = (VOS_UINT32)(-(VOS_INT32)ulLong);
                ucPrefixSign = '-';
            }
            lBase = 10;
            goto number;
        case 'n':
            if (lFlags & LONGINT)
            {
                /*lint -e147 -e586*/
                *va_arg(argp, VOS_INT32 *) = (VOS_INT32)(pucOutBuf-out_buf);
                /*lint +e147 +e586*/
            }
            else if (lFlags & SHORTINT)
            {
                /*lint -e147 -e586*/
                *va_arg(argp, VOS_INT16 *) = (VOS_INT16)(pucOutBuf-out_buf);
                /*lint +e147 +e586*/
            }
            else
            {
                /*lint -e147 -e586*/
                *va_arg(argp, VOS_INT *) = (VOS_INT)(pucOutBuf-out_buf);
                /*lint +e147 +e586*/
            }
            break;
        case 'O':
            lFlags |= LONGINT;
        /*lint -e616 */
        case 'o':
        /*lint +e616 */
            /*lint -e147 -e586*/
            ARG(unsigned);
            /*lint +e147 +e586*/
            lBase = 8;
            goto nosign;
        case 'p':
            /*lint -e147 -e586*/
            ulLong = (VOS_UINT_PTR)va_arg(argp, VOS_VOID *);
            /*lint +e147 +e586*/
            lBase = 16;
            goto nosign;
        case 's':
            /*lint -e147 -e586*/
            if ( (pcBuf = va_arg(argp, VOS_CHAR *))==0 )
            {
                /*lint +e147 +e586*/
                pcBuf = "(null)";
            }
            if (lPrec >= 0)
            {
                VOS_CHAR *p;

                if ((p = (VOS_CHAR *)VOS_MemChr((const VOS_CHAR *)pcBuf, 0, lPrec))!=0)
                {
                    lConverSize = p - pcBuf;
                    if (lConverSize > lPrec)
                    {
                        lConverSize = lPrec;
                    }
                }
                else
                {
                    lConverSize = lPrec;
                }
            }
            else
            {
                lConverSize =(VOS_INT)VOS_StrLen(pcBuf);
            }
            ucPrefixSign = '\0';
            goto pforw;
        case 'U':
            lFlags |= LONGINT;
        /*lint -e616 */
        case 'u':
        /*lint +e616 */
            /*lint -e147 -e586*/
            ARG(unsigned);
            /*lint +e147 +e586*/
            lBase = 10;
            goto nosign;
        case 'X':
            pucDigs = "0123456789ABCDEF";
        /*lint -e616 */
        case 'x':
        /*lint +e616 */
            /*lint -e147 -e586*/
            ARG(unsigned);
            /*lint +e147 +e586*/
            lBase = 16;
            if (lFlags & ALT && ulLong != 0)
            {
                lFlags |= HEXPREFIX;
            }

nosign:            ucPrefixSign = '\0';

number:    if ((lDprec = lPrec) >= 0)
            {
                lFlags &= ~ZEROPAD;
            }

            pcBuf = ucBuf + BUF;
            if (ulLong != 0 || lPrec != 0)
            {
                do {
                    *--pcBuf = pucDigs[ulLong %(VOS_UINT32) lBase];
                    ulLong /=(VOS_UINT32) lBase;
                } while (ulLong);
                pucDigs = "0123456789abcdef";
                if (lFlags & ALT && lBase == 8 && *pcBuf != '0')
                {
                    *--pcBuf = '0';
                }
            }
            lConverSize = ucBuf + BUF - pcBuf;

pforw:
            lFieldSize = lConverSize + lFpprec;
            if (ucPrefixSign)
            {
                lFieldSize++;
            }
            if (lFlags & HEXPREFIX)
            {
                lFieldSize += 2;
            }
            lRealSize = lDprec > lFieldSize ? lDprec : lFieldSize;

            if ((lFlags & (LADJUST|ZEROPAD)) == 0 && lFormatWidth)
            {
                for (ltemp = lRealSize; ltemp < lFormatWidth; ltemp++)
                {
                    *pucOutBuf++=' ';
                }
            }

            if (ucPrefixSign)
            {
                *pucOutBuf++=ucPrefixSign;
            }
            if (lFlags & HEXPREFIX)
            {
                *pucOutBuf++='0';
                *pucOutBuf++=(char)*pucFmt;
            }
            if ((lFlags & (LADJUST|ZEROPAD)) == ZEROPAD)
            {
                for (ltemp = lRealSize; ltemp < lFormatWidth; ltemp++)
                {
                    *pucOutBuf++='0';
                }
            }
            for (ltemp = lFieldSize; ltemp < lDprec; ltemp++)
            {
                *pucOutBuf++='0';
            }

            ltemp = lConverSize;
            while (--ltemp >= 0)
            {
                *pucOutBuf++=*pcBuf++;
            }

            while (--lFpprec >= 0)
            {
                *pucOutBuf++='0';
            }

            if (lFlags & LADJUST)
            {
                for (ltemp = lRealSize; ltemp < lFormatWidth; ltemp++)
                {
                    *pucOutBuf++=' ';
                }
            }

            break;
        case '\0':
                        goto    lvspret;
        case 'T':
            /*lint -e147 -e586*/
            num = va_arg(argp,VOS_INT32);
            /*lint +e147 +e586*/
            _Second2DateTime((VOS_UINT32)num, &ulDate, &ulTime);

            buf_pointer = &ucBuf[BUF- 1];

            /* second */
            ulTimelong = ulTime & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = ':';

            /* minute */
            ulTimelong = ( ulTime >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = ':';

            /* hour */
            ulTimelong = ulTime >> 16;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = ' ';

            /* date */
            ulTimelong = ulDate & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = '/';

            /* month */
            ulTimelong = ( ulDate >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = '/';

            /* year */
            ulTimelong = ulDate >> 16;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            /*lint -e534*/
            VOS_StrNCpy(pucOutBuf, buf_pointer, 19);
            /*lint +e534*/
            pucOutBuf += 19 ;
            continue ;

        case 't':
            /*lint -e147 -e586*/
            num = va_arg(argp,VOS_INT32);
            /*lint +e147 +e586*/
            _Second2DateTime((VOS_UINT32)num, &ulDate, &ulTime);

            buf_pointer = &ucBuf[BUF- 1];

            /* date */
            ulTimelong = ulDate & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = '/';

            /* month */
            ulTimelong = ( ulDate >> 8 ) & 0xff;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            *--buf_pointer = '/';

            /* year */
            ulTimelong = ulDate >> 16;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');
            ulTimelong /= 10;
            *--buf_pointer = (VOS_CHAR)(( ulTimelong % 10) + '0');

            /*lint -e534*/
            VOS_StrNCpy(pucOutBuf, buf_pointer, 10);
            /*lint +e534*/
            pucOutBuf += 10 ;

            continue ;
        default:
                        *pucOutBuf++=(char)*pucFmt;

        }
    }
    /*lint +e801 +e445 */
lvspret:
        *pucOutBuf=0;
        return (VOS_INT)(pucOutBuf-out_buf);
}

/*****************************************************************************
 Function   : VOS_vsprintf
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT32 VOS_vsprintf(VOS_CHAR * str,const VOS_CHAR *format,va_list argument)
{
    VOS_INT32 i;

    if (( VOS_NULL_PTR == str ) || ( VOS_NULL_PTR == format ) )
    {
        return -1;
    }

    i=(VOS_INT32)ANSI_vsprintf( str , format , argument );
    return i;
}

/*****************************************************************************
 Function   : VOS_vsprintf_s
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT VOS_vsprintf_s(VOS_CHAR * str, VOS_SIZE_T ulDestSize, const VOS_CHAR *format,va_list argument)
{
    if (( VOS_NULL_PTR == str ) || ( VOS_NULL_PTR == format ) )
    {
        return -1;
    }

    if ( ulDestSize > VOS_SECUREC_MEM_MAX_LEN )
    {
        return -1;
    }

    return (VOS_INT)ANSI_vsprintf(str, format, argument);
}

/*****************************************************************************
 Function   : VOS_sprintf
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT32 VOS_sprintf(VOS_CHAR *str, const VOS_CHAR *fmt, ...)
{

    /*lint -e530 -e830 */
    va_list arg;
    register VOS_INT32 nc;

    /* HY17247 */
    if (( VOS_NULL_PTR == str ) || ( VOS_NULL_PTR == fmt ) )
    {
        return -1;
    }

    /*lint -e586*/
    va_start(arg, fmt);
    /*lint +e586*/
    nc = ANSI_vsprintf(str, (const VOS_CHAR *) fmt, arg);
    /*lint -e586*/
    va_end(arg);
    /*lint +e586*/
    return (nc);
    /*lint +e530 +e830 */
}

/*****************************************************************************
 Function   : VOS_sprintf_s
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT VOS_sprintf_s(VOS_CHAR *str, VOS_SIZE_T ulDestSize, const VOS_CHAR *fmt, ...)
{
    /*lint -e530 -e830 */
    va_list arg;
    register VOS_INT nc;

    /* HY17247 */
    if (( VOS_NULL_PTR == str ) || ( VOS_NULL_PTR == fmt ) )
    {
        return -1;
    }

    if ( ulDestSize > VOS_SECUREC_MEM_MAX_LEN )
    {
        return -1;
    }

    /*lint -e586*/
    va_start(arg, fmt);
    /*lint +e586*/
    nc = (VOS_INT)ANSI_vsprintf(str, (const VOS_CHAR *) fmt, arg);
    /*lint -e586*/
    va_end(arg);
    /*lint +e586*/
    return (nc);
    /*lint +e530 +e830 */   
}

#define LENGTH_OF_PRINT_LINE_BUF        1024

/*****************************************************************************
 Function   : vos_assert
 Description: Report an assert
 Input      : pcFileName -- File name of this assert occuring
              ulLineNo   -- Line number of this assert occuring
 Output     : None
 Return     : None
 *****************************************************************************/
VOS_VOID vos_assert( VOS_UINT32 ulFileID, VOS_INT LineNo)
{
    /*lint -e813 */
    VOS_CHAR vos_PrintBuf[LENGTH_OF_PRINT_LINE_BUF];
    /*lint +e813 */

    VOS_CHAR *String = "Assert";

    /*lint -e534*/
    VOS_sprintf( vos_PrintBuf, "%s File: %d, Line: %d", String, ulFileID, LineNo );

    vos_printf("\n %s.\r\n",vos_PrintBuf);
    /*lint +e534*/

    return;
}

/*****************************************************************************
 Function   : PrintHexBuf
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID PrintHexBuf(char *PrintBuf, char *MemBuf, int Length)
{
    /*char vos_PrintBuf[LENGTH_OF_PRINT_LINE_BUF];*/
    int i,HalfByte;
    char *ptrPrintBuf;
    unsigned char *Buf;

    if(Length >= LENGTH_OF_PRINT_LINE_BUF/3-1)
        return;

    ptrPrintBuf = PrintBuf;
    Buf = (unsigned char*)MemBuf;
    for(i=0;i<Length;i++)
    {
        HalfByte = Buf[i]/16;
        if(HalfByte < 0x0A)
            *ptrPrintBuf = (char)(HalfByte + '0');
        else
           *ptrPrintBuf = (char)(HalfByte - 0x0A + 'A');
        ptrPrintBuf++;

        HalfByte = Buf[i]%16;
        if(HalfByte < 0x0A)
            *ptrPrintBuf = (char)(HalfByte + '0');
        else
            *ptrPrintBuf = (char)(HalfByte - 0x0A + 'A');
        ptrPrintBuf++;

        *ptrPrintBuf = ' ';
        ptrPrintBuf++;
    }

    *ptrPrintBuf = '\0';
    return;
}

/*****************************************************************************
 Function   : PrintMemBuf
 Description:
 Input      :
            :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID PrintMemBuf(char *String ,char *Buf, int Length)
{
    /*lint -e813 */
    char vos_PrintBuf[LENGTH_OF_PRINT_LINE_BUF];
    /*lint +e813 */
    int iNumber;

    /* lint -e534 */
    /* iNumber = sprintf( vos_PrintBuf, "\n%s",String); */

    iNumber = VOS_sprintf( vos_PrintBuf, "\n%s",String);
    
    PrintHexBuf(&vos_PrintBuf[iNumber], Buf, Length);

    /*lint -e534*/
    vos_printf("\n%s.\r\n", vos_PrintBuf);
    /*lint +e534*/

    return;
}






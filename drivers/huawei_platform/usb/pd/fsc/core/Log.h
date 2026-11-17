/*
 * Log.h
 *
 * Log driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifdef FSC_DEBUG

#ifndef FSC_LOG_H
#define	FSC_LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "platform.h"

#define LOG_SIZE 64

typedef struct{
    FSC_U16 state;
    FSC_U16 time_ms;
    FSC_U16 time_s;
} StateLogEntry;

typedef struct{
    StateLogEntry logQueue[ LOG_SIZE ];
    FSC_U8 Start;
    FSC_U8 End;
    FSC_U8 Count;
} StateLog;

void InitializeStateLog(StateLog *log);
FSC_BOOL WriteStateLog(StateLog *log, FSC_U16 state, FSC_U16 time_ms, FSC_U16 time_s);
FSC_BOOL ReadStateLog(StateLog *log, FSC_U16 * state, FSC_U16 * time_ms, FSC_U16 * time_s);
FSC_BOOL IsStateLogFull(StateLog *log);
FSC_BOOL IsStateLogEmpty(StateLog *log);
void DeleteStateLog(StateLog *log);

#ifdef	__cplusplus
}
#endif // __cplusplus

#endif	/* FSC_LOG_H */

#endif // FSC_DEBUG

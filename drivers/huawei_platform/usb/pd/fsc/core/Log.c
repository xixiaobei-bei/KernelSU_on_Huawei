/*
 * Log.c
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

#include "Log.h"

void InitializeStateLog(StateLog *log)
{
    log->Count = 0;
    log->End = 0;
    log->Start = 0;
}

FSC_BOOL WriteStateLog(StateLog *log, FSC_U16 state, FSC_U16 time_ms, FSC_U16 time_s)
{
    if(!IsStateLogFull(log))
    {
        FSC_U8 index = log->End;
        log->logQueue[index].state = state;
        log->logQueue[index].time_ms = time_ms;
        log->logQueue[index].time_s = time_s;

        log->End += 1;
        if(log->End == LOG_SIZE)
        {
            log->End = 0;
        }

        log->Count += 1;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

FSC_BOOL ReadStateLog(StateLog *log, FSC_U16 * state, FSC_U16 * time_ms, FSC_U16 * time_s) // Read first log and delete entry
{
    if(!IsStateLogEmpty(log))
    {
        FSC_U8 index = log->Start;
        *state = log->logQueue[index].state;
        *time_ms = log->logQueue[index].time_ms;
        *time_s = log->logQueue[index].time_s;

        log->Start += 1;
        if(log->Start == LOG_SIZE)
        {
            log->Start = 0;
        }

        log->Count -= 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

FSC_BOOL IsStateLogFull(StateLog *log)
{
    return (log->Count == LOG_SIZE) ? TRUE : FALSE;
}

FSC_BOOL IsStateLogEmpty(StateLog *log)
{
    return (!log->Count) ? TRUE : FALSE;
}

void DeleteStateLog(StateLog *log)
{
}

#endif // FSC_DEBUG

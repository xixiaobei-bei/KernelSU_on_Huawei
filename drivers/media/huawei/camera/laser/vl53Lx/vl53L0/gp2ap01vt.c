/* drivers/input/misc/gp2ap01vt00f.c - GP2AP01VT00F v1.1.0 time of flight sensor driver
 *
 * Copyright (C) 2017 Sharp Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <media/media-entity.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-of.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/camera.h>

/*
 * power specific includes
 */
#include <linux/pwm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>

#include "gp2ap01vt.h"
#include <media/huawei/laser_cfg.h>
#include "hw_laser_i2c.h"
//lint -save -e573 -e574

enum{
    XSDN,
    INTR,
    GPIO_MAX,
};

enum{
    LASER_IOVDD,
    LASER_AVDD,
    VDD_MAX,
};

#ifdef DEBUG_TIME_LOG
struct timeval start_tv, stop_tv;
#endif

#define LASER_PRINTCYLE 30 //for print laser data
#define MSLEEP_25MS 25
#define RAW_DATA_LEN 27
#define POLL_DELAY_TIME 33
#define MSEC2USEC 1000
#define METER2MILLIMETER 1000

//#define USE_INT
static void DataInit(struct gp2ap01vt00f_data *data);
static void ReadRangeData(struct gp2ap01vt00f_data *data);
static void gp2ap01vt00f_cancel_handler(struct gp2ap01vt00f_data *data);

extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static int gp2ap01vt00f_i2c_read(u8 reg, unsigned char *rbuf, int len, struct i2c_client *client)
{
    int err = -1;
    struct i2c_msg  i2cMsg[2];
    uint8_t buff = 0;

    if (client == NULL) {
    	return -ENODEV;
    }

    i2cMsg[0].addr = client->addr;
    i2cMsg[0].flags = 0;
    i2cMsg[0].len = 1;
    i2cMsg[0].buf = &buff;
    buff = reg;
    i2cMsg[1].addr = client->addr;
    i2cMsg[1].flags = I2C_M_RD;
    i2cMsg[1].len = len;
    i2cMsg[1].buf = rbuf;

    err = i2c_transfer(client->adapter, &i2cMsg[0], 2);
    if (err < 0) {
        laser_err("i2c transfer error(%d)!", err);
    }
    return err;
}

static int gp2ap01vt00f_i2c_write(u8 reg, u8 *wbuf, struct i2c_client *client)
{
    int err = 0;
    struct i2c_msg  i2cMsg = {0};
    unsigned char   buff[2] = {0};
    int retry = 10;

    if (client == NULL) {
        return -ENODEV;
    }

    while (retry--) {
        buff[0] = reg;
        buff[1] = *wbuf;

        i2cMsg.addr = client->addr;
        i2cMsg.flags = 0;
        i2cMsg.len = 2;
        i2cMsg.buf = buff;
        err = i2c_transfer(client->adapter, &i2cMsg, 1);
        laser_dbg("gp2ap01vt00f_i2c_write : 0x%x, 0x%x\n", reg, *wbuf);
        if (err >= 0) {
            return 0;
        }
    }
    laser_err("i2c transfer error(%d)!", err);
    return err;
}

#ifdef DEBUG_TIME_LOG
static void gp2ap01vt00f_DebugTimeGet(struct timeval *ptv)
{
    do_gettimeofday(ptv);
}

static void gp2ap01vt00f_DebugTimeDuration(struct timeval *pstart_tv,
			struct timeval *pstop_tv)
{
    if(pstart_tv ==NULL || pstop_tv==NULL)
        return;
    long total_sec, total_msec;
    total_sec = pstop_tv->tv_sec - pstart_tv->tv_sec;
    total_msec = (pstop_tv->tv_usec - pstart_tv->tv_usec)/MSEC2USEC;
    total_msec += total_sec * MSEC2USEC;
    laser_dbg("elapsedTime:%ld\n", total_msec);
}
#endif

/* initialize parameters */
void initGPparams(struct gp2ap01vt00f_data *data)
{
    data->range_result       = 0;
    data->range_result2	     = 0;
    data->ret_sig               = 0;
    data->ret_sig2             = 0;
    data->ref_sig               = 0;
    data->ret_amb             = 0;
    data->reg_error_status  = 0;
    data->error_status        = 0;
    data->range_raw          = 0;
    data->range_mm          = 0;
    data->range_result_ave = 0;
    data->ret_sig_ave         = 0;
    data->ret_amb_ave       = 0;
    data->num_measure      = 10;
}

static void SetDeviceMode(struct gp2ap01vt00f_data *data, u8 mode1, u8 mode2)
{
    /* operation mode, SINGLE_MODE / CONTINUOUS_MODE*/
    data->operation_mode = mode1;
    /* get mode, INTERRUPT_MODE / POLLING_MODE */
    data->get_mode = mode2;
}
/* Sesor initialization */
static void DataInit(struct gp2ap01vt00f_data *data)
{
    u8  wdata = 0;
    u8  rdata[2] = {0};
    if(data == NULL)
        return;
    laser_info("laser init enter");
    /* init parameters */
    initGPparams(data);

    /* Software reset */
    wdata = 0x01;
    gp2ap01vt00f_i2c_write(0x02, &wdata, data->client);

    if (data->get_mode == POLLING_MODE) {
        wdata = 0x00;
        gp2ap01vt00f_i2c_write(0x02, &wdata, data->client);
    } else if (data->get_mode == INTERRUPT_MODE) {
        wdata = 0x12;
        gp2ap01vt00f_i2c_write(0x02, &wdata, data->client);	/* pulse interrupt mode */
        // wdata = 0x10;    seq_i2c_write(slave_addr, 0x02, &wdata, 1); /* level interrupt mode */
    }

    wdata = 0x00;
    gp2ap01vt00f_i2c_write(0x03, &wdata, data->client);
    wdata = 0xE9;
    gp2ap01vt00f_i2c_write(0x04, &wdata, data->client);
    wdata = 0x1F;
    gp2ap01vt00f_i2c_write(0x05, &wdata, data->client);
    wdata = 0x09;
    gp2ap01vt00f_i2c_write(0x06, &wdata, data->client);
    wdata = 0xB3;
    gp2ap01vt00f_i2c_write(0x07, &wdata, data->client);
    wdata = 0x00;
    gp2ap01vt00f_i2c_write(0x08, &wdata, data->client);
    wdata = 0x11;
    gp2ap01vt00f_i2c_write(0x0A, &wdata, data->client);
    wdata = 0x06;
    gp2ap01vt00f_i2c_write(0x0B, &wdata, data->client);
    wdata = 0x34;
    gp2ap01vt00f_i2c_write(0x0D, &wdata, data->client);
    wdata = 0x74;
    gp2ap01vt00f_i2c_write(0x46, &wdata, data->client);
    wdata = 0x5F;
    gp2ap01vt00f_i2c_write(0x47, &wdata, data->client);

    /* set trimmng data at GP2AP01VT00F */
    wdata = 0xC0;
    gp2ap01vt00f_i2c_write(0x00, &wdata, data->client);
    msleep(5);	/* 4msec */
    wdata = 0x00;
    gp2ap01vt00f_i2c_write(0x00, &wdata, data->client);
    gp2ap01vt00f_i2c_read(0x52, rdata, sizeof(rdata), data->client);

    data->c1 = rdata[0];
    data->c2 = rdata[1];
    laser_info("laser init:gp2ap01vt00f c1:%d, c2:%d", data->c1, data->c2);
}

static void StartMeasurement(struct gp2ap01vt00f_data *data)
{
    u8 wdata = 0;
    if(data ==NULL)
        return;
    laser_info("laser debug:  StartMeasurement, mode =%d", data->operation_mode);
    if (data->operation_mode == SINGLE_MODE) {
        wdata = 0x80;
    } else if (data->operation_mode == CONTINUOUS_MODE) {
        wdata = 0xC0;
    }
    gp2ap01vt00f_i2c_write(0x00, &wdata, data->client);
}

static void StopMeasurement(struct gp2ap01vt00f_data *data)
{
    u8 wdata = 0;
    if(data ==NULL)
        return;
    wdata = 0x00;
    gp2ap01vt00f_i2c_write(0x00, &wdata, data->client);
}

static int sensor_onoff(u8 onoff, struct gp2ap01vt00f_data *data)
{
    if(data ==NULL)
        return EINVAL;
    laser_info("laser debug:gp2ap01vt00f sensor_onoff = %d", onoff);
    data->fracount = 0;
    if (onoff == 1) {	/* continuous mode */
        if (data->enabled != 1) {
            mutex_lock(&data->work_mutex);
            StartMeasurement(data);
            mutex_unlock(&data->work_mutex);
            laser_info("laser:gp2ap01vt00f sensor enable!");
        }
    } else { /* shutdown */
        if (data->enabled != 0) {
            mutex_lock(&data->work_mutex);
            StopMeasurement(data);
            //disable_irq_wake(data->gp2ap01vt00f_irq);
            /* cancel work handler */
            gp2ap01vt00f_cancel_handler(data);
            mutex_unlock(&data->work_mutex);
            laser_info("laser :gp2ap01vt00f sensor disable!");
        }
    }
    data->enabled = onoff;
#ifndef USE_INT
    /* Unblock the thread execution */
    if (data->enabled)
        wake_up(&data->poll_thread_wq);
#endif
    return 0;
}

static int gp2ap01vt00f_laser_get_data(struct gp2ap01vt00f_data *data,
		void* p)
{
    unsigned int rtncount = 80;
    hwlaser_RangingData_L0_t rangeData = {0};
    hwlaser_RangingData_t* udata = NULL;

    if(data ==NULL ||p==NULL)
        return -1;
    rangeData.RangeDMaxMilliMeter = 0;
    udata = (hwlaser_RangingData_t*) p;
    rangeData.RangeMilliMeter = data->range_mm;
    rangeData.SignalRateRtnMegaCps = data->ret_sig;
    rangeData.AmbientRateRtnMegaCps = data->ret_amb;
    rangeData.RangeStatus = data->error_status;

    rangeData.TimeStamp = 0;
    rangeData.MeasurementTimeUsec = 0;
    rangeData.EffectiveSpadRtnCount = data->ret_sig/rtncount;
    if(data->fracount%LASER_PRINTCYLE == 0){
        laser_info("laser debug: distance =%d, status=%d", data->range_mm, data->error_status);
    }
    data->fracount++;
    memcpy_s(&(udata->u.dataL0), sizeof(hwlaser_RangingData_L0_t), &rangeData, sizeof(hwlaser_RangingData_L0_t));
    udata->v.dataExtL0.SignalRtnCounts20ns = data->ret_sig2;
    return 0;
}

/*sharp factory code for laser calib, no to change magic value*/
/* for calibration */
static void GetRangingMeasurementDataAve_xtalk_test(struct gp2ap01vt00f_data *data)
{
    int i = 0;
    u8 wdata = 0;
    if(data ==NULL)
        return;
    if (data->enabled == 1) {
        sensor_onoff(0, data);
    }

    /* 15nsec mode, roa:19.5msec */
    if (data->get_mode == INTERRUPT_MODE) {
        wdata = 0x00;
        gp2ap01vt00f_i2c_write(0x02, &wdata, data->client);	/* pulse interrupt mode */
    }

    wdata = 0x4F;
    gp2ap01vt00f_i2c_write(0x06, &wdata, data->client);
    wdata = 0x25;
    gp2ap01vt00f_i2c_write(0x0D, &wdata, data->client);

    StartMeasurement(data);
    msleep(MSLEEP_25MS); 	/* 25msec */

    data->range_result_ave = 0;
    data->ret_sig_ave = 0;
    data->ret_amb_ave = 0;

    for (i=0;i<data->num_measure;i++) {
        msleep(MSLEEP_25MS); 	/* 25msec */
        ReadRangeData(data);
        data->range_result_ave += data->range_result;
        data->ret_sig_ave += data->ret_sig;
        data->ret_amb_ave += data->ret_amb;
        //laser_info("laser :%d,%d,%d",data->range_result_ave, data->ret_sig_ave, data->ret_amb_ave);
    }
    data->range_result_ave  = data->range_result_ave  / data->num_measure;
    data->ret_sig_ave   = data->ret_sig_ave  / data->num_measure;
    data->ret_amb_ave   = data->ret_amb_ave  / data->num_measure;
    laser_info( "laser:%d,%d,%d",data->range_result_ave, data->ret_sig_ave, data->ret_amb_ave);

    StopMeasurement(data);

    /* Release test setting */
    if (data->get_mode == INTERRUPT_MODE) {
        wdata = 0x12;
        gp2ap01vt00f_i2c_write(0x02, &wdata, data->client);	/* pulse interrupt mode */
    }

    wdata = 0x0F;
    gp2ap01vt00f_i2c_write(0x06, &wdata, data->client);
    wdata = 0x34;
    gp2ap01vt00f_i2c_write(0x0D, &wdata, data->client);

    data->range_result = data->range_result_ave;
    data->ret_sig = data->ret_sig_ave;
    data->ret_amb = data->ret_amb_ave;
}

/* Get Data from sensor */
/*sharp factory code for laser calib, no to change magic value*/
static void ReadRangeData(struct gp2ap01vt00f_data *data)
{
    u8  rdata[RAW_DATA_LEN] = {0};
    u8  rdata_reg01h = 0;
    if(data ==NULL)
        return;
    mutex_lock(&data->work_mutex);
    gp2ap01vt00f_i2c_read(0x01, &rdata_reg01h, sizeof(rdata_reg01h), data->client);
    gp2ap01vt00f_i2c_read(0x0E, rdata, sizeof(rdata), data->client);
    mutex_unlock(&data->work_mutex);

    data->reg01h    = rdata_reg01h;
    data->reg10h    = rdata[2] & 0xf0;
    data->range_result  = ((rdata[2] & 0x0f) << 16)  | (rdata[1] << 8)  | rdata[0];
    data->ret_sig   = (rdata[5] << 16)  | (rdata[4] << 8)  | rdata[3];
    data->ref_sig   = (rdata[8] << 16)  | (rdata[7] << 8)  | rdata[6];
    data->ret_amb   = (rdata[11] << 16) | (rdata[10] << 8) | rdata[9];
    data->range_result2 = ((rdata[23] & 0x0f) << 16)  | (rdata[22] << 8)  | rdata[21];
    data->ret_sig2  = (rdata[26] << 16)  | (rdata[25] << 8)  | rdata[24];
    data->reg_error_status = (data->reg10h << 8) | data->reg01h;
    return;
}

/* func res = 100_0000 * sin() */
/*sharp factory code for laser calib, no to change magic value*/
static int32_t GP2AP01VT_approximate_sin(uint32_t range)
{
    int32_t coeffAB[15][2] = {
        {2759,     0},
        {2624, 13472},
        {2465, 37279},
        {2258, 78633},
        {2008,141334},
        {1718,228316},
        {1479,311919},
        {1310,375341},
        {1134,445549},
        { 953,522496},
        { 767,606063},
        { 578,696050},
        { 386,792184},
        { 191,894113},
        {  47,973574},
    };

    int32_t rangeConvX = 0;
    int32_t coeffC = 1;
    uint8_t indx = 0;
    int32_t ret = 0;

    if (range < 562) {
        rangeConvX = range;
        coeffC = 1;
    } else if (range < 1124) {
        rangeConvX = 1124 - range;
        coeffC = 1;
    } else if (range < 1686) {
        rangeConvX = range - 1124;
        coeffC = -1;
    } else if (range < 2248) {
        rangeConvX = 2248 - range;
        coeffC = -1;
    } else {
        rangeConvX = range - 2248;
        coeffC = 1;
    }

    if (rangeConvX < 100) {
        indx = 0;
    } else if (rangeConvX < 150) {
        indx = 1;
    } else if (rangeConvX < 200) {
        indx = 2;
    } else if (rangeConvX < 250) {
        indx = 3;
    } else if (rangeConvX < 300) {
        indx = 4;
    } else if (rangeConvX < 350) {
        indx = 5;
    } else if (rangeConvX < 375) {
        indx = 6;
    } else if (rangeConvX < 400) {
        indx = 7;
    } else if (rangeConvX < 425) {
        indx = 8;
    } else if (rangeConvX < 450) {
        indx = 9;
    } else if (rangeConvX < 475) {
        indx = 10;
    } else if (rangeConvX < 500) {
        indx = 11;
    } else if (rangeConvX < 525) {
        indx = 12;
    } else if (rangeConvX < 550) {
        indx = 13;
    } else {
        indx = 14;
    }

    ret = coeffC * (coeffAB[indx][0] * rangeConvX + coeffAB[indx][1] + 5) /10 ;
    return ret;
}
/*sharp factory code for laser calib, no to change magic value*/
static int32_t GP2AP01VT_approximate_exp(uint32_t range)
{
    int32_t coeffAB[17][2] = {
        {-660999,20500000},
        {-443568,18325685},
        {-297803,15410392},
        {-200079,12478661},
        {-134557, 9857799},
        { -90622, 7661057},
        { -61158, 5893195},
        { -41394, 4509711},
        { -28132, 3448806},
        { -19231, 2647630},
        { -13251, 2049692},
        {  -9231, 1607501},
        {  -6525, 1282773},
        {  -4700, 1045525},
        {  -3466,  872762},
        {  -2629,  747132},
        {   -811,  456390},
    };

    uint8_t indx = 0;
    int32_t ret = 0;

    if (range < 10) {
        indx = 0;
    } else if (range < 20) {
        indx = 1;
    } else if (range < 30) {
        indx = 2;
    } else if (range < 40) {
        indx = 3;
    } else if (range < 50) {
        indx = 4;
    } else if (range < 60) {
        indx = 5;
    } else if (range < 70) {
        indx = 6;
    } else if (range < 80) {
        indx = 7;
    } else if (range < 90) {
        indx = 8;
    } else if (range < 100) {
        indx = 9;
    } else if (range < 110) {
        indx = 10;
    } else if (range < 120) {
        indx = 11;
    } else if (range < 130) {
        indx = 12;
    } else if (range < 140) {
        indx = 13;
    } else if (range < 150) {
        indx = 14;
    } else if (range < 160) {
        indx = 15;
    } else {
        indx = 16;
    }

    ret = (coeffAB[indx][0]*(int32_t)range + coeffAB[indx][1]) / 1000;
    if (ret < 0)
        ret = 0;
    return ret;
}

/*sharp factory code for laser calib, no to change magic value*/
static void GP2AP01VT_apply_xtalk_compensation(struct gp2ap01vt00f_data *data)
{
    int32_t SignalCoefficientM = 0, SignalCoefficientS = 0;
    int32_t CompensationRange = 0;
    int32_t EfficientSignal = 0;
    if(data ==NULL)
        return;
    EfficientSignal = data->ret_sig + data->ret_amb * 2 /10 ;
    SignalCoefficientM = EfficientSignal * 100 / data->xtalk_calib_data - 100;
    SignalCoefficientS = 5509 * GP2AP01VT_approximate_sin((uint32_t)data->range_raw) / 1000;

    laser_dbg("ToFSensor: SignalCoefficientM = %d\n", SignalCoefficientM);
    laser_dbg("ToFSensor: SignalCoefficientS = %d\n", SignalCoefficientS);

    CompensationRange = 75 * SignalCoefficientS / SignalCoefficientM /1000 + data->range_raw;
    laser_dbg("ToFSensor: before range_mm = %d\n", (int)data->range_raw);
    data->range_mm = CompensationRange;
    laser_dbg("ToFSensor: after range_mm = %d\n", (int)data->range_mm);
}
/*sharp factory code for laser calib, no to change magic value*/
static void GP2AP01VT_CalculateDistance(struct gp2ap01vt00f_data *data)
{
    int32_t range_mm_cal = 0;
    int32_t range2_mm_cal = 0;
    int32_t signal0_amb1 = 0;
    int32_t signal0_amb2 = 0;
    int32_t soft_waf1 = 0;
    int32_t adjust_range1 = 0;
    int32_t adjust_range2 = 0;
    if(data ==NULL)
        return;
    #if 1
	/* Error flag */
    if ( ( (data->reg_error_status & 0x0010) ==  0x0010 ) ||		/* internal circuit error2 */
            ( (data->reg_error_status & 0x1000) ==  0x1000 ) ||    	/* internal circuit error1 */
                ( (data->reg_error_status & 0x0004) ==  0x0004 )   ) {  	/* vcsel short */
        data->error_status = HARDWARE_ERROR;
        data->range_mm = 8888;
        laser_err("laser debug:error = %d", data->reg_error_status);
        return;
    } else if ( (data->reg_error_status & 0x2000) ==  0x2000 ) {
        data->error_status = WAF_ERROR;							/* warp aroud filter */
        data->range_mm = 8888;
        laser_err("laser debug:error = %d", data->reg_error_status);
        return;
    }
    #endif
    /* Calculate distance */
    range_mm_cal  = (data->range_result - data->c1 * 256);
    range2_mm_cal = (data->range_result2 - data->c2 * 256);
    if (range_mm_cal > 100000) {
        adjust_range1 = 10000;
        range_mm_cal /= 100;
    } else {
        adjust_range1 = 1000000;
    }
    if (range2_mm_cal > 100000) {
        adjust_range2 = 10000;
        range2_mm_cal /= 100;
    } else {
        adjust_range2 = 1000000;
    }
    range_mm_cal  = 6532 * range_mm_cal / adjust_range1;
    range2_mm_cal = 8792 * range2_mm_cal / adjust_range2;

    /* Offset with panel */
    range_mm_cal  = range_mm_cal + data->offset_calib_data;
    range2_mm_cal = range2_mm_cal + data->offset_calib_data * 135 / 100;

    /* Signal check */
    signal0_amb1 = 349 + (2 * data->ret_amb / 10) + data->xtalk_calib_data;
    signal0_amb2 = (2 * data->ret_amb /10 ) + 27 * data->xtalk_calib_data / 10;
    //laser_info("range1_cal = %d, range2_cal = %d", range_mm_cal, range2_mm_cal);
    //laser_info("signal0_amb1 = %d, signal0_amb2 = %d", signal0_amb1, signal0_amb2);
    soft_waf1 = GP2AP01VT_approximate_exp(range2_mm_cal) + data->xtalk_calib_data * 713 / 1000;

    if (data->ret_amb > 10800) {    /* Too much ambient */
        data->range_mm = 8888;
        data->error_status = SIGNAL_ERROR;
    } else if ( (data->ret_sig > 60000) && ( (range_mm_cal >= 2100) || (range_mm_cal < 0) ) )	{   /* Close range */
        data->range_mm = 0;
        data->error_status = CLOSE_RANGE_ERROR;
    } else if (range_mm_cal < 0) {
        data->range_mm = 8888;
        data->error_status = SIGNAL_ERROR;
    } else if ( (data->ret_sig < signal0_amb1) || (data->ret_sig < signal0_amb2) ) {    /* Low return signal */
        data->range_mm = 8888;
        data->error_status = SIGNAL_ERROR;
    } else if ( (data->xtalk_calib_data != 0) && ( (data->ret_sig2 < soft_waf1) ||  /* Soft waf error */
            ( ( data->ret_sig2 < (330 + data->xtalk_calib_data * 713 / 1000) ) && (range2_mm_cal < 400) ) ) ) {
        data->range_mm = 8888;
        data->error_status = WAF_ERROR;
    } else {    /* Correct range*/
        data->range_raw = range_mm_cal;
        data->error_status = VALID_DATA;
    }

    /* Crosstalk compensation */
    if ( (data->xtalk_calib_data > 0) && (data->error_status == VALID_DATA) ) {
        GP2AP01VT_apply_xtalk_compensation(data);
    } else if (data->error_status == VALID_DATA) {
        data->range_mm = data->range_raw;
    }
    //laser_info("laser :error_status = %d, range_mm = %d\n", data->error_status, data->range_mm);

    /* more than MAX */
    if ( (data->range_mm > 2250) && (data->range_mm != 8888) ) {
        data->range_mm = 2250;
        data->error_status = SIGNAL_ERROR;
    }
    return;
}

/*sharp factory code for laser calib, no to change magic value*/
static uint8_t get_signal_judge_coefficientM(struct gp2ap01vt00f_data *data,
		int32_t targetDistance,
		int32_t SignalCoefficientS,
		int32_t *SignalCoefficientM)
{
    uint8_t Status = 0;
    const uint32_t vcsel_pulse = 6410;
    const uint32_t spad_pulse = 3700;
    // const float meas_pulse = 15.0;
    int32_t diffRangePs = 0;
    int32_t paramSoverM[2] = {0,0};
    int32_t bound[2] = {
        2710,
        4570
    };
    int32_t tmpint32 = 0;
    if(data ==NULL || SignalCoefficientM== NULL)
        return EINVAL;

    diffRangePs = (targetDistance - data->range_mm) * 1000 / 150;
    tmpint32 = (vcsel_pulse + spad_pulse)/2 - diffRangePs;
    tmpint32 *= tmpint32;

    paramSoverM[0] = 2 * diffRangePs;
    paramSoverM[1] = vcsel_pulse - tmpint32 / spad_pulse ;

    //laser_dbg("diffRange=%d,tempint32=%d\n", diffRangePs, tmpint32);
    //laser_dbg("SoverM0=%d,SoverM1=%d\n", paramSoverM[0], paramSoverM[1]);
    if (0 < paramSoverM[0] && paramSoverM[0] < bound[0] ) {
        *SignalCoefficientM = SignalCoefficientS / paramSoverM[0];
    } else if (bound[0] <= paramSoverM[1] && paramSoverM[1] < bound[1] ) {
        *SignalCoefficientM = SignalCoefficientS / paramSoverM[1];
    } else {
        laser_err("xtalk calibration error1!");
        Status = 1;
    }

    if (*SignalCoefficientM <= 100 )
        Status = 1;

    laser_dbg("CoeffM=%d\n",*SignalCoefficientM);
    return Status;
}

/*sharp factory code for laser calib, no to change magic value*/
static void GP2AP01VT_calc_xtalk_signal(struct gp2ap01vt00f_data *data, int32_t targetDistance)
{
    int32_t EfficientSignal_ave = 0;
    int32_t SignalCoefficientS = 0;
    int32_t SignalCoefficientM = 0;
    uint8_t Status = 0;
    if(data ==NULL)
        return;

    EfficientSignal_ave = data->ret_sig + 2 * data->ret_amb /10;
    SignalCoefficientS = 5509 * GP2AP01VT_approximate_sin((uint32_t)data->range_mm) / 1000;

    Status = get_signal_judge_coefficientM(data, targetDistance, SignalCoefficientS, &SignalCoefficientM);

    if (Status == 0) {
        data->xtalk_calib_data = EfficientSignal_ave * 100 / (100 + SignalCoefficientM) * 8507 / 19516;
    } else {
        data->xtalk_calib_data = 8888;
        data->error_status = 255;
    }
    /* EEPROM, Save xtalk calibration data on the host */
}

/* Gray Card 17%, distance:600mm, dark condition at factory
   targetDistance(mm) : Crosstalk value from the panel
   Please optical design so that crosstalk value is less than 200 counts */
void GP2AP01VT_PerformXTalkCalibration(struct gp2ap01vt00f_data *data, int32_t targetDistance)
{
    if(data ==NULL)
        return;
    /* 10 times average */
    GetRangingMeasurementDataAve_xtalk_test(data);

    data->xtalk_calib_data = 0;
    /* Calculate distance with average data */
    GP2AP01VT_CalculateDistance(data);

    if ((targetDistance == 0) ||
            (data->error_status != VALID_DATA) ||
            (data->range_mm >= targetDistance)) {
        laser_info("xtalk calibration range_mm:%d, targetDistance =%d", data->range_mm, targetDistance);
        data->xtalk_calib_data = 0;
    } else {
        GP2AP01VT_calc_xtalk_signal(data, targetDistance);
    }
    //GP2AP01VT_calc_xtalk_signal(data, targetDistance);
}

/* White card 88%, distance:100mm, dark condition at factory
 * targetDistance[mm] */
 /*sharp factory code for laser calib, no to change magic value*/
static void GP2AP01VT_PerformOffsetCalibration(struct gp2ap01vt00f_data *data, int32_t targetDistance)
{
    int32_t offset = 0;
    int32_t tmpxtalk = 0;
    if(data ==NULL)
        return;
    /* 10 times average */
    GetRangingMeasurementDataAve_xtalk_test(data);
    tmpxtalk = data->xtalk_calib_data;
    data->xtalk_calib_data = 0;
    data->offset_calib_data = 0;

    /* Calculate distance with average data */
    GP2AP01VT_CalculateDistance(data);
    data->xtalk_calib_data = tmpxtalk;

    if (data->range_mm < targetDistance)
        offset = ((10 * targetDistance - 10 * (int32_t)data->range_mm) + 5) / 10;
    else if (data->range_mm > targetDistance)
        offset = ((10 * targetDistance - 10 * (int32_t)data->range_mm) - 5) / 10;
    else
        offset = 0;

    data->offset_calib_data = offset;
    laser_info("range_result  offset = %d, ", offset);
}

static void gp2ap01vt00f_cancel_handler(struct gp2ap01vt00f_data *data)
{
    unsigned long flags = 0;
    bool ret = 0;
    if(data ==NULL)
        return;
    spin_lock_irqsave(&data->update_lock.wait_lock, flags);
    ret = cancel_delayed_work(&data->gp2ap01vt00f_int_work);
    if (ret == 0)
        laser_info("%s :cancel_delayed_work return self\n", __FUNCTION__);
    spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);
}

void gp2ap01vt00f_schedule_handler(struct gp2ap01vt00f_data *data)
{
    unsigned long flags = 0;
    if(data ==NULL)
        return;
    laser_dbg("%s(%d) : Starting schedule handler\n", __FUNCTION__, __LINE__);
    spin_lock_irqsave(&data->update_lock.wait_lock, flags);

    cancel_delayed_work(&data->gp2ap01vt00f_int_work);
    schedule_delayed_work(&data->gp2ap01vt00f_int_work, 0);
    spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);
}

#ifdef USE_INT
irqreturn_t gp2ap01vt00f_interrupt_handler(int irq, void *dev_id)
{
    if(dev_id ==NULL)
        return;
    struct gp2ap01vt00f_data   *data = dev_id;
    laser_dbg("gp2ap01vt00f_interrupt_handler\n");
    schedule_delayed_work(&data->gp2ap01vt00f_int_work, 0);
    return IRQ_HANDLED;
}
#else
/* Flag used to exit the thread when kthread_stop() is invoked */
static int poll_thread_exit = 0;
int gp2ap01vt00f_poll_thread(void *data)
{
    struct gp2ap01vt00f_data *gp2ap01vt_dev = NULL;
    uint32_t sleep_time = 0;
    if(data ==NULL)
        return EINVAL;
    gp2ap01vt_dev = data;
    laser_info("%s(%d) : Starting Polling thread\n", __FUNCTION__, __LINE__);

    while(!kthread_should_stop()) {
        /* Check if enable_ps_sensor is true or exit request is made. If not block */
        wait_event(gp2ap01vt_dev->poll_thread_wq,
                (gp2ap01vt_dev->enabled || poll_thread_exit));
        if (poll_thread_exit) {
            laser_err("%s(%d) : Exiting the poll thread", __FUNCTION__, __LINE__);
            break;
        }
        mutex_lock(&gp2ap01vt_dev->work_mutex);
        sleep_time = gp2ap01vt_dev->delay_ms;
        gp2ap01vt00f_schedule_handler(gp2ap01vt_dev);
        mutex_unlock(&gp2ap01vt_dev->work_mutex);
        /* Sleep for delay_ms milliseconds */
        msleep(sleep_time);
    }
    return 0;
}
#endif

static void gp2ap01vt00f_int_work_func(struct work_struct *work)
{
    struct timeval tv;
    struct gp2ap01vt00f_data  *data = NULL;
    if (work == NULL) {
        return;
    }    
    data = container_of((struct delayed_work *)work,
                                        struct gp2ap01vt00f_data, gp2ap01vt00f_int_work);

    do_gettimeofday(&tv);

#ifdef DEBUG_TIME_LOG
    gp2ap01vt00f_DebugTimeGet(&stop_tv);
    gp2ap01vt00f_DebugTimeDuration(&start_tv, &stop_tv);
#endif
    /* Read range data via i2c*/
    ReadRangeData(data);
    GP2AP01VT_CalculateDistance(data);
#ifdef DEBUG_TIME_LOG
    gp2ap01vt00f_DebugTimeGet(&start_tv);
#endif
}

int gp2ap01vt00f_power_on_off(struct gp2ap01vt00f_data *data, int is_on)
{
    int rc = 0;
    struct i2c_data *i2cdata = NULL;
    if(NULL != data && NULL != data->client_object)
    {
        laser_info("laser:power_on_off  is_on=%d\n", is_on);
        i2cdata = (struct i2c_data *) data->client_object;
        rc = gpio_direction_output(i2cdata->xsdn_gpio, is_on);
        if (rc) {
            laser_err("fail to power control laser %d\n", rc);
        }
        msleep(POWER_SLEEP_TIME);
    }
    return rc;
}

static int gp2ap01vt00f_laser_get_set_cal_data(struct gp2ap01vt00f_data* data, hwlaser_calibration_data_t *p)
{
    int rc = 0;
    FixPoint1616_t *xtalk = NULL;
    int32_t *offset = NULL;
    uint32_t *pspadcount  = NULL;
    uint8_t *pspadmod  = NULL;
    uint8_t *pvhvset  = NULL;
    uint8_t *pphascal  = NULL;
    hwlaser_calibration_data_t *cal_data = (hwlaser_calibration_data_t*)p;
    if(!data || !p)
        return -EINVAL;
    offset = &(cal_data->u.dataL0.offset);
    xtalk = &(cal_data->u.dataL0.xtalk);
    pspadcount = &(cal_data->u.dataL0.spadcount);
    pspadmod = &(cal_data->u.dataL0.spadmod);
    pvhvset = &(cal_data->u.dataL0.vhvset);
    pphascal = &(cal_data->u.dataL0.phascal);

    if(!cal_data->is_read)
    {
	data->offset_calib_data = (*offset) / METER2MILLIMETER;
	data->xtalk_calib_data = *xtalk;
    laser_info("laser get offset =%d, xtalk value=%d\n", data->offset_calib_data, *xtalk);
    }
    else
    {
        *xtalk = data->xtalk_calib_data;
        *offset = data->offset_calib_data * METER2MILLIMETER; //millimeter
        laser_info("laser set offset =%d, xtalk value=%d\n", *offset,*xtalk);
        //for compatible
        *pspadcount = 0;
        *pspadmod = 0;
        *pvhvset = 0;
        *pphascal = 0;
    }
    return rc;
}

static int gp2ap01vt00f_get_id(struct gp2ap01vt00f_data* data, hwlaser_status_t *laser_status)
{
    int rc = -EINVAL;
    char* laser_name = "gp2ap01vt";
    if(NULL == data || NULL == laser_status)
        return rc;
    laser_info("laser get chip id ioctl\n");
    if(data->init_flag == 1){
        strncpy_s(laser_status->name, HWLASER_NAME_SIZE-1,laser_name, strlen(laser_name)+1);
        laser_status->status = 0;
    }else {
        laser_status->status = -1;
    }
    rc = laser_status->status;
    return rc;
}

static int gp2ap01vt00f_perform_cal(struct gp2ap01vt00f_data* data, hwlaser_ioctl_perform_calibration_t* cal)
{
    int rc = -EINVAL;
    if(NULL == data || NULL == cal)
        return rc;

    switch(cal->calibration_type){
        case HWLASER_CALIBRATION_CROSSTALK:
            laser_info("laser: xtalk TargetDistance[mm]: %d", cal->param1);
	     GP2AP01VT_PerformXTalkCalibration(data, cal->param1);
            rc = 0;
            break;
        case HWLASER_CALIBRATION_OFFSET:
            laser_info("laser: offset TargetDistance[mm]: %d", cal->param1);
	     GP2AP01VT_PerformOffsetCalibration(data, cal->param1);            rc = 0;
            break;
         default:
            rc = 0;
            break;
    }
    return rc;
}

static int gp2ap01vt00f_laser_parameter(struct gp2ap01vt00f_data *data, hwlaser_parameter_t* param)
{
    int rc = 0;
    if(NULL == data || NULL == param)
        return -EINVAL;

    switch(param->name) {
        case (DEVICEMODE_PAR):
            if (param->is_read) {
                param->value = 1;
                param->status = 0;
            } else {
                param->status =rc;
            }
            break;
        case OFFSET_PAR:
            rc = 0;
            break;
        case XTALKRATE_PAR:
            if (param->is_read) {
                param->value =  data->xtalk_calib_data;
                param->status = 0;
            }else{
                data->xtalk_calib_data = param->value;
            }
            rc = param->status;
            break;
        case TIMINGBUDGET_PAR:
            if (param->is_read) {
                param->value = data->timingBudget/MSEC2USEC;
                param->status = 0;
            } else {
                data->timingBudget = param->value*MSEC2USEC;
            }
            rc = param->status;
            break;
        case DISTANCEMODE_PAR:
            rc = 0;
            break;
        case SIGMALIMIT_PAR:
            if (param->is_read) {
                param->value = data->sigmaLimit;
                param->status = 0;
            } else {
                data->sigmaLimit = param->value;
                laser_info("laser sigmaLimit=%d", data->sigmaLimit);
            }
            rc = param->status;
            break;
        case SIGNALLIMIT_PAR:
            if (param->is_read) {
                param->value = data->signalRateLimit;
                param->status = 0;
            } else {
                data->signalRateLimit = param->value;
                laser_info("laser signalRateLimit=%d", data->signalRateLimit);
            }
            rc = param->status;
            break;
        case RIT_PAR:
            if (param->is_read) {
                param->value = data->ritEnable;
                param->value2 = data->ritValue;
                param->status = 0;
            } else {
                data->ritEnable = param->value;
                data->ritValue = param->value2;
                laser_info("laser ritValue=%d, ritEnable=%d", data->ritValue, data->ritEnable);
            }
            rc = param->status;
            break;
        case NVM_PAR:
            rc = 0;
            break;
        default:
            rc = 0;
            break;
        }
    return rc;
}

long gp2ap01vt00f_laser_ioctl(void *hw_data, unsigned int cmd, void  *p)
{
    struct gp2ap01vt00f_data *data = (struct gp2ap01vt00f_data*)hw_data;
    hwlaser_calibration_data_t *cal_data = NULL;
    hwlaser_parameter_t *param = NULL;
    hwlaser_status_t* status = NULL;
    hwlaser_ioctl_perform_calibration_t* cal_perform = NULL;
    int rc = 0;
    hwlaser_info_t *pinfo = NULL;

    if (!data)
        return -EINVAL;

    switch (cmd) {
        case HWLASER_IOCTL_GET_INFO:
            pinfo = (hwlaser_info_t *)p;
            strncpy_s(pinfo->name, HWLASER_NAME_SIZE-1, "gp2ap01vt00f", HWLASER_NAME_SIZE-1);
            strncpy_s(pinfo->product_name, HWLASER_NAME_SIZE-1, "ALPS", HWLASER_NAME_SIZE-1);
            pinfo->version = HWLASER_SHARP_L0_VERSION;
            pinfo->ap_pos = HWLASER_POS_AP;
            break;
        case HWLASER_IOCTL_CONFIG:
            break;

        case HWLASER_IOCTL_PERFORM_CALIBRATION:
            cal_perform = (hwlaser_ioctl_perform_calibration_t*) p;
            rc = gp2ap01vt00f_perform_cal(data, cal_perform);
            laser_info("laser:perform calibration end");
            break;
        case HWLASER_IOCTL_POWERON:
            rc = gp2ap01vt00f_power_on_off(data, true);
            break;
        case HWLASER_IOCTL_POWEROFF:
            rc = gp2ap01vt00f_power_on_off(data, false);
            break;
        case HWLASER_IOCTL_INIT:
            laser_info("laser data init ioctl");
             DataInit(data);
            break;
        case HWLASER_IOCTL_CALIBRATION_DATA:
            laser_info("HWLASER_IOCTL_CALIBRATION_DATA");
            cal_data = (hwlaser_calibration_data_t *)p;
            rc = gp2ap01vt00f_laser_get_set_cal_data(data,cal_data);
            break;
         case HWLASER_IOCTL_START:
            sensor_onoff(1, data);
            break;
        case HWLASER_IOCTL_STOP:
            if (data->enabled == 1){
                sensor_onoff(0, data);
            }
            break;
        case HWLASER_IOCTL_ROI:
                laser_info("set roi not support");
            break;
        case HWLASER_IOCTL_PARAMETER:
            param = (hwlaser_parameter_t *)p;
            rc = gp2ap01vt00f_laser_parameter(data,param);
            break;
        case HWLASER_IOCTL_MZ_DATA:
            rc = gp2ap01vt00f_laser_get_data(data,p);
            break;
        case HWLASER_IOCTL_MATCHID:
            status = (hwlaser_status_t*) p;
            rc = gp2ap01vt00f_get_id(data, status);
            break;
        default:
            laser_info("cmd not support!!");
            rc = -EINVAL;
            break;
    }
    return rc;
}

static int gp2ap01vt00f_setup(struct gp2ap01vt00f_data *data)
{
    int rc = 0;
    if(NULL == data || NULL == data->client_object)
    {
        laser_info("data NULL");
        return -EINVAL;
    }

    data->enabled = 0;
    data->delay_ms = POLL_DELAY_TIME;
    data->flushCount = 0;
    /* init mutex */
    mutex_init(&data->update_lock);
    mutex_init(&data->work_mutex);
    /* init work handler */
    INIT_DELAYED_WORK(&data->gp2ap01vt00f_int_work, gp2ap01vt00f_int_work_func);

#ifdef USE_INT
	/* irq */
    laser_dbg("gp2ap01vt00f->gp2ap01vt00f_irq %d \n", gp2ap01vt00f->irq);
    rc = request_irq((unsigned int)(data->gp2ap01vt00f_irq), gp2ap01vt00f_interrupt_handler,
                                                        IRQF_TRIGGER_FALLING, "gp2ap01vt00f_int", (void *)data);

    if (rc < 0) {
        laser_info("allocate gp2ap01vt00f int fail ! result:%d\n",  rc);
        free_irq(data->gp2ap01vt00f_irq, data);
        return rc;
    }
    disable_irq(data->gp2ap01vt00f_irq);
#else
    init_waitqueue_head(&data->poll_thread_wq);
    data->poll_thread = kthread_run(&gp2ap01vt00f_poll_thread,
                                                                            (void *)data,
                                                                    "SHARP-GP2AP01VT");
    if (data->poll_thread == NULL) {
        laser_err("%s(%d) - Failed to create Polling thread", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    return rc;
}

int gp2ap01vt00f_data_init(void** pdata, struct i2c_data *i2c_object)
{
    int rc = 0;
    struct gp2ap01vt00f_data *gp2ap01vt_data = NULL;
    if(pdata == NULL || i2c_object == NULL)
        return -EINVAL;

    gp2ap01vt_data = kzalloc(sizeof(struct gp2ap01vt00f_data), GFP_KERNEL);
    if (!gp2ap01vt_data) {
        rc = -ENOMEM;
        return rc;
    }
    gp2ap01vt_data->client_object = (void*)i2c_object;
    gp2ap01vt_data->client = i2c_object->client;

    /* setup device name */
    gp2ap01vt_data->dev_name = dev_name(&i2c_object->client->dev);
    gp2ap01vt_data->gp2ap01vt00f_irq = i2c_object->irq;
    gp2ap01vt_data->init_flag = 1;
    rc = gp2ap01vt00f_setup(gp2ap01vt_data);
    if(rc) {
        laser_info("gp2ap01 setup fail");
        kfree(gp2ap01vt_data);
        return rc;
    }
    SetDeviceMode(gp2ap01vt_data, CONTINUOUS_MODE, POLLING_MODE);
    *pdata = (void *)gp2ap01vt_data;
    i2c_set_clientdata(i2c_object->client, gp2ap01vt_data);
    return rc;
}

int gp2ap01vt00f_laser_remove(struct i2c_client *client)
{
    struct gp2ap01vt00f_data *gp2ap01vt_data =  NULL;
    if(client == NULL)
        return -EINVAL;
    gp2ap01vt_data =  i2c_get_clientdata(client);
#ifndef USE_INT
    poll_thread_exit = 1;
    kthread_stop(gp2ap01vt_data->poll_thread);
#endif
    kfree(gp2ap01vt_data);
    client->adapter = NULL;
    return 0;
}
//lint -restore

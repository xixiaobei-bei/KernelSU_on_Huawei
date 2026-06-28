/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "../hisi_fb_def.h"
#include "../dp/dp_aux.h"
#include "hisi_fb.h"
#include "../dp/intr.h"
#include "hdcp13.h"

#define SHA1_MAX_LENGTH 650    //635+2+8

uint8_t g_hdcp_mode = 0;
uint32_t link_error_count=0;
uint64_t g_BKSV=0;
uint8_t g_bcaps = 0;


static uint8_t g_dp_on = 0;
static uint8_t g_Binfo[2];
//static uint8_t g_sha1_buffer[SHA1_MAX_LENGTH];
//static uint8_t V_prime[20];
static struct task_struct *hdcp_polling_task = NULL;
static uint8_t hdcp_polling_flag = 0;
static uint8_t hdcp_stop_polling = 0;




static int GetKSVListFromDPCD(struct dp_ctrl *dptx, uint8_t* sha1_buffer, uint32_t* plength)
{
    uint32_t dev_count,  i;
    uint8_t temp[16];
    uint32_t ptr=0;
    uint8_t* pKSVList;
    uint32_t len = 10;
    int retval;

    if ((dptx == NULL) || (sha1_buffer == NULL)) {
        HISI_FB_ERR("NULL Pointer\n");
        return -1;
    }

    pKSVList = sha1_buffer;

    retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd Binfo, retval=%d.\n", retval);
        return retval;
    }
    HISI_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0],g_Binfo[1]);
    dev_count = g_Binfo[0];

    while (dev_count>0)
    {
        if (dev_count >=3)
        {
            retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, 15);
            if (retval) {
                HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
                return retval;
            }
            for(i=0; i<15; i++)
            {
                pKSVList[ptr++] = temp[i];
            }
            dev_count -= 3;
        }
        else
        {
            retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, dev_count*5);
            if (retval) {
                HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
                return retval;
            }
            for(i=0; i<dev_count*5; i++)
            {
                pKSVList[ptr++] = temp[i];
            }
            dev_count = 0;
        }
    }
    len += ptr;
    HISI_FB_DEBUG("Read %d KSV:\n", ptr);
    pKSVList[ptr++] = g_Binfo[0];
    pKSVList[ptr++] = g_Binfo[1];

    /*for(i=0; i<8; i++)
    {
        pKSVList[ptr++]  = (M0>>(i*8)) & 0xFF;
    }*/

    //test print
    //HISI_FB_ERR("sha1_buffer:\n");
    //for (i=0; i<len; i++)
    //    HISI_FB_ERR("%d:%x\n", i,pKSVList[i]);

    *plength = len;
    return 0;
}


static int Read_V_prime_fromDPCD(struct dp_ctrl *dptx, uint8_t* pV_prime)
{
    int retval;

    if ((dptx == NULL) || (pV_prime == NULL)) {
        HISI_FB_ERR("NULL Pointer\n");
        return -1;
    }

    retval = dptx_read_bytes_from_dpcd(dptx, 0x68014, &pV_prime[0], 16);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
        return retval;
    }
    retval = dptx_read_bytes_from_dpcd(dptx, 0x68024, &pV_prime[16], 4);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
        return retval;
    }

    return 0;
}


/////////////////////////////////////////
//extern API
/////////////////////////////////////////
int HDCP_Read_TEInfo(struct dp_ctrl *dptx)
{
    uint8_t temp[16];
    uint32_t i=0;
    int retval;

    if (dptx == NULL) {
        HISI_FB_ERR("NULL Pointer\n");
        return -1;
    }

    retval = dptx_read_bytes_from_dpcd(dptx, 0x68000, temp, 5);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd bKSV, retval=%d.\n", retval);
        return retval;
    }
    g_BKSV = 0;
    for(i=0; i<5; i++)
    {
        g_BKSV += temp[4-i];
        //HISI_FB_DEBUG(" BKSV: %d: 0x%x\n", 4-i, temp[4-i]);
        if(i<4)
            g_BKSV <<= 8;
    }
    //HISI_FB_DEBUG("dptx_read BKSV: 0x%lx\n", g_BKSV);

    retval = dptx_read_dpcd(dptx, 0x68028, &g_bcaps);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd bcaps, retval=%d.\n", retval);
        return retval;
    }
    HISI_FB_DEBUG("dptx_read bcaps:%x!\n", g_bcaps);

    //dptx_read_dpcd(dptx, 0x68029, &bstatus);

    retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
    if (retval) {
        HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd binfo, retval=%d.\n", retval);
        return retval;
    }
    HISI_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0],g_Binfo[1]);

    //dptx_writel(dptx, 0x3608, 0x1);     //use manual An temply, need add random number
 
    link_error_count=0;
    HDCP_Stop_Polling_task(0);
    g_hdcp_mode = 0;

    return 0;
}


void HDCP13_enable(struct dp_ctrl *dptx, bool en)
{
    if (!en)
    {
        link_error_count=0;
        HDCP_Stop_Polling_task(0);
    }
    if (dptx == NULL) {
        HISI_FB_ERR("NULL Pointer\n");
        return;
    }

    if(dptx->hisifd->secure_ctrl.hdcp13_enable)
        dptx->hisifd->secure_ctrl.hdcp13_enable(en);
    else
        HISI_FB_ERR("[HDCP] ATF:hdcp13_enable is NULL\n");
}

void HDCP_DP_on(bool en)
{
    g_dp_on = en;
    if (hdcp_polling_task) {
        kthread_stop(hdcp_polling_task);
        hdcp_polling_task = NULL;
        hdcp_polling_flag = 0;
    }
}

void HDCP_Stop_Polling_task(uint8_t stop)
{
    hdcp_stop_polling = stop;
}



int HDCP_GetSHA1Buffer(uint8_t* pSHA1buffer, uint32_t* plength, uint8_t* pV_prime)
{
    struct hisi_fb_data_type *hisifd;
    struct dp_ctrl *dptx;
  
    if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
    }
 
    hisifd = platform_get_drvdata(g_dp_pdev);    
    if (hisifd == NULL) {
            HISI_FB_ERR("hisifd is NULL!\n");
            return -1;
    }
    dptx = &(hisifd->dp);

    if (GetKSVListFromDPCD(dptx, pSHA1buffer, plength))
    {
        HISI_FB_ERR("GetKSVListFromDPCD failed!\n");
        return -1;
    }

    //Read V'
    if (Read_V_prime_fromDPCD(dptx, pV_prime))
    {
        HISI_FB_ERR("Read_V_prime_fromDPCD failed!\n");
        return -1;
    }
    /*for (i=0; i<20; i++)
    {
        HISI_FB_DEBUG("%d:0x%x\n", i, V_prime[i]);
    }*/

    return 0;
}

static int hdcp_polling_thread(void *p)
{
    uint32_t i=0;
    uint32_t temp_value;
    struct hdcp_params *hparams;
    struct dp_ctrl *dptx;

    dptx = (struct dp_ctrl *)p;
    hparams = &dptx->hparams;

    while(!kthread_should_stop()) {
        msleep(1000);
        if (hdcp_polling_flag)
        {
            if (!g_dp_on)
            {
                HISI_FB_WARNING("dp is off, no access hdcp register!\n");
                hdcp_polling_flag = 0;
                i = 0;
                continue;
            }

            if (hdcp_stop_polling)
            {
                hdcp_stop_polling = 0;
                HISI_FB_DEBUG("Auth already have result, stop polling!\n");
                hdcp_polling_flag = 0;
                i = 0;
                continue;
            }

            if (link_error_count >= MAX_LINK_ERROR_COUNT)
            {
                HISI_FB_INFO("Auth Repeater fail, Re-try auth mannually!!\n");
                HDCP13_enable(dptx, 0);
                msleep(10);
                HDCP13_enable(dptx, 1);
                i = 0;
                continue;
            }

            if (dptx->hisifd->secure_ctrl.hdcp_reg_get)
                temp_value = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_OBS);
            else
            {
                hdcp_polling_flag = 0;
                i = 0;
                continue;
            }
            HISI_FB_DEBUG("Current state is:0x%x!\n", temp_value);
            if (((temp_value & 0xFFFF) != REPEATER_STATE) && (i<6) )
            {
                i++;
                continue;
            }

            if ((temp_value & 0xFFFF) != REPEATER_STATE)
            {
                HISI_FB_WARNING("Not stop at repeater state A9, at 0x%x!!!\n", temp_value);
                hparams->auth_fail_count++;
                //disable output
                HDCP13_enable(dptx, 0);
                if (hparams->auth_fail_count > DPTX_HDCP_MAX_REPEATER_AUTH_RETRY)
                {
                    if(dptx->hisifd->secure_ctrl.hdcp_enc_mode)
                        dptx->hisifd->secure_ctrl.hdcp_enc_mode(1);
                    hdcp_polling_flag = 0;
                    HISI_FB_ERR("Disable DP output becasue of reach max allowed retries count=%d.\n", hparams->auth_fail_count);
                }
                else
                {
                    HISI_FB_INFO("Repeater state error, Re-try auth again:%d!!\n", hparams->auth_fail_count);
                    msleep(10);
                    HDCP13_enable(dptx, 1);
                }
                i = 0;
                continue;
            }

            HISI_FB_INFO("Stop at repeater state A9!!!\n");
            switch_set_state(&dptx->sdev, Hot_Plug_HDCP13);
            hdcp_polling_flag = 0;
            i = 0;
        }
    }
    return 0;
}

int HDCP13_WaitAuthenticationStop(void)
{
    struct hisi_fb_data_type *hisifd;
    struct dp_ctrl *dptx;
    //uint32_t i=0;
    //uint32_t temp_value;

    if (!(g_bcaps&0x2)) {
		HISI_FB_INFO("bcaps is receiver!\n");
		return 0;
    }

    if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
    }

    hisifd = platform_get_drvdata(g_dp_pdev);
    if (hisifd == NULL) {
            HISI_FB_ERR("hisifd is NULL!\n");
            return -1;
    }
    dptx = &(hisifd->dp);

    //wait for HW auth stop
    if (!hdcp_polling_task) {
        hdcp_polling_task = kthread_create(hdcp_polling_thread, dptx, "hdcp_polling_task");
        if(IS_ERR(hdcp_polling_task)) {
            HISI_FB_ERR("Unable to start kernel hdcp_polling_task./n");
            hdcp_polling_task = NULL;
            return -2;//-EINVAL;
        }
    }
    hdcp_polling_flag = 1;
    wake_up_process(hdcp_polling_task);
    /*do
    {
        msleep(1000);
        i++;
        if (!g_dp_on)
        {
            HISI_FB_WARNING("dp is off, no access hdcp register!\n");
            return -1;
        }
        temp_value = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_OBS);
        HISI_FB_DEBUG("Current state is:%x!\n", temp_value);
    }while (((temp_value & 0xFFFF) != REPEATER_STATE) && (i<6));

    if ((temp_value & 0xFFFF) != REPEATER_STATE)
    {
        HISI_FB_WARNING("Not stop at repeater state A9!!!\n");
        if (link_error_count >= MAX_LINK_ERROR_COUNT)
        {
            HISI_FB_INFO("Re-try auth mannually!!\n");
            HDCP13_enable(dptx, 0);
            msleep(10);
            HDCP13_enable(dptx, 1);
            return 0;
        }
        //HDCP13_enable(dptx, 0);
        return -2;
    }
    HISI_FB_INFO("Stop at repeater state A9!!!\n");
#ifdef CONFIG_SWITCH
    switch_set_state(&dptx->sdev, Hot_Plug_HDCP13);
#endif*/

    return 0;
}





#ifdef _PRE_WLAN_RF_110X_CALI_DPD
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_cali_dpd.h"
#include "oam_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_DPD_C

/*****************************************************************************
  2 函数声明
*****************************************************************************/
#define DPD_CALI_GAIN_REF_IDX 49
#define DPD_CALI_AMP_REF_RMS 16
#define LUT_End_Point 10

#define GET2COMP(a) (a >= 1024) ? (a - 2048) : (a)
#define SET2COMP(a) (a >= 0) ? (a) : (2048 + a)
#define absd(a)     (a >= 0) ? (a) : (-a)

oal_uint16 g_st_LUT_Gain[DPD_CALI_LUT_LENGTH],
oal_uint16 g_st_DPD_Gain[DPD_CALI_LUT_LENGTH];
hi1102_complex_stru g_st_LUT_Theta[DPD_CALI_LUT_LENGTH];
hi1102_complex_stru g_st_DPD_Theta[DPD_CALI_LUT_LENGTH];

/*****************************************************************************
  3 函数实现
*****************************************************************************/

OAL_STATIC  oal_int32 hi1102_cali_div_round_closest(oal_int32 l_a, oal_int32 l_b)
{
    oal_int32 l_result = 0;
    if ((l_b == 0) || ((l_b << 1) == 0)) {
        l_result = (oal_int32)l_a;
    } else {
        if (((l_a >= 0) && (l_b > 0)) || ((l_a <= 0) && (l_b < 0))) {
            l_result = (oal_int32)(((l_a << 1) + l_b) / (l_b << 1));
        } else {
            l_result = (oal_int32)(((l_a << 1) - l_b) / (l_b << 1));
        }
    }

    return l_result;
}


OAL_STATIC oal_void dpd_cordic(oal_int32 l_real, oal_int32 l_image, oal_uint32* pl_amp,
    oal_int32* pl_theta1, oal_int32* pl_theta2)
{
    oal_int32 flag;
    oal_int32 l_x[13] = {0};
    oal_int32 l_y[13] = {0};
    oal_int32 l_real_table[13] = {1447, 1891, 2008, 2037, 2045, 2046, 2047, 2047, 2047, 2047, 2047, 2047, 2047};
    oal_int32 l_imag_table[13] = {1447, 783, 399, 201, 100, 50, 25, 13, 6, 3, 2, 1, 0};
    oal_int32 l_theta1 = 2047;
    oal_int32 l_theta2 = 0;
    oal_int32 l_temp1 = 0;
    oal_int32 l_temp2 = 0;
    oal_int32 l_thetatmp1 = 0;
    oal_int32 l_thetatmp2 = 0;
    oal_int32 l_divider = 2048;
    oal_uint8 us_idx = 0;

    l_x[0] = absd(l_real);
    l_y[0] = absd(l_image);
    flag = absd(l_image);

    for (us_idx = 0; us_idx < 10; us_idx++) {
        if (flag > 0) {
            l_temp1 = l_real_table[us_idx] ;
            l_temp2 = -l_imag_table[us_idx];
        } else if (flag < 0) {
            l_temp1 = l_real_table[us_idx];
            l_temp2 = l_imag_table[us_idx];
        } else {
            break;
        }

        l_x[us_idx + 1] = hi1102_cali_div_round_closest((l_x[us_idx] * l_temp1 -  l_y[us_idx] * l_temp2), l_divider);
        l_y[us_idx + 1] = hi1102_cali_div_round_closest((l_y[us_idx] * l_temp1 + l_x[us_idx] * l_temp2), l_divider);

        l_thetatmp1 = hi1102_cali_div_round_closest((l_theta1 * l_temp1 - l_theta2 * l_temp2), l_divider);
        l_thetatmp2 = hi1102_cali_div_round_closest((l_theta2 * l_temp1 + l_theta1 * l_temp2), l_divider);
        l_theta1 = l_thetatmp1;
        l_theta2 = l_thetatmp2;

        flag = l_y[us_idx + 1];
    }

    if (l_real < 0) {
        l_theta1 = -l_theta1;
    }

    if (l_image < 0) {
        l_theta2 = -l_theta2;
    }

    *pl_amp = (oal_uint32)l_x[us_idx];
    *pl_theta1 = (oal_int32)l_theta1;
    *pl_theta2 = (oal_int32)l_theta2;
}


oal_uint32 hi1102_rf_cali_dpd_corr_calc(oal_uint32* hi1102_dpd_cali_data_read, oal_uint32* hi1102_dpd_cali_data_calc)
{
    oal_uint32 vrms = 0;
    oal_uint32 l_amp = 0;
    oal_uint32 pa_gain_target;
    oal_int32  l_real = 0;
    oal_int32  l_imag = 0;
    oal_int32  l_divider = 2048;
    oal_int32  l_dpd_real = 0;
    oal_int32  l_dpd_imag = 0;
    oal_uint8  idx = 0;
    oal_uint8  jdx = 0;

    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        l_real = GET2COMP((((oal_uint32)(*(hi1102_dpd_cali_data_read + idx))) & 0x7ff));
        l_imag = GET2COMP(((((oal_uint32)(*(hi1102_dpd_cali_data_read + idx))) >> 11) & 0x7ff));

        dpd_cordic(l_real, l_imag, &l_amp, &g_st_LUT_Theta[idx].l_real, &g_st_LUT_Theta[idx].l_imag);

        g_st_LUT_Gain[idx] = (oal_uint16)hi1102_cali_div_round_closest((oal_int64)(l_amp << 10),
                                                                       (oal_int64)(DPD_CALI_AMP_REF_RMS * (idx + 1)));
    }

    pa_gain_target = g_st_LUT_Gain[DPD_CALI_GAIN_REF_IDX];
    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        if (g_st_LUT_Gain[idx] == 0) {
            return OAL_FAIL;
        }

        g_st_LUT_Gain[idx] = (oal_uint16)hi1102_cali_div_round_closest((oal_int64)pa_gain_target << 9,
                                                                       (oal_int64)g_st_LUT_Gain[idx]);
    }

    for (idx = 0; idx < LUT_End_Point; idx++) {
        g_st_LUT_Gain[idx] = g_st_LUT_Gain[LUT_End_Point - 1];
        g_st_LUT_Theta[idx] = g_st_LUT_Theta[LUT_End_Point - 1];
    }

    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        g_st_DPD_Gain[idx] = g_st_LUT_Gain[idx];
    }

    /* gain */
    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        vrms = ((DPD_CALI_AMP_REF_RMS * (idx + 1)) * g_st_LUT_Gain[idx]) >> 9;
        for (jdx = 0; jdx < (DPD_CALI_LUT_LENGTH - 1); jdx++) {
            if ((vrms < ((DPD_CALI_AMP_REF_RMS * (jdx + 2)))) && (vrms >= (DPD_CALI_AMP_REF_RMS * (jdx + 1)))) {
                g_st_DPD_Gain[idx] = g_st_LUT_Gain[jdx];
                break;
            }

            if (vrms < (DPD_CALI_AMP_REF_RMS)) {
                g_st_DPD_Gain[idx] = g_st_LUT_Gain[0];
                break;
            }

            if (vrms >= (DPD_CALI_AMP_REF_RMS * DPD_CALI_LUT_LENGTH)) {
                g_st_DPD_Gain[idx] = g_st_LUT_Gain[DPD_CALI_LUT_LENGTH - 1];
                break;
            }
        }

        if (g_st_DPD_Gain[idx] > 1023) {
            g_st_DPD_Gain[idx] = 1023;
        }
    }

    /* phase */
    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        vrms = (oal_uint32)(((DPD_CALI_AMP_REF_RMS * (idx + 1)) * g_st_DPD_Gain[idx]) >> 9);
        for (idx = 0; jdx < (DPD_CALI_LUT_LENGTH - 1); jdx++) {
            if ((vrms < ((DPD_CALI_AMP_REF_RMS * (jdx + 2)))) && (vrms >= (DPD_CALI_AMP_REF_RMS * (jdx + 1)))) {
                g_st_DPD_Theta[idx] = g_st_LUT_Theta[jdx];
                break;
            }

            if (vrms < (DPD_CALI_AMP_REF_RMS)) {
                g_st_DPD_Theta[idx] = g_st_LUT_Theta[0];
                break;
            }

            if (vrms >= (DPD_CALI_AMP_REF_RMS * DPD_CALI_LUT_LENGTH)) {
                g_st_DPD_Theta[idx] = g_st_LUT_Theta[DPD_CALI_LUT_LENGTH - 1];
                break;
            }
        }
    }
    for (idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++) {
        l_dpd_real = hi1102_cali_div_round_closest(g_st_DPD_Gain[idx] * g_st_DPD_Theta[idx].l_real, l_divider);
        l_dpd_imag = hi1102_cali_div_round_closest(g_st_DPD_Gain[idx] * g_st_DPD_Theta[idx].l_imag, l_divider);
        *(hi1102_dpd_cali_data_calc + idx) = (SET2COMP(l_dpd_real)) | ((SET2COMP(l_dpd_imag)) << 11);
    }

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_RF_1102CALI_DPD */


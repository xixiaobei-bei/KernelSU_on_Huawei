

#include "hmac_cali_mgmt.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_DEBUG_C

OAL_STATIC void hmac_print_buff(int8_t *buff)
{
    buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(buff);
    memset_s(buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
}

OAL_STATIC int32_t hmac_print_2g_cail_result(uint8_t uc_cali_chn_idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, hi1103_cali_param_stru *pst_cali_data)
{
    int8_t *pc_string = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_2g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(hi1103_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
                "tx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
                "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, uc_cali_chn_idx,
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_i,
        pst_cali_data->ast_2gcali_param.st_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_q,

        pst_cali_data->ast_2gcali_param.st_cali_logen_cmp[uc_cali_chn_idx].uc_ssb_cmp,
        pst_cali_data->ast_2gcali_param.st_cali_logen_cmp[uc_cali_chn_idx].uc_buf0_cmp,
        pst_cali_data->ast_2gcali_param.st_cali_logen_cmp[uc_cali_chn_idx].uc_buf1_cmp,

        pst_cali_data->ast_2gcali_param.st_cali_tx_power_cmp_2g[uc_cali_chn_idx].uc_ppa_cmp,
        pst_cali_data->ast_2gcali_param.st_cali_tx_power_cmp_2g[uc_cali_chn_idx].uc_atx_pwr_cmp,
        pst_cali_data->ast_2gcali_param.st_cali_tx_power_cmp_2g[uc_cali_chn_idx].uc_dtx_pwr_cmp,
        pst_cali_data->ast_2gcali_param.st_cali_tx_power_cmp_2g[uc_cali_chn_idx].c_dp_init,

        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_0].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_0].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_1].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_1].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_2].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_2].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_3].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_3].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_4].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_4].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_5].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_5].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_6].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_6].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_7].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_7].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_8].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_8].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_9].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_9].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_10].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_10].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_11].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_11].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_12].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_12].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_13].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_13].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_14].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_14].us_txdc_cmp_q,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_15].us_txdc_cmp_i,
        pst_cali_data->ast_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx][BYTE_OFFSET_15].us_txdc_cmp_q);
}

OAL_STATIC int32_t hmac_print_5g_cail_result(uint8_t cali_chn_idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, hi1103_cali_param_stru *pst_cali_data)
{
    int8_t *pc_string = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(hi1103_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = (cali_chn_idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        : "5G 80M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, cali_chn_idx,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_ssb_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_buf0_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_buf1_cmp,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_ppa_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_mx_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_atx_pwr_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_dtx_pwr_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_ppf_cmp_val.uc_ppf_val,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_1].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_1].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_2].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_2].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_3].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_3].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_4].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_4].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_5].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_5].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_6].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_6].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_7].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_7].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_8].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_8].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_9].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_9].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_10].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_10].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_11].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_11].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_12].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_12].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_13].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_13].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_14].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_14].us_txdc_cmp_q,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_15].us_txdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_15].us_txdc_cmp_q);
}

void hmac_dump_cali_result(void)
{
    hi1103_cali_param_stru *pst_cali_data = NULL;
    uint8_t               uc_cali_chn_idx_1;
    uint8_t               uc_cali_chn_idx;
    int8_t               *pc_print_buff = NULL;
    int32_t               l_string_tmp_len;
    uint32_t              string_len    = 0;
    uint8_t               uc_chan_idx;
    uint8_t              *puc_bfgx_rc_code = NULL;
    uint32_t              rc_size;

    oam_info_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::START.}");

    /* OTA上报 */
    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::pc_print_buff null.}",
            OAM_REPORT_MAX_STRING_LEN);
        return;
    }

    puc_bfgx_rc_code = (uint8_t *)wifi_get_bfgx_rc_data_buf_addr(&rc_size);
    if (puc_bfgx_rc_code == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::puc_bfgx_rc_code null.}",
            OAM_REPORT_MAX_STRING_LEN);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return;
    }
    pst_cali_data = (hi1103_cali_param_stru *)get_cali_data_buf_addr();
    *puc_bfgx_rc_code = pst_cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code;

    /*lint -e734*/
    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++) {
        pst_cali_data += uc_chan_idx;
        oam_warning_log3(0, OAM_SF_CALIBRATE, "chan:%d, cali_data dog_tag:%d, check_hw_status:%d",
            uc_chan_idx, pst_cali_data->dog_tag, pst_cali_data->check_hw_status);
        oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
            pst_cali_data->st_cali_update_info.bit_temperature, pst_cali_data->st_cali_update_info.uc_5g_chan_idx1,
            pst_cali_data->st_cali_update_info.uc_5g_chan_idx2,
            pst_cali_data->st_cali_update_info.cali_time);
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
            pst_cali_data->en_save_all, pst_cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
            pst_cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code,
            pst_cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);

        oam_warning_log3(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:classa[0x%x], classb[0x%x], close_fem[%d]",
            pst_cali_data->st_pa_ical_cmp.uc_classa_cmp, pst_cali_data->st_pa_ical_cmp.uc_classb_cmp,
            pst_cali_data->en_need_close_fem_cali);

        /* g_new_txiq_comp_val_stru TO BE ADDED */
        /* 2.4g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx_1++) {
            l_string_tmp_len = hmac_print_2g_cail_result(uc_cali_chn_idx_1, pc_print_buff + string_len,
                (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), pst_cali_data);
            if (l_string_tmp_len <= 0) {
                goto sprint_fail;
            }
            string_len += (uint32_t)l_string_tmp_len;
        }

        hmac_print_buff(pc_print_buff);
        string_len = 0;

        /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip()) {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= (OAL_5G_CHANNEL_NUM >> BIT_OFFSET_1); uc_cali_chn_idx_1++) {
            for (uc_cali_chn_idx = uc_cali_chn_idx_1 << BIT_OFFSET_1;
                 uc_cali_chn_idx < oal_min(OAL_5G_CHANNEL_NUM, ((uc_cali_chn_idx_1 + 1) << BIT_OFFSET_1));
                 uc_cali_chn_idx++) {
                l_string_tmp_len = hmac_print_5g_cail_result(uc_cali_chn_idx, pc_print_buff + string_len,
                    (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), pst_cali_data);
                if (l_string_tmp_len <= 0) {
                    goto sprint_fail;
                }
                string_len += (uint32_t)l_string_tmp_len;
            }

            hmac_print_buff(pc_print_buff);
            string_len = 0;
        }
    }
    /*lint +e734*/
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    hmac_print_buff(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);

    return;
}

OAL_STATIC int32_t hmac_print_1105_2g_cail_result(uint8_t uc_cali_chn_idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, hi1105_cali_param_stru *pst_cali_data)
{
    int8_t *pc_string = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_2g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(hi1105_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
                "tx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc mixbuf[0], \
                siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, uc_cali_chn_idx,
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_1],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_2],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_3],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_4],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_5],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_6],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_7],
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_i,
        pst_cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_q,

        pst_cali_data->st_2gcali_param.ast_cali_logen_cmp[uc_cali_chn_idx].uc_ssb_cmp,
        pst_cali_data->st_2gcali_param.ast_cali_logen_cmp[uc_cali_chn_idx].uc_buf0_cmp,
        pst_cali_data->st_2gcali_param.ast_cali_logen_cmp[uc_cali_chn_idx].uc_buf1_cmp,

        pst_cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[uc_cali_chn_idx].us_ppa_cmp,
        pst_cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[uc_cali_chn_idx].us_atx_pwr_cmp,
        pst_cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[uc_cali_chn_idx].uc_dtx_pwr_cmp,
        pst_cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[uc_cali_chn_idx].c_dp_init,

        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_0],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_0],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_1],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_1],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_2],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_2],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_3],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_3],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_4],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_4],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_5],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_5],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_6],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_6],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_i[BYTE_OFFSET_7],
        pst_cali_data->st_2gcali_param.ast_txdc_cmp_val[uc_cali_chn_idx].aus_txdc_cmp_q[BYTE_OFFSET_7]);
}

OAL_STATIC int32_t hmac_print_1105_5g_cail_result(uint8_t cali_chn_idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, hi1105_cali_param_stru *pst_cali_data)
{
    int8_t *pc_string = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(hi1105_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = (cali_chn_idx < OAL_5G_20M_CHANNEL_NUM) ? "5G 20M: cali data index[%u]\n"
        "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "digital_rxdc_i_q[0x%x 0x%x]\n"
        "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
        "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
        "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
        "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n" :
        "5G 80M: cali data index[%u]\n"
        "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
        "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "digital_rxdc_i_q[0x%x 0x%x]\n"
        "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
        "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
        "tx_dc mixbuf[0], \
        siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, cali_chn_idx,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_ssb_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_buf0_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_logen_cmp.uc_buf1_cmp,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.us_ppa_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_mx_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.us_atx_pwr_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_cali_tx_power_cmp_5g.uc_dtx_pwr_cmp,
        pst_cali_data->ast_5gcali_param[cali_chn_idx].st_ppf_cmp_val.uc_ppf_val,

        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_0],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_1],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_2],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_3],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_4],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_5],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_6],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_i[BYTE_OFFSET_7],
        pst_cali_data->ast_5gcali_param[cali_chn_idx].ast_txdc_cmp_val[BYTE_OFFSET_0].aus_txdc_cmp_q[BYTE_OFFSET_7]);
}
void hmac_dump_rx_en_result(hi1105_cali_param_stru *cali_data, uint8_t rf_id)
{
    if (cali_data->rx_en_invalid != 0) {
        chr_exception_p(CHR_CHIP_CALI_ERR_REPORT_EVENTID, &cali_data->rx_en_invalid,
            sizeof(cali_data->rx_en_invalid));
        oam_error_log2(0, OAM_SF_CALIBRATE,
            "{hmac_dump_rx_en_result:: rf[%d] rx_en line ctrl invalid %d !}",
            rf_id, cali_data->rx_en_invalid);
    }
}

void hmac_dump_1105_cali_result(void)
{
    hi1105_cali_param_stru *pst_cali_data = NULL;
    uint8_t               uc_cali_chn_idx_1;
    uint8_t               idx;
    int8_t               *pc_print_buff = NULL;
    int32_t               l_string_tmp_len;
    uint32_t              string_len    = 0;
    uint8_t               uc_chan_idx;
    uint8_t              *puc_bfgx_rc_code = NULL;
    uint32_t              rc_size;

    oam_info_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::START.}");

    /* OTA上报 */
    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::pc_print_buff null.}",
            OAM_REPORT_MAX_STRING_LEN);
        return;
    }

    puc_bfgx_rc_code = (uint8_t *)wifi_get_bfgx_rc_data_buf_addr(&rc_size);
    if (puc_bfgx_rc_code == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::puc_bfgx_rc_code null.}",
            OAM_REPORT_MAX_STRING_LEN);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return;
    }
    pst_cali_data = (hi1105_cali_param_stru *)get_cali_data_buf_addr();
    *puc_bfgx_rc_code = pst_cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code;

    /*lint -e734*/
    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++) {
        pst_cali_data += uc_chan_idx;
        /* 线控失效打点维测 */
        hmac_dump_rx_en_result(pst_cali_data, uc_chan_idx);

        oam_warning_log3(0, OAM_SF_CALIBRATE, "chan:%d, cali_data dog_tag:%d, check_hw_status:%d",
            uc_chan_idx, pst_cali_data->dog_tag, pst_cali_data->check_hw_status);
        oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
            pst_cali_data->st_cali_update_info.bit_temperature, pst_cali_data->st_cali_update_info.uc_5g_chan_idx1,
            pst_cali_data->st_cali_update_info.uc_5g_chan_idx2,
            pst_cali_data->st_cali_update_info.us_cali_time);
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
            pst_cali_data->en_save_all, pst_cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
            pst_cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code,
            pst_cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);

        oam_warning_log3(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:classa[0x%x], classb[0x%x], close_fem[%d]",
            pst_cali_data->st_pa_ical_cmp.uc_classa_cmp, pst_cali_data->st_pa_ical_cmp.uc_classb_cmp,
            pst_cali_data->en_need_close_fem_cali);

        /* new_txiq_comp_val_stru TO BE ADDED */
        /* 2.4g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx_1++) {
            l_string_tmp_len = hmac_print_1105_2g_cail_result(uc_cali_chn_idx_1, pc_print_buff + string_len,
                (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), pst_cali_data);
            if (l_string_tmp_len <= 0) {
                goto sprint_fail;
            }
            string_len += (uint32_t)l_string_tmp_len;
        }

        hmac_print_buff(pc_print_buff);
        string_len = 0;

        /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip()) {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= (OAL_5G_CHANNEL_NUM >> BIT_OFFSET_1); uc_cali_chn_idx_1++) {
            for (idx = (uc_cali_chn_idx_1 << BIT_OFFSET_1);
                idx < oal_min(OAL_5G_CHANNEL_NUM, ((uc_cali_chn_idx_1 + 1) << BIT_OFFSET_1)); idx++) {
                l_string_tmp_len = hmac_print_1105_5g_cail_result(idx, pc_print_buff + string_len,
                    (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), pst_cali_data);
                if (l_string_tmp_len <= 0) {
                    goto sprint_fail;
                }
                string_len += (uint32_t)l_string_tmp_len;
            }

            hmac_print_buff(pc_print_buff);
            string_len = 0;
        }
    }
    /*lint +e734*/
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    hmac_print_buff(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);

    return;
}

#ifdef _PRE_DEBUG_CODE
OAL_STATIC void hmac_print_1106_base_cail_result(uint8_t rf_idx, hi1106_cali_param_stru *cali_data)
{
    oam_warning_log3(0, OAM_SF_CALIBRATE, "rf:%d, cali_data dog_tag:%d, check_hw_status:%d",
        rf_idx, cali_data->dog_tag, cali_data->check_hw_status);
    oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
        cali_data->st_cali_update_info.bit_temperature,
        cali_data->st_cali_update_info.uc_5g_chan_idx1,
        cali_data->st_cali_update_info.uc_5g_chan_idx2,
        cali_data->st_cali_update_info.us_cali_time);
    oam_warning_log4(0, OAM_SF_CALIBRATE,
        "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
        cali_data->en_save_all, cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
        cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code,
        cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);
    oam_warning_log1(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:close_fem[%d]",
        cali_data->en_need_close_fem_cali);
}

OAL_STATIC int32_t hmac_print_1106_2g_cail_result(uint8_t chn_idx,
    int8_t *buff, int32_t remain_len, hi1106_cali_param_stru *cali_data)
{
    int8_t *pc_string = NULL;

    if (remain_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE, "hmac_print_2g_cail_result:len[%d] max[%d] cali_size[%d]",
            remain_len, OAM_REPORT_MAX_STRING_LEN, sizeof(hi1106_cali_param_stru));
        return remain_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\ntx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc [0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(buff, (uint32_t)remain_len, (uint32_t)remain_len - 1, pc_string, chn_idx,
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_0],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_1],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_2],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_3],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_4],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_5],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_6],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].aus_analog_rxdc_siso_cmp[BIT_OFFSET_7],
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].us_digital_rxdc_cmp_i,
        cali_data->st_2Gcali_param.ast_cali_rx_dc_cmp[BIT_OFFSET_0].us_digital_rxdc_cmp_q,

        cali_data->st_2Gcali_param.ast_cali_logen_cmp[chn_idx].uc_ssb_cmp,
        cali_data->st_2Gcali_param.ast_cali_logen_cmp[chn_idx].uc_buf0_cmp,
        cali_data->st_2Gcali_param.ast_cali_logen_cmp[chn_idx].uc_buf1_cmp,

        cali_data->st_2Gcali_param.ast_cali_tx_power_cmp_2G[chn_idx].us_ppa_cmp,
        cali_data->st_2Gcali_param.ast_cali_tx_power_cmp_2G[chn_idx].us_atx_pwr_cmp,
        cali_data->st_2Gcali_param.ast_cali_tx_power_cmp_2G[chn_idx].uc_dtx_pwr_cmp,
        cali_data->st_2Gcali_param.ast_cali_tx_power_cmp_2G[chn_idx].c_dp_init,

        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_0],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_0],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_1],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_1],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_2],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_2],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_3],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_3],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_4],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_4],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_5],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_5],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_6],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_6],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_i[BIT_OFFSET_7],
        cali_data->st_2Gcali_param.ast_txdc_cmp_val[chn_idx].aus_txdc_cmp_q[BIT_OFFSET_7]);
}

OAL_STATIC int32_t hmac_print_1106_5g_cail_result(uint8_t chn_idx,
    int8_t *pc_print_buff, int32_t remain_len, hi1106_cali_param_stru *cali_data)
{
    int8_t *pc_string;

    pc_string = "5G: cali data index[%u]\n"
        "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
        "digital_rxdc_i_q[0x%x 0x%x]\n"
        "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
        "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
        "tx_dc [0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)remain_len, (uint32_t)remain_len - 1,
        pc_string, chn_idx,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_0],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_1],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_2],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_3],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_4],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_5],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_6],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BIT_OFFSET_7],
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

        cali_data->ast_5Gcali_param[chn_idx].st_cali_logen_cmp.uc_ssb_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_logen_cmp.uc_buf0_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_logen_cmp.uc_buf1_cmp,

        cali_data->ast_5Gcali_param[chn_idx].st_cali_tx_power_cmp_5G.us_ppa_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_tx_power_cmp_5G.uc_mx_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_tx_power_cmp_5G.us_atx_pwr_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_cali_tx_power_cmp_5G.uc_dtx_pwr_cmp,
        cali_data->ast_5Gcali_param[chn_idx].st_ppf_cmp_val.uc_ppf_val,

        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_0],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_0],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_1],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_1],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_2],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_2],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_3],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_3],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_4],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_4],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_5],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_5],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_6],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_6],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_i[BIT_OFFSET_7],
        cali_data->ast_5Gcali_param[chn_idx].ast_txdc_cmp_val[BIT_OFFSET_0].aus_txdc_cmp_q[BIT_OFFSET_7]);
}
void hmac_print_error(int8_t *buff)
{
    oam_warning_log0(0, OAM_SF_CFG, "{hmac_print_error:: OAL_SPRINTF return error!}");
    hmac_print_buff(buff);
    oal_mem_free_m(buff, OAL_TRUE);
}

uint32_t hmac_dump_1106_2g_cali(int8_t *buff, hi1106_cali_param_stru *cali_data)
{
    uint8_t   chn_idx;
    int32_t   str_tmp_len;
    int32_t   remain_len;
    uint32_t  str_len     = 0;

    for (chn_idx = 0; chn_idx < OAL_2G_CHANNEL_NUM; chn_idx++) {
        remain_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - str_len - 1);
        if (remain_len <= 0) {
            hmac_print_error(buff);
            return OAL_FAIL;
        }
        str_tmp_len = hmac_print_1106_2g_cail_result(chn_idx, buff + str_len, remain_len, cali_data);
        str_len += (uint32_t)str_tmp_len;
    }

    hmac_print_buff(buff);
    return OAL_SUCC;
}

uint32_t hmac_dump_1106_5g_cali(int8_t *buff, hi1106_cali_param_stru *cali_data)
{
    uint8_t               idx;
    uint8_t               chn_idx_1;
    int32_t               str_tmp_len;
    int32_t               remain_len;
    uint32_t              str_len = 0;
    /*lint -e734*/
    for (chn_idx_1 = 0; chn_idx_1 <= (OAL_5G_CHANNEL_NUM >> BIT_OFFSET_1); chn_idx_1++) {
        for (idx = (chn_idx_1 << BIT_OFFSET_1);
            idx < oal_min(OAL_5G_CHANNEL_NUM, ((chn_idx_1 + 1) << BIT_OFFSET_1)); idx++) {
            remain_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - str_len - 1);
            if (remain_len <= 0) {
                hmac_print_error(buff);
                return OAL_FAIL;
            }
            str_tmp_len = hmac_print_1106_5g_cail_result(idx, buff + str_len, remain_len, cali_data);
            str_len += (uint32_t)str_tmp_len;
        }

        hmac_print_buff(buff);
    }
    /*lint +e734*/
    return OAL_SUCC;
}

void hmac_dump_1106_cali_result(void)
{
    hi1106_cali_param_stru *cali_data;
    int8_t               *buff;
    uint8_t               rf_idx;
    uint8_t              *bfgx_rc_code;
    uint32_t              rc_size;
    uint32_t              ret;

    /* OTA上报 */
    buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    bfgx_rc_code = (uint8_t *)wifi_get_bfgx_rc_data_buf_addr(&rc_size);
    cali_data = (hi1106_cali_param_stru *)get_cali_data_buf_addr();
    if ((buff == NULL) || (bfgx_rc_code == NULL) || (cali_data == NULL)) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::para null.}");
        return;
    }

    *bfgx_rc_code = cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code;

    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        cali_data += rf_idx;

        hmac_print_1106_base_cail_result(rf_idx, cali_data);

        /* 2.4g 不超过ota上报最大字节分3次输出 */
        ret = hmac_dump_1106_2g_cali(buff, cali_data);
        if (ret != OAL_SUCC) {
            return;
        }

        /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip()) {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        ret = hmac_dump_1106_5g_cali(buff, cali_data);
        if (ret != OAL_SUCC) {
            return;
        }
    }
    oal_mem_free_m(buff, OAL_TRUE);
}
#endif

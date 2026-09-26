
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_wapi_sms4.h"
#include "hmac_wapi_wpi.h"
#include "hmac_wapi.h"
#include "wlan_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WAPI_WPI_C

#define MAX_MIC_LEN    4096 /* message integrity code */
#define SMS4_STEP      4
#define IV_LEN         4    /* Initialization Vector */
#define RK_LEN         32   /* round key */
#define MIC_LEN        4

#ifdef _PRE_WLAN_FEATURE_WAPI

uint32_t hmac_wpi_encrypt(uint8_t *puc_iv, uint8_t *puc_bufin, uint32_t buflen, uint8_t *puc_key,
    uint8_t *puc_bufout)
{
    uint32_t aul_iv_out[IV_LEN];
    uint32_t *pul_in = NULL;
    uint32_t *pul_out = NULL;
    uint8_t *puc_out = NULL;
    uint8_t *puc_in = NULL;
    uint32_t counter;
    uint32_t comp;
    uint32_t loop;
    uint32_t aul_pr_keyin[RK_LEN] = { 0 };

    if (buflen < 1) {
#ifdef WAPI_DEBUG_MODE
        g_stMacDriverStats.ulSms4OfbInParmInvalid++;
#endif
        return OAL_FAIL;
    }

    hmac_sms4_keyext(puc_key, aul_pr_keyin, sizeof(aul_pr_keyin));

    counter = buflen / SMS4_MIC_LEN;
    comp = buflen % SMS4_MIC_LEN;

    /* get the iv */
    hmac_sms4_crypt(puc_iv, (uint8_t *)aul_iv_out, aul_pr_keyin, sizeof(aul_pr_keyin));
    pul_in = (uint32_t *)puc_bufin;
    pul_out = (uint32_t *)puc_bufout;

    for (loop = 0; loop < counter; loop++) {
        pul_out[BYTE_OFFSET_0] = pul_in[BYTE_OFFSET_0] ^ aul_iv_out[BYTE_OFFSET_0];
        pul_out[BYTE_OFFSET_1] = pul_in[BYTE_OFFSET_1] ^ aul_iv_out[BYTE_OFFSET_1];
        pul_out[BYTE_OFFSET_2] = pul_in[BYTE_OFFSET_2] ^ aul_iv_out[BYTE_OFFSET_2];
        pul_out[BYTE_OFFSET_3] = pul_in[BYTE_OFFSET_3] ^ aul_iv_out[BYTE_OFFSET_3];
        hmac_sms4_crypt((uint8_t *)aul_iv_out, (uint8_t *)aul_iv_out, aul_pr_keyin, sizeof(aul_pr_keyin));
        pul_in += SMS4_STEP;
        pul_out += SMS4_STEP;
    }

    puc_in = (uint8_t *)pul_in;
    puc_out = (uint8_t *)pul_out;
    puc_iv = (uint8_t *)aul_iv_out;

    for (loop = 0; loop < comp; loop++) {
        puc_out[loop] = puc_in[loop] ^ puc_iv[loop];
    }

    return OAL_SUCC;
}

uint32_t hmac_wpi_decrypt(uint8_t *puc_iv, uint8_t *puc_bufin, uint32_t buflen, uint8_t *puc_key,
    uint8_t *puc_bufout)
{
    return hmac_wpi_encrypt(puc_iv, puc_bufin, buflen, puc_key, puc_bufout);
}

void hmac_wpi_swap_pn(uint8_t *puc_pn, uint8_t uc_len)
{
    uint8_t uc_index;
    uint8_t uc_temp;
    uint8_t uc_len_tmp;

    uc_len_tmp = uc_len / 2; /* 2表示取一半长度 */
    for (uc_index = 0; uc_index < uc_len_tmp; uc_index++) {
        uc_temp = puc_pn[uc_index];
        puc_pn[uc_index] = puc_pn[uc_len - 1 - uc_index];
        puc_pn[uc_len - 1 - uc_index] = uc_temp;
    }
}


uint32_t hmac_wpi_pmac(uint8_t *puc_iv, uint8_t *puc_buf, uint32_t pamclen, uint8_t *puc_key, uint8_t *puc_mic,
    uint8_t uc_mic_len)
{
    uint32_t aul_mic_tmp[MIC_LEN];
    uint32_t loop;
    uint32_t *pul_in = NULL;
    uint32_t aul_pr_mac_keyin[RK_LEN] = { 0 };

    if ((pamclen < 1) || (pamclen > MAX_MIC_LEN)) {
        return OAL_FAIL;
    }

    if (uc_mic_len < sizeof(aul_mic_tmp)) {
        return OAL_FAIL;
    }

    hmac_sms4_keyext(puc_key, aul_pr_mac_keyin, sizeof(aul_pr_mac_keyin));
    pul_in = (uint32_t *)puc_buf;
    hmac_sms4_crypt(puc_iv, (uint8_t *)aul_mic_tmp, aul_pr_mac_keyin, sizeof(aul_pr_mac_keyin));

    for (loop = 0; loop < pamclen; loop++) {
        aul_mic_tmp[BYTE_OFFSET_0] ^= pul_in[BYTE_OFFSET_0];
        aul_mic_tmp[BYTE_OFFSET_1] ^= pul_in[BYTE_OFFSET_1];
        aul_mic_tmp[BYTE_OFFSET_2] ^= pul_in[BYTE_OFFSET_2];
        aul_mic_tmp[BYTE_OFFSET_3] ^= pul_in[BYTE_OFFSET_3];
        pul_in += SMS4_STEP;
        hmac_sms4_crypt((uint8_t *)aul_mic_tmp, (uint8_t *)aul_mic_tmp, aul_pr_mac_keyin, sizeof(aul_pr_mac_keyin));
    }

    pul_in = (uint32_t *)puc_mic;
    pul_in[BYTE_OFFSET_0] = aul_mic_tmp[BYTE_OFFSET_0];
    pul_in[BYTE_OFFSET_1] = aul_mic_tmp[BYTE_OFFSET_1];
    pul_in[BYTE_OFFSET_2] = aul_mic_tmp[BYTE_OFFSET_2];
    pul_in[BYTE_OFFSET_3] = aul_mic_tmp[BYTE_OFFSET_3];

    return OAL_SUCC;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
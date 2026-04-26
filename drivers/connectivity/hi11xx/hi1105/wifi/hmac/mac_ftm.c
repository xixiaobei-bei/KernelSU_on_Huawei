

#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#include "mac_mib.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAL_FILE_ID_MAC_FTM_C

/* FTM定制化能力 1：支持 0：不支持 bit0:是否支持ftm initiator bit1:是否支持ftm responder bit2：是否支持ftm range */
uint8_t g_mac_ftm_cap = 0;


uint8_t mac_is_ftm_enable(mac_vap_stru *mac_vap)
{
    if (mac_mib_get_FtmResponderModeActivated(mac_vap) == OAL_TRUE) {
        return OAL_TRUE;
    }
    if (mac_mib_get_FtmInitiatorModeActivated(mac_vap) == OAL_TRUE) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


void mac_ftm_mib_init(mac_vap_stru *mac_vap)
{
    if (IS_LEGACY_STA(mac_vap)) {
        mac_mib_set_FtmInitiatorModeActivated(mac_vap, (g_mac_ftm_cap & BIT0 ? OAL_TRUE : OAL_FALSE));
        mac_mib_set_FtmRangeReportActivated(mac_vap, (g_mac_ftm_cap & BIT2 ? OAL_TRUE : OAL_FALSE));
    }
    if (IS_LEGACY_AP(mac_vap)) {
        mac_mib_set_FtmResponderModeActivated(mac_vap, (g_mac_ftm_cap & BIT1 ? OAL_TRUE : OAL_FALSE));
    }
}


uint8_t mac_ftm_get_cap(void)
{
    return g_mac_ftm_cap;
}


void mac_ftm_add_to_ext_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *buffer, uint8_t *ie_len)
{
    mac_ext_cap_ftm_ie_stru *ext_cap_ftm = NULL;
    memset_s(buffer + MAC_IE_HDR_LEN + MAC_XCAPS_EX_LEN, (MAC_XCAPS_EX_FTM_LEN - MAC_XCAPS_EX_LEN), 0,
        (MAC_XCAPS_EX_FTM_LEN - MAC_XCAPS_EX_LEN));

    if ((mac_mib_get_FtmInitiatorModeActivated(mac_vap) == OAL_FALSE) &&
        (mac_mib_get_FtmResponderModeActivated(mac_vap) == OAL_FALSE)) {
        return;
    }

    buffer[1] = MAC_XCAPS_EX_FTM_LEN;
    ext_cap_ftm = (mac_ext_cap_ftm_ie_stru *)(buffer + MAC_IE_HDR_LEN);
    ext_cap_ftm->bit_ftm_responder = mac_mib_get_FtmResponderModeActivated(mac_vap);
    ext_cap_ftm->bit_ftm_initiator = mac_mib_get_FtmInitiatorModeActivated(mac_vap);

    (*ie_len)++;
}
#endif /* _PRE_WLAN_FEATURE_FTM */

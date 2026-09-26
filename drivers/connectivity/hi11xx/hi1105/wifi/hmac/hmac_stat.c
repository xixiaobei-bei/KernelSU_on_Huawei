

#include "hmac_stat.h"
#include "hmac_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_STAT_C

hmac_device_stat_stru g_hmac_device_stats;

void hmac_stat_init_device_stat(hmac_device_stru *hmac_device)
{
    hmac_device->device_stats = &g_hmac_device_stats;
    memset_s(&g_hmac_device_stats, sizeof(g_hmac_device_stats), 0, sizeof(g_hmac_device_stats));
}

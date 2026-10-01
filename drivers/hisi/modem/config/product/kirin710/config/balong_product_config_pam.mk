# MD5: d7f61570eb1be10bad4349104594fc66
CFG_BBP_MASTER_NONE                             := 0
CFG_BBP_MASTER_VER1                             := 1
CFG_BBP_MASTER_VER2                             := 2
CFG_BBP_MASTER_VER3                             := 3
CFG_BBP_MASTER_VER4                             := 4
CFG_BBP_MASTER_VER5                             := 5
CFG_BBP_MASTER_VER6                             := 6
CFG_FEATURE_BBP_MASTER_VER                      := (BBP_MASTER_VER5)
CFG_FEATURE_POWER_TIMER                         := FEATURE_OFF
CFG_FEATURE_UE_UICC_MULTI_APP_SUPPORT           := FEATURE_ON
CFG_FEATURE_VSIM                                := FEATURE_ON
CFG_FEATURE_GUC_BBP_TRIG                        := FEATURE_ON
CFG_FEATURE_CSIM_NONCHECK                       := FEATURE_OFF
CFG_FEATURE_GUC_BBP_TRIG_NEWVERSION             := FEATURE_ON
CFG_FEATURE_VOS_REDUCE_MEM_CFG                  := FEATURE_OFF
CFG_FEATURE_RTC_TIMER_DBG                       := FEATURE_OFF
CFG_FEATURE_PHONE_SC                            := FEATURE_ON
ifeq ($(FEATURE_OTA_NETLOCK),FEATURE_ON)
CFG_FEATURE_SC_NETWORK_UPDATE              := FEATURE_ON
else
CFG_FEATURE_SC_NETWORK_UPDATE              := FEATURE_OFF
endif
CFG_FEATURE_SC_SIGNATURE_UPDATE                 := FEATURE_ON
CFG_FEATURE_VSIM_ICC_SEC_CHANNEL                := FEATURE_ON
CFG_FEATURE_BOSTON_AFTER_FEATURE                := FEATURE_ON
CFG_FEATURE_XSIM                                := FEATURE_OFF
CFG_FEATURE_SCI_SWITCH_OPTIMIZE                 := FEATURE_ON
CFG_FEATURE_VOS_18H_TIMER                       := FEATURE_OFF
CFG_FEATURE_SET_C1C2_VALUE                      := FEATURE_OFF

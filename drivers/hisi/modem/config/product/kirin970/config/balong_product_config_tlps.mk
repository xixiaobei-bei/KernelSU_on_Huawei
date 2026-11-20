# MD5: 848fafd1ea694f0dc38b8f50abfdc5ff
CFG_FEATURE_IMS                                 := FEATURE_ON
CFG_FEATURE_LTE_R11                             := FEATURE_ON
CFG_FEATURE_LTE_R13                             := FEATURE_ON
CFG_FEATURE_LTE_R14 							:= FEATURE_OFF
CFG_FEATURE_LTE_MBMS                            := FEATURE_OFF
CFG_FEATURE_LTE_CDMA_R11                        := FEATURE_OFF
CFG_FEATURE_PTM                                 := FEATURE_ON
ifeq ($(cdma),false)
CFG_FEATURE_BALONG_CL                           := FEATURE_OFF
else
CFG_FEATURE_BALONG_CL                           := FEATURE_ON
endif
CFG_FEATURE_LTE_CAT_PART_R11                    := YES
CFG_TL_PS_MAX_TRCH_NUM_8    := YES
CFG_FEATURE_LPP                                 := FEATURE_ON
CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD         := FEATURE_OFF
CFG_FEATURE_CSG                            := FEATURE_ON
CFG_FEATURE_DFS_SYNC                            := FEATURE_OFF
CFG_FEATURE_VERSION_V8                          := FEATURE_OFF
CFG_FEATURE_DCX0_TLHPA_WRITE_PHY_NV             := FEATURE_ON
CFG_FEATURE_CSS_CLOUD_MEMORY_IMPROVE           := FEATURE_ON
CFG_FEATURE_CSS_RAT_RECOGNIZE                      := FEATURE_ON
CFG_FEATURE_MODEM1_SUPPORT_LTE                := FEATURE_ON
CFG_FEATURE_MEM_DECREASE                      := FEATURE_OFF
CFG_FEATURE_DATA_RETRY                            := FEATURE_ON
CFG_FEATURE_M2M                            := FEATURE_ON
CFG_FEATURE_CSS_DSDS_START_BGS                      := FEATURE_OFF
ifeq ($(CFG_CHIP_TYPE_CS),FEATURE_ON)
CFG_FEATURE_KIRIN970_ES_VERSION                := FEATURE_OFF
else
CFG_FEATURE_KIRIN970_ES_VERSION                := FEATURE_ON
endif
ifeq ($(CFG_CHIP_TYPE_CS),FEATURE_ON)
CFG_FEATURE_LL2_SUPPORT_LLRAM_DL_TB            := FEATURE_ON
else
CFG_FEATURE_LL2_SUPPORT_LLRAM_DL_TB            := FEATURE_OFF
endif
CFG_FEATURE_MEMORY_REDUCE                      := FEATURE_OFF
CFG_FEATURE_LTE_JAM_DET                        := FEATURE_OFF
CFG_FEATURE_LTE_ASN_CUT                        := FEATURE_ON
CFG_FEATURE_LL2_SPECIAL_VERSION                := FEATURE_OFF
CFG_FEATURE_LL2_SUPPORT_DSDSV3                 := FEATURE_ON
CFG_FEATURE_LMAC_LPHY_EPDCCH_CR                 := FEATURE_OFF
CFG_FEATURE_UL_HCODE                 			:= FEATURE_OFF
CFG_FEATURE_LTE_R12                                := FEATURE_ON
CFG_FEATURE_LTE_DC                                 := FEATURE_ON
CFG_FEATURE_LTE_LAA                 			   := FEATURE_ON

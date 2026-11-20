# bcmdhd
#
# Copyright (C) 2022, Broadcom.
#
#      Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed to you
# under the terms of the GNU General Public License version 2 (the "GPL"),
# available at http://www.broadcom.com/licenses/GPLv2.php, with the
# following added to such license:
#
#      As a special exception, the copyright holders of this software give you
# permission to link this software with independent modules, and to copy and
# distribute the resulting executable under terms of your choice, provided that
# you also meet, for each linked independent module, the terms and conditions of
# the license of that module.  An independent module is a module which is not
# derived from this software.  The special exception does not apply to any
# modifications of the software.
#
#
# <<Broadcom-WL-IPTag/Open:>>
#

##### customer features pick up
# use '='(equal) rathern than '?=' for below items if needs to force overwrite external predefined value
CONFIG_CHIPSET ?= BCM43752
CONFIG_BCMDHD = y
CONFIG_BCMDHD_PCIE = n
CONFIG_BCMDHD_SDIO = y
CONFIG_BCMDHD_SDIO_SDMMC = y
CONFIG_BCMDHD_OOB_HOST_WAKE = y
# 0:SDPCM_TXGLOM_CPY(copy)
# 1:SDPCM_TXGLOM_MDESC(ADMA)
CONFIG_BCMDHD_SDIO_TXGLOM_MANNER ?= 1
CONFIG_BCMDHD_OOB ?= y
CONFIG_BUILD_CFG80211 ?= y
CONFIG_NEW_BACKPORT4 ?= n
CONFIG_WPA3_SUPPORT ?= y
CONFIG_BCMDHD_DEBUG ?= y
CONFIG_BCMDHD_LOG ?= y
CONFIG_BROADCOM_WIFI_RESERVED_MEM = y
CONFIG_ANDROID_PLATFORM ?= y
CONFIG_BCMDHD_PKTMON ?= y
CONFIG_DHD_USE_STATIC_BUF = y


### bus type checking
ifeq ($(CONFIG_BCMDHD_PCIE),)
  ifeq ($(CONFIG_BCMDHD_SDIO),)
    CONFIG_BCMDHD_PCIE = y
    CONFIG_BCMDHD_SDIO = n
  else # CONFIG_BCMDHD_SDIO
    ifeq ($(CONFIG_BCMDHD_SDIO),y)
      CONFIG_BCMDHD_PCIE = n
      CONFIG_BCMDHD_SDIO = y
    else # CONFIG_BCMDHD_SDIO
      CONFIG_BCMDHD_PCIE = y
      CONFIG_BCMDHD_SDIO = n
    endif # CONFIG_BCMDHD_SDIO
  endif # CONFIG_BCMDHD_SDIO
else # CONFIG_BCMDHD_PCIE
  ifeq ($(CONFIG_BCMDHD_PCIE),y)
    CONFIG_BCMDHD_PCIE = y
    CONFIG_BCMDHD_SDIO = n
  else # CONFIG_BCMDHD_PCIE
    ifeq ($(CONFIG_BCMDHD_SDIO),)
      CONFIG_BCMDHD_PCIE = y
      CONFIG_BCMDHD_SDIO = n
    else # CONFIG_BCMDHD_SDIO
      ifeq ($(CONFIG_BCMDHD_SDIO),y)
        CONFIG_BCMDHD_PCIE = n
        CONFIG_BCMDHD_SDIO = y
      else # CONFIG_BCMDHD_SDIO
        CONFIG_BCMDHD_PCIE = y
        CONFIG_BCMDHD_SDIO = n
      endif # CONFIG_BCMDHD_SDIO
    endif # CONFIG_BCMDHD_SDIO
  endif # CONFIG_BCMDHD_SDIO
endif # CONFIG_BCMDHD_PCIE
ifeq ($(CONFIG_BCMDHD_SDIO),y)
  ifneq ($(CONFIG_BCMDHD_SDIO_SDMMC),y)
    CONFIG_BCMDHD_SDIO_SDSTD = y
  else # CONFIG_BCMDHD_SDIO_SDMMC
    CONFIG_BCMDHD_SDIO_SDSTD = n
  endif # CONFIG_BCMDHD_SDIO_SDMMC
endif # CONFIG_BCMDHD_SDIO

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
### Path to the module source
M ?= $(shell pwd)

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
BCMDHD_ROOT := $(patsubst %/,%,$(dir $(MKFILE_PATH)))

# For inbuilt module, below configs will be provided via defconfig
# But for out-of-tree module, explicitly define them here and add
# them as cflags
ifeq ($(KERNEL_SRC),)
  LINUXDIR ?= $(KDIR)
  CC ?= gcc
  LD ?= ld
  NM ?= nm
  OBJCOPY ?= objcopy
  OBJDUMP ?= objdump
  STRIP ?= strip
  ifneq ($(CROSS_COMPILE),)
    # Allow CROSS_COMPILE to specify compiler base
    CC := $(CROSS_COMPILE)gcc
    LD := $(CROSS_COMPILE)ld
    NM := $(CROSS_COMPILE)nm
    OBJCOPY := $(CROSS_COMPILE)objcopy
    OBJDUMP := $(CROSS_COMPILE)objdump
    STRIP := $(CROSS_COMPILE)strip
  endif # CROSS_COMPILE

  ifneq ($(LINUXDIR),)
    KERNEL_SRC ?= $(LINUXDIR)
  endif
  BCMDHD_ROOT=$(src)
else # KERNEL_SRC
  KBUILD_OPTIONS += BCMDHD_ROOT=$(shell cd $(KERNEL_SRC); readlink -e $(M))
endif # KERNEL_SRC

##### modular driver may fail to start up if use static
ifeq ($(CONFIG_BCMDHD),m) # for modular DHD driver
  # skip static buf
  CONFIG_DHD_USE_STATIC_BUF=
endif # for modular DHD driver

### Derive Linux version
LINUXSRC := $(LINUXDIR)
### Try to fix for not local build
ifneq ($(wildcard $(LINUXSRC)/source), )
  LINUXSRC := $(LINUXDIR)/source
endif
ifneq ($(LINUXSRC),)
  VERSION = $(shell grep "^VERSION =" $(LINUXSRC)/Makefile | cut -d " " -f 3)
  PATCHLEVEL = $(shell grep "^PATCHLEVEL =" $(LINUXSRC)/Makefile | cut -d " " -f 3)
  SUBLEVEL = $(shell grep "^SUBLEVEL =" $(LINUXSRC)/Makefile | cut -d " " -f 3)
endif # LINUXSRC
BCM_KVERSIONSTRING := $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))

### Compares two dotted numeric strings (e.g 2.3.16.1) for $1 >= $2
define wlan_version_ge
$(findstring TRUE,$(shell bash -c 'sort -cu -t. -k1,1nr -k2,2nr -k3,3nr -k4,4nr <(echo -e "$2\n$1") 2>&1 || echo TRUE'))
endef # wlan_version_ge

##### SDIO/PCIE Basic feature
WFLAGS := -Wall \
    -Wstrict-prototypes \
    -Werror \
    -Wno-main \
    -Wno-format-zero-length \
    -Wswitch \
    -Wno-sign-compare \
    -Wno-pointer-sign \
    -Wno-attributes \
    -Wno-unused-parameter \
    -fno-strict-aliasing \
    -Woverride-init \
    -Wtype-limits \
    -Wuninitialized \
    -Wunused-variable \
    -DSTRICT_GCC_WARNINGS

ifeq ($(call wlan_version_ge,$(GCCVER),8.0),TRUE)
WFLAGS += \
    -Werror=sizeof-pointer-memaccess \
    -Werror=stringop-overflow \
    -Werror=format
endif # ($(call wlan_version_ge,$(GCCVER),8.0),TRUE)

WFLAGS_EXTRA_A := \
    -Wclobbered \
    -Wmissing-parameter-type \
    -Wold-style-declaration \
    -Wunused-but-set-variable \
    -Wmaybe-uninitialized

WFLAGS_EXTRA_B := \
    -Wmissing-field-initializers

# more stric syntax checking accordingly
#ifneq ($(CC),clang)
ifeq ($(filter clang,$(CC)),)
  #WFLAGS += ${WFLAGS_EXTRA_A}
endif # CC != clang

ifeq ($(CONFIG_ARCH_HISI),y)
  WFLAGS += ${WFLAGS_EXTRA_B}
endif # CONFIG_ARCH_HISI = y


##### common feature
DHDCFLAGS += ${WFLAGS}

DHDCFLAGS += -Dlinux -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DBCMDMA32        \
	-DBCMFILEIMAGE -DDHDTHREAD -DSHOW_EVENTS -DWLP2P -DWIFI_ACT_FRAME  \
	-DARP_OFFLOAD_SUPPORT -DKEEP_ALIVE -DPKT_FILTER_SUPPORT            \
	-DEMBEDDED_PLATFORM -DSEC_ENHANCEMENT -DDHD_USE_RANDMAC            \
	-DUSE_NEW_RSPEC_DEFS -DWL_P2P_USE_RANDMAC -DBCMUTILS_ERR_CODES \
	-DWL_ENABLE_P2P_IF -DWL_NEWCFG_PRIVCMD_SUPPORT

##### optional feature
# FW download manner should be either:
#  DHD_LINUX_STD_FW_API: standard kernel FW API
#  DHD_SUPPORT_VFS_CALL: legacy file operation API
DHDCFLAGS += -DDHD_SUPPORT_VFS_CALL

DHDCFLAGS += -DHW_SKIP_CMD52_TIMEOUT_LOG

DHDCFLAGS += -DWL_VIRTUAL_APSTA
DHDCFLAGS += -DPOWERUP_MAX_RETRY=0
DHDCFLAGS += -DDHD_NOTIFY_MAC_CHANGED
ifeq ($(CONFIG_ARCH_HISI),y)
  DHDCFLAGS += -DBOARD_HIKEY  # get memory dump
  DHDCFLAGS += -DSHOW_LOGTRACE -DDHD_COREDUMP
  ### send up the event as skb packet and some WL command like 'escanresults' may need this
  DHDCFLAGS += -DWL_EVENT_ENAB
endif # CONFIG_ARCH_HISI = y

# Debugability
ifeq ($(CONFIG_BCMDHD_LOG),y)
  CONFIG_BCMDHD_DEBUG = y
endif # CONFIG_BCMDHD_LOG
ifeq ($(CONFIG_BCMDHD_DEBUG),y)
  DHDCFLAGS += -DDHD_DEBUG
  DHDCFLAGS += -DBCMDBG
  # Dongle init fail
  DHDCFLAGS += -DDEBUG_DNGL_INIT_FAIL
  #DHDCFLAGS += -DDHD_FW_COREDUMP
  DHDCFLAGS += -DBCMDBG_MEM
  DHDCFLAGS += -DDHD_RND_DEBUG
  DHDCFLAGS += -DBCMPERFSTATS
  DHDCFLAGS += -DDNGL_EVENT_SUPPORT
  DHDCFLAGS += -DREPORT_FATAL_TIMEOUTS
  DHDCFLAGS += -DDEVICE_TX_STUCK_DETECT
  DHDCFLAGS += -DTX_STATUS_LATENCY_STATS
  DHDCFLAGS += -DAWDL_SLOT_STATS
  DHDCFLAGS += -DDBG_RING_LOG_INIT_DEFAULT
  #DHDCFLAGS += -DBCMDBG_ASSERT
  DHDCFLAGS += -DNO_BCMDBG_ASSERT
  DHDCFLAGS += -DDBG_PKT_MON
  DHDCFLAGS += -DRSSI_MONITOR_SUPPORT
  DHDCFLAGS += -DSUPPORT_RSSI_SUM_REPORT
  DHDCFLAGS += -DSET_SSID_FAIL_CUSTOM_RC=100
  # issue TRAP related
  DHDCFLAGS += -DDHD_DETECT_CONSECUTIVE_MFG_HANG
  DHDCFLAGS += -DBCM_DETECT_TURN_ON_FAILURE
  ifeq ($(CONFIG_BCMDHD_PCIE),y)
    DHDCFLAGS += -DDHD_TIMESYNC
  endif # CONFIG_BCMDHD_PCIE
  ifeq ($(CONFIG_BCMDHD_SDIO),y)
    DHDCFLAGS += -DDEBUGABILITY
  endif # CONFIG_BCMDHD_SDIO
  ifeq ($(CONFIG_BUILD_CFG80211),y)
    DHDCFLAGS += -DDEBUGFS_CFG80211
  endif # CONFIG_BUILD_CFG80211

  ifeq ($(CONFIG_FIB_RULES),y)
    # HAL File dump is supported only for iptable builds(brcm_wlan_iptables_defconfig)
    #DHDCFLAGS += -DDHD_FILE_DUMP_EVENT
  endif
endif # CONFIG_BCMDHD_DEBUG

ifeq ($(CONFIG_BCMDHD_LOG),y)
  # Enable Log Dump
  DHDCFLAGS += -DDHD_LOG_DUMP
  DHDCFLAGS += -DSHOW_LOGTRACE
  # coredump
  #DHDCFLAGS += -DDHD_FW_COREDUMP
  #  depends on DHD_LOG_DUMP
  DHDCFLAGS += -DDHD_DUMP_FILE_WRITE_FROM_KERNEL
  # Enable log print rate limit
  DHDCFLAGS += -DDHD_LOG_PRINT_RATE_LIMIT
  # Logging
  DHDCFLAGS += -DDHD_8021X_DUMP
  DHDCFLAGS += -DDHD_DHCP_DUMP
  DHDCFLAGS += -DDHD_ICMP_DUMP
  DHDCFLAGS += -DDHD_ARP_DUMP
  DHDCFLAGS += -DDHD_DNS_DUMP
  DHDCFLAGS += -DDHD_PKTDUMP_ROAM
  DHDCFLAGS += -DDHD_IFDEBUG
  DHDCFLAGS += -DCUSTOM_LOG_DUMP_BUFSIZE_MB=1
  DHDCFLAGS += -DDHD_DBG_DUMP
  # Print out kernel panic point of file and line info when assertion happened
  #DHDCFLAGS += -DBCMASSERT_LOG
  # Enable SI utility error log
  DHDCFLAGS += -DSI_ERROR_ENFORCE
  ifeq ($(CONFIG_BCMDHD_PCIE),y)
    # Enable SSSR Dump
    DHDCFLAGS += -DDHD_SSSR_DUMP
    # Enable SMD/Minidump collection
    DHDCFLAGS += -DD2H_MINIDUMP
    # ROT and Scan timeout debugging due to Kernel scheduling problem
    DHDCFLAGS += -DDHD_KERNEL_SCHED_DEBUG
    # EDL
    DHDCFLAGS += -DEWP_EDL
    DHDCFLAGS += -DEWP_ETD_PRSRV_LOGS
    # EWP
    DHDCFLAGS += -DDHD_PCIE_REG_ACCESS
    DHDCFLAGS += -DEWP_ECNTRS_LOGGING
    DHDCFLAGS += -DEWP_RTT_LOGGING
    DHDCFLAGS += -DDHD_HMAPTEST
    DHDCFLAGS += -DEVENT_LOG_RATE_HC
    DHDCFLAGS += -DCONFIG_BCMDHD_PREALLOC_PKTIDMAP
    DHDCFLAGS += -DCONFIG_BCMDHD_PREALLOC_MEMDUMP
    DHDCFLAGS += -DCONFIG_BCMDHD_GET_OOB_STATE
  endif # CONFIG_BCMDHD_PCIE
endif # CONFIG_BCMDHD_LOG

# turn on below key features supported
DHDCFLAGS += -DDHD_MSG_LEVEL_SUPPORT
DHDCFLAGS += -DDHD_FW_LOG_SUPPORT
DHDCFLAGS += -DDHD_FW_CONS_SUPPORT

# for debug trap
DHDCFLAGS += -DDHD_USE_WARN_RATHER_THAN_BUG_ON
DHDCFLAGS += -DWL_CFGVENDOR_SEND_HANG_EVENT

ifeq ($(CONFIG_FIB_RULES),y)
  # HAL File dump is supported only for iptable builds(brcm_wlan_iptables_defconfig)
  #DHDCFLAGS := $(filter-out -DDHD_DUMP_FILE_WRITE_FROM_KERNEL ,$(DHDCFLAGS))
endif

# Enable SBN feature
DHDCFLAGS += -DDHD_SBN

# customer configuration file
DHDCFLAGS += -DREAD_CONFIG_FROM_FILE

# MBO/OCE
DHDCFLAGS += -DWL_OCE
DHDCFLAGS += -DWL_MBO

ifneq ($(CONFIG_WIRELESS_EXT),)
	DHDCFLAGS += -DWL_WIRELESS_EXT -DUSE_IW
endif

# Wapi
DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

# CCX
DHDCFLAGS += -DBCMCCX

# FBT
DHDCFLAGS += -DWLFBT

# OKC
DHDCFLAGS += -DOKC_SUPPORT

# monitor feature
#ifeq ($(CONFIG_BCMDHD_PCIE),y)
#  #Support Monitor Mode only under PCIE
#  DHDCFLAGS += -DWL_MONITOR
#endif # CONFIG_BCMDHD_PCIE
# DHDCFLAGS += -DDBG_PKT_MON
# DHDCFLAGS += -DHOST_RADIOTAP_CONV

ifeq ($(CONFIG_BCMDHD_PKTMON),y)
# For smartmu feature , both of the following MACRO have to be enabled
	DHDCFLAGS += -DWL_MON_OWN_PKT
	DHDCFLAGS += -DWL_MONITOR
endif

# CFG80211
ifeq ($(CONFIG_BUILD_CFG80211),y)
  DHDCFLAGS += -DWL_CFG80211
  ifeq ($(CONFIG_CFG80211_FILS_BKPORT),y)
    DHDCFLAGS += -DWL_FILS
  endif # CONFIG_CFG80211_FILS_BKPORT
  ifeq ($(CONFIG_CFG80211_FILS_ROAM_BKPORT),y)
    DHDCFLAGS += -DWL_FILS_ROAM_OFFLD
    DHDCFLAGS += -DCFG80211_ROAM_API_GE_4_12
  endif # CONFIG_CFG80211_FILS_ROAM_BKPORT

  # Android iface management
  DHDCFLAGS += -DWL_IFACE_MGMT

  # for WPA3
  ifeq ($(CONFIG_WPA3_SUPPORT),y)
    # old kernel needs back port support
    ifneq ($(call wlan_version_gt,$(BCM_KVERSIONSTRING),4.16),TRUE)
    CONFIG_CFG80211_SAE_BKPORT ?= y
    endif # ($(call wlan_version_gt,$(BCM_KVERSIONSTRING),4.16),TRUE)

    # WPA3 external authenticator
    ### for external supplicant (only kernel > 4.16 support external authenticator)
    ifeq ($(call wlan_version_gt,$(BCM_KVERSIONSTRING),4.16),TRUE)
    DHDCFLAGS += -DWL_CLIENT_SAE
    endif # ($(call wlan_version_gt,$(BCM_KVERSIONSTRING),4.16),TRUE)
    ### for dongle in-driver-supplicant
    # DHDCFLAGS += -DWL_SAE
  endif # CONFIG_WPA3_SUPPORT

  ifeq ($(CONFIG_CFG80211_SCANTYPE_BKPORT),y)
    DHDCFLAGS += -DWL_SCAN_TYPE
  endif # CONFIG_CFG80211_SCANTYPE_BKPORT
endif # CONFIG_BUILD_CFG80211

DHDCFLAGS += -DWL_CLIENT_SAE

# Block ARP during DHCP on STA/SoftAP concurrent mode
DHDCFLAGS += -DAPSTA_BLOCK_ARP_DURING_DHCP

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  # Enable Load Balancing support by default.
  # DHD_LB_RXP - Perform RX Packet processing in parallel
  # DHD_LB_STATS - To display the Load Blancing statistics
  DHDCFLAGS += -DDHD_LB -DDHD_LB_RXP -DDHD_LB_TXP -DDHD_LB_STATS
  DHDCFLAGS += -DDHD_LB_PRIMARY_CPUS=0xF0 -DDHD_LB_SECONDARY_CPUS=0x0E
  # Use spin_lock_bh locks
  DHDCFLAGS += -DDHD_USE_SPIN_LOCK_BH
  # Enable CTO Recovery
  DHDCFLAGS += -DBCMPCIE_CTO_PREVENTION
  # Enable inband device wake feature
  DHDCFLAGS += -DPCIE_INB_DW

  # IRQSET in DHD
  ifeq ($(CONFIG_BCMDHD),y) # only turn on when built-in since it requires kernel non-export '__irq_set_affinity'
    DHDCFLAGS += -DDHD_LB_IRQSET
  endif # CONFIG_BCMDHD
  DHDCFLAGS += -DDHD_MSI_SUPPORT
  # Enable PKTID AUDIT
  DHDCFLAGS += -DDHD_PKTID_AUDIT_ENABLED
  DHDCFLAGS += -DIDLE_TX_FLOW_MGMT
endif # CONFIG_BCMDHD_PCIE

# CUSTOMER flags

# Basic / Common Feature
DHDCFLAGS += -DDHDTCPACK_SUPPRESS
DHDCFLAGS += -DDISABLE_FRAMEBURST_VSDB
DHDCFLAGS += -DUSE_WL_FRAMEBURST
DHDCFLAGS += -DUSE_WL_TXBF
DHDCFLAGS += -DSOFTAP_UAPSD_OFF
DHDCFLAGS += -DVSDB
DHDCFLAGS += -DWL_CFG80211_STA_EVENT
DHDCFLAGS += -DWL11U
DHDCFLAGS += -DMFP

DHDCFLAGS += -DWIPHY_DYNAMIC_UPDATE
#use customer country code mapping table
DHDCFLAGS += -DCUSTOM_COUNTRY_CODE

# Android Feature
ifeq ($(CONFIG_ANDROID_PLATFORM),y)
  DHDCFLAGS += -DOEM_ANDROID
  DHDCFLAGS += -DGET_CUSTOM_MAC_ENABLE
  CONFIG_DHD_SET_RANDOM_MAC_VAL ?= 0x001A11
  DHDCFLAGS += -DPNO_SUPPORT
  # Scheduled scan (PNO)
  DHDCFLAGS += -DWL_SCHED_SCAN
  # Gscan needs PNO
  DHDCFLAGS += -DGSCAN_SUPPORT
  DHDCFLAGS += -DAPF
  DHDCFLAGS += -DDHD_GET_VALID_CHANNELS
  DHDCFLAGS += -DLINKSTAT_SUPPORT
  DHDCFLAGS += -DPFN_SCANRESULT_2
  DHDCFLAGS += -DWL_IFACE_COMB_NUM_CHANNELS
  # FW ROAM control
  DHDCFLAGS += -DROAMEXP_SUPPORT
  #FAKEAP
  DHDCFLAGS += -DWL_BCNRECV
  #ADDTS
  DHDCFLAGS += -DWL_CAC_TS
else # CONFIG_ANDROID_PLATFORM
  DHDCFLAGS += -DDISABLE_ANDROID_PNO
endif # CONFIG_ANDROID_PLATFORM

# MAC
DHDCFLAGS += -DENABLE_ARP_SNOOP_MODE
# Skip supplicant bssid and channel hints
DHDCFLAGS += -DWL_SKIP_CONNECT_HINTS
# Phy / System
DHDCFLAGS += -DDHD_ENABLE_LPC
DHDCFLAGS += -DDISABLE_PM_BCNRX
DHDCFLAGS += -DFCC_PWR_LIMIT_2G
DHDCFLAGS += -DSUPPORT_2G_VHT
DHDCFLAGS += -DSUPPORT_5G_1024QAM_VHT
DHDCFLAGS += -DSUPPORT_LTECX
DHDCFLAGS += -DSUPPORT_LQCM
DHDCFLAGS += -DSUPPORT_SET_CAC
DHDCFLAGS += -DSUPPORT_WL_TXPOWER
# Roaming feature
DHDCFLAGS += -DDHD_LOSSLESS_ROAMING
DHDCFLAGS += -DWL_ROAM_TIMEOUT_MS=2000 # ms
DHDCFLAGS += -DENABLE_FW_ROAM_SUSPEND
DHDCFLAGS += -DROAM_API
DHDCFLAGS += -DROAM_AP_ENV_DETECTION
DHDCFLAGS += -DROAM_CHANNEL_CACHE
DHDCFLAGS += -DROAM_ENABLE
DHDCFLAGS += -DSKIP_ROAM_TRIGGER_RESET
DHDCFLAGS += -DWBTEXT
DHDCFLAGS += -DWBTEXT_BTMDELTA=0
DHDCFLAGS += -DWBTEXT_SCORE_V2
DHDCFLAGS += -DRRM_BCNREQ_MAX_CHAN_TIME=12
DHDCFLAGS += -DWL_LASTEVT
DHDCFLAGS += -DROAM_EVT_DISABLE
# Wake
DHDCFLAGS += -DCONFIG_HAS_WAKELOCK
# Enable wakelock debug function
DHDCFLAGS += -DDHD_TRACE_WAKE_LOCK
DHDCFLAGS += -DDHD_WAKE_EVENT_STATUS
DHDCFLAGS += -DDHD_WAKE_RX_STATUS
#DHDCFLAGS += -DDHD_USE_SCAN_WAKELOCK

#Android Q
DHDCFLAGS += -DWL_USE_RANDOMIZED_SCAN
DHDCFLAGS += -DSTA_RANDMAC_ENFORCED
#Connected MAC randomization
DHDCFLAGS += -DWL_STA_ASSOC_RAND
#Soft AP MAC randomization
DHDCFLAGS += -DWL_SOFTAP_RAND
#p2p MAC randomization
DHDCFLAGS += -DWL_P2P_RAND
# Enable below define for production
#DHDCFLAGS += -DMACADDR_PROVISION_ENFORCED
DHDCFLAGS += -DVENDOR_HW_UPDATE

DHDCFLAGS += -DHW_NVRAM_VARS_CALIBRATE
DHDCFLAGS += -DSUPPORT_WT_WIFI_MIMO_CALIBRATE

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  DHDCFLAGS += -DDHD_WAKE_STATUS
  DHDCFLAGS += -DDHD_WAKE_EVENT_STATUS
  DHDCFLAGS += -DDHD_WAKE_RX_STATUS
endif # CONFIG_BCMDHD_PCIE
DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP
DHDCFLAGS += -DSET_SSID_FAIL_CUSTOM_RC=100
# Packet
#DHDCFLAGS += -DBLOCK_IPV6_PACKET
#DHDCFLAGS += -DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT # NAN test failure
DHDCFLAGS += -DENABLE_IPMCAST_FILTER
DHDCFLAGS += -DPASS_ALL_MCAST_PKTS
DHDCFLAGS += -DPKTPRIO_OVERRIDE

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  # Kernel/Platform Related Feature
  #DHDCFLAGS += -DDHD_LB_TXP_DEFAULT_ENAB # Not needed for Brix
  #DHDCFLAGS += -DDHD_RECOVER_TIMEOUT # Not needed for Brix
  #DHDCFLAGS += -DDHD_USE_ATOMIC_PKTGET # Not needed for Brix
  #DHDCFLAGS += -DDHD_USE_COHERENT_MEM_FOR_RING # Not needed for Brix
  #DHDCFLAGS += -DDONGLE_ENABLE_ISOLATION # Not compatible with Brix platform
  #DHDCFLAGS += -DKEEP_WIFION_OPTION # Not compatible with Brix platform
endif # CONFIG_BCMDHD_PCIE

DHDCFLAGS += -DWL_SUPPORT_BACKPORTED_KPATCHES
# SoftAP
DHDCFLAGS += -DHW_SOFTAP_MANAGEMENT
DHDCFLAGS += -DHW_SOFTAP_REASON_CODE
DHDCFLAGS += -DSUPPORT_AP_HIGHER_BEACONRATE
DHDCFLAGS += -DSUPPORT_AP_RADIO_PWRSAVE
DHDCFLAGS += -DSUPPORT_HIDDEN_AP
DHDCFLAGS += -DSUPPORT_SOFTAP_SINGL_DISASSOC
DHDCFLAGS += -DWL_SUPPORT_AUTO_CHANNEL
DHDCFLAGS += -DSUPPORT_SOFTAP_WPAWPA2_MIXED
# P2P
DHDCFLAGS += -DP2P_LISTEN_OFFLOADING
DHDCFLAGS += -DP2P_SKIP_DFS
# P2P GO Negotiation
# DHDCFLAGS += -DWL_CFG80211_GON_COLLISION

# SCAN
DHDCFLAGS += -DCUSTOMER_SCAN_TIMEOUT_SETTING
DHDCFLAGS += -DDISABLE_PRUNED_SCAN
DHDCFLAGS += -DESCAN_BUF_OVERFLOW_MGMT
DHDCFLAGS += -DUSE_INITIAL_SHORT_DWELL_TIME
DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
DHDCFLAGS += -DSUPPORT_RANDOM_MAC_SCAN
DHDCFLAGS += -DCUSTOM_SCAN_PASSIVE_TIME=110
DHDCFLAGS += -DCUSTOM_FORCE_NODFS_FLAG    #SCAN time

# Suspend/Resume
DHDCFLAGS += -DENABLE_MAX_DTIM_IN_SUSPEND
DHDCFLAGS += -DSUPPORT_DEEP_SLEEP
# Misc Features
DHDCFLAGS += -DDHD_BLOB_EXISTENCE_CHECK
DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD
DHDCFLAGS += -DWL_RELMCAST
DHDCFLAGS += -DWL_SUPP_EVENT
DHDCFLAGS += -DDISABLE_WL_FRAMEBURST_SOFTAP
DHDCFLAGS += -DFILTER_IE
DHDCFLAGS += -DCUSTOM_LONG_RETRY_LIMIT=12
# usually the upper layer supplicant can buffer the M1 before associated
#DHDCFLAGS += -DDHD_POST_EAPOL_M1_AFTER_ROAM_EVT
# Kind of WAR
DHDCFLAGS += -DENABLE_TDLS_AUTO_MODE
DHDCFLAGS += -DEXPLICIT_DISCIF_CLEANUP
DHDCFLAGS += -DSKIP_WLFC_ON_CONCURRENT
DHDCFLAGS += -DCUSTOM_BLOCK_DEAUTH_AT_EAP_FAILURE
DHDCFLAGS += -DTDLS_MSG_ONLY_WFD
# Custom tuning value
DHDCFLAGS += -DCUSTOM_ROAM_TIME_THRESH_IN_SUSPEND=6000
DHDCFLAGS += -DCUSTOM_EVENT_PM_WAKE=30
DHDCFLAGS += -DCUSTOM_KEEP_ALIVE_SETTING=30000
# For special PNO Event keep wake lock for 10sec
DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=10
DHDCFLAGS += -DSPECIFIC_MAC_GEN_SCHEME
DHDCFLAGS += -DSET_RANDOM_MAC_SOFTAP
ifeq ($(CONFIG_DHD_SET_RANDOM_MAC_VAL),y)
  DHDCFLAGS += -DCONFIG_DHD_SET_RANDOM_MAC_VAL=$(CONFIG_DHD_SET_RANDOM_MAC_VAL)
endif # CONFIG_DHD_SET_RANDOM_MAC_VAL
DHDCFLAGS += -DCUSTOM_TDLS_IDLE_MODE_SETTING=10000
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_HIGH=-80
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_LOW=-85
DHDCFLAGS += -DCUSTOM_TCPACK_SUPP_RATIO=15
DHDCFLAGS += -DCUSTOM_TCPACK_DELAY_TIME=10
DHDCFLAGS += -DD3_ACK_RESP_TIMEOUT=4000
DHDCFLAGS += -DIOCTL_RESP_TIMEOUT=5000
DHDCFLAGS += -DMAX_DTIM_ALLOWED_INTERVAL=925
DHDCFLAGS += -DNUM_SCB_MAX_PROBE=3
DHDCFLAGS += -DWL_SCB_TIMEOUT=10
# need to tune according if needs
DHDCFLAGS += -DWIFI_TURNON_DELAY=200
DHDCFLAGS += -DWIFI_TURNOFF_DELAY=10
# AMPDU
DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64

#Static preallocated buffers
DHDCFLAGS += -DDHD_USE_STATIC_MEMDUMP

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  DHDCFLAGS += -DDHD_USE_STATIC_CTRLBUF
  #Use coherent pool
  DHDCFLAGS += -DDHD_USE_COHERENT_MEM_FOR_RING

  # 4way handshake disconnection feature
  DHDCFLAGS += -DDHD_4WAYM4_FAIL_DISCONNECT

  # Enabling this flag will allocate the coherent memory
  # from Atomic pool. For 64 bit ARM arch the default
  # coherent memory pool size is 256K. This size has to be
  # increased to 4M if the DHD uses this feature. So if the
  # BSP cannot be modified DHD should not be allocating from
  # coherent pool.
  #DHDCFLAGS += -DDHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL

  ## OOB
  ifeq ($(CONFIG_BCMDHD_OOB_HOST_WAKE),y)
    DHDCFLAGS += -DBCMPCIE_OOB_HOST_WAKE
    DHDCFLAGS += -DDHD_USE_PCIE_OOB_THREADED_IRQ
  endif # CONFIG_BCMDHD_OOB_HOST_WAKE
endif # CONFIG_BCMDHD_PCIE

# DMA64 suppports on 64bit Architecture
ifeq ($(CONFIG_64BIT),y)
  DHDCFLAGS := $(filter-out -DBCMDMA32,$(DHDCFLAGS))
  DHDCFLAGS += -DBCMDMA64OSL
endif # CONFIG_64BIT

DHDCFLAGS += -DVSDB

# TDLS enable
DHDCFLAGS += -DWLTDLS -DWLTDLS_AUTO_ENABLE
# For TDLS tear down inactive time 40 sec
DHDCFLAGS += -DCUSTOM_TDLS_IDLE_MODE_SETTING=10000
# for TDLS RSSI HIGH for establishing TDLS link
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_HIGH=-80
# for TDLS RSSI HIGH for tearing down TDLS link
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_LOW=-85

# Roaming trigger
DHDCFLAGS += -DCUSTOM_ROAM_TRIGGER_SETTING=-75
DHDCFLAGS += -DCUSTOM_ROAM_DELTA_SETTING=10
# Set PM 2 always regardless suspend/resume
DHDCFLAGS += -DSUPPORT_PM2_ONLY
# remove this if allow DHD driver manage PM automatically
DHDCFLAGS += -DDISABLE_AUTO_CONTROL_PM

DHDCFLAGS += -DMIRACAST_AMPDU_SIZE=8
#Vendor Extension
ifeq ($(call wlan_version_ge,$(BCM_KVERSIONSTRING),3.14.0),TRUE)
  DHDCFLAGS += -DWL_VENDOR_EXT_SUPPORT
  ifeq ($(CONFIG_BUILD_CFG80211),y)
  DHDCFLAGS += -DBCM_PRIV_CMD_SUPPORT
  DHDCFLAGS += -DWL_PRIV_CMD_OVER_NL80211
  endif # CONFIG_BUILD_CFG80211
endif # wlan_version_ge

#RSSI Monitor
DHDCFLAGS += -DRSSI_MONITOR_SUPPORT
#RTT
DHDCFLAGS += -DRTT_SUPPORT -DRTT_DEBUG -DWL_RTT_LCI -DWL_RTT_ONE_WAY
#NDOffload
DHDCFLAGS += -DNDO_CONFIG_SUPPORT
DHDCFLAGS += -DIPV6_NDO_SUPPORT

# Early suspend
DHDCFLAGS += -DDHD_USE_EARLYSUSPEND

# For Scan result patch
DHDCFLAGS += -DESCAN_RESULT_PATCH
DHDCFLAGS += -DDUAL_ESCAN_RESULT_BUFFER

# NAN
DHDCFLAGS += -DWL_NAN -DWL_NAN_DISC_CACHE -DWL_NANP2P

# CSI
DHDCFLAGS += -DCSI_SUPPORT

# SAR Tx power scenario
DHDCFLAGS += -DWL_SAR_TX_POWER

# OWE
DHDCFLAGS += -DWL_OWE

# Silent roam
DHDCFLAGS += -DCONFIG_SILENT_ROAM

# Disable HE on P2P based on peer support
DHDCFLAGS += -DWL_DISABLE_HE_P2P

# For Static Buffer
ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
  DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
  DHDCFLAGS += -DENHANCED_STATIC_BUF
  DHDCFLAGS += -DSTATIC_WL_PRIV_STRUCT
  #static if
  DHDCFLAGS += -DWL_STATIC_IF
  DHDCFLAGS += -DCONFIG_BROADCOM_WIFI_RESERVED_MEM
endif # CONFIG_DHD_USE_STATIC_BUF

DHDCFLAGS += -DWL_STATIC_IF

# Ioctl timeout 5000ms
DHDCFLAGS += -DIOCTL_RESP_TIMEOUT=5000

# Prevent rx thread monopolize
DHDCFLAGS += -DWAIT_DEQUEUE

# idle count
DHDCFLAGS += -DDHD_USE_IDLECOUNT

# SKB TAILPAD to avoid out of boundary memory access
DHDCFLAGS += -DDHDENABLE_TAILPAD

# Disable FRAMEBURST on VSDB
DHDCFLAGS += -DDISABLE_FRAMEBURST_VSDB

# WPS
DHDCFLAGS += -DWL_WPS_SYNC -DBCMCRYPTO_COMPONENT

# TCP SYNC flood attack block
DHDCFLAGS += -DDHDTCPSYNC_FLOOD_BLK -DTCP_SYNC_FLOOD_LIMIT=50

# Path name to store the FW Debug symbol files
DHDCFLAGS += -DPLATFORM_PATH="\"/data/misc/wifi/\""

#???
DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=5

DHDCFLAGS += -DDISABLE_1024QAM_VHT
##########################
# driver type
# m: module type driver
# y: built-in type driver
##########################
DRIVER_TYPE ?= $(CONFIG_BCMDHD)

##### Chip dependent feature
ifneq ($(filter BCM43, $(CONFIG_CHIPSET)),)
  # New Features
  DHDCFLAGS += -D$(CONFIG_CHIPSET) -DWL11AX -DWL11AC
endif # CONFIG_BCM4375

ifeq ($(CONFIG_BCMDHD_SDIO),y)
  # DHDCFLAGS += -DTOE
  DHDCFLAGS += -DBDC -DDHD_BCMEVENTS -DMMC_SDIO_ABORT
  DHDCFLAGS += -DBCMSDIO -DUSE_SDIOFIFO_IOVAR
  ifeq ($(CONFIG_BCMDHD_SDIO_SDSTD),y)
    DHDCFLAGS += -DBCMSDIOH_STD
    DHDCFLAGS += -DBCMSDIOH_STD_TUNING_WAR
  endif # CONFIG_BCMDHD_SDIO_SDSTD
  ifeq ($(CONFIG_BCMDHD_SDIO_SDMMC),y)
    DHDCFLAGS += -DBCMLXSDMMC
  endif # CONFIG_BCMDHD_SDIO_SDMMC
  DHDCFLAGS += -DPROP_TXSTATUS
  DHDCFLAGS += -DLIMIT_BORROW
  ifeq ($(CONFIG_BCMDHD_OOB),y)
    DHDCFLAGS += -DOOB_INTR_ONLY
    DHDCFLAGS += -DHW_OOB
    ifeq ($(CONFIG_BCMDHD_DISABLE_WOWLAN),y)
      DHDCFLAGS += -DDISABLE_WOWLAN
    endif # CONFIG_BCMDHD_DISABLE_WOWLAN
  else
    ifeq ($(CONFIG_BCMDHD_SDIO_SDMMC),y)
      DHDCFLAGS += -DSDIO_ISR_THREAD
    endif # CONFIG_BCMDHD_SDIO_SDMMC
  endif # CONFIG_BCMDHD_OOB
  DHDCFLAGS += -DSDTEST
  # tput enhancement
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DCUSTOM_SDIO_F2_BLKSIZE=256
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=128
  DHDCFLAGS += -DBCMSDIOH_TXGLOM
  DHDCFLAGS += -DCUSTOM_TXGLOM=$(CONFIG_BCMDHD_SDIO_TXGLOM_MANNER)
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DREPEAT_READFRAME
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=36
  DHDCFLAGS += -DDHD_SDIO_DYN_TXGLOM_SIZE

  DHDCFLAGS += -DWLFC_STATE_PREALLOC

  DHDCFLAGS += -DPROP_TXSTATUS_VSDB

  DHDCFLAGS += -DCUSTOM_DPC_PRIO_SETTING=98
  DHDCFLAGS += -DCUSTOM_RXF_PRIO_SETTING=99
  DHDCFLAGS += -DDHD_TXBOUND=72
  DHDCFLAGS += -DDHD_SDALIGN=4

  #Enable set rxf and dpc to different CPUCORE to enhance Ixia Tput
  DHDCFLAGS += -DCUSTOM_SET_CPUCORE
  DHDCFLAGS += -DPRIMARY_CPUCORE=1
  DHDCFLAGS += -DDPC_CPUCORE=5
  DHDCFLAGS += -DRXF_CPUCORE=4
  DHDCFLAGS += -DMAX_RETRY_SET_CPUCORE=10
  DHDCFLAGS += -DDHD_RXBOUND=512
  DHDCFLAGS += -DMAX_HDR_READ=256
  DHDCFLAGS += -DDHD_FIRSTREAD=256

endif # CONFIG_BCMDHD_SDIO

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  DHDCFLAGS += -DPCIE_FULL_DONGLE -DBCMPCIE -DCUSTOM_DPC_PRIO_SETTING=-1
  # HEAP ASLR
  DHDCFLAGS += -DBCM_ASLR_HEAP

  DHDCFLAGS += -DMAX_CNTL_TX_TIMEOUT=1
  ifneq ($(filter y, $(CONFIG_ARCH_MSM) $(CONFIG_ARCH_QCOM)),)
    DHDCFLAGS += -DMSM_PCIE_LINKDOWN_RECOVERY
  endif # CONFIG_ARCH_MSM || CONFIG_ARCH_QCOM
  ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
    DHDCFLAGS += -DDHD_USE_STATIC_IOCTLBUF
  endif

  # Enable health check event handling
  DHDCFLAGS += -DDNGL_EVENT_SUPPORT
  DHDCFLAGS += -DHCHK_COMMON_SW_EVENT
endif # CONFIG_BCMDHD_PCIE

# Expand TCP tx queue
DHDCFLAGS += -DDHD_TCP_PACING_SHIFT

# prioritize 802.1x packet
DHDCFLAGS += -DEAPOL_PKT_PRIO

ifeq ($(CONFIG_BCMDHD),y) # for builtin DHD driver
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DWAKEUP_KSOFTIRQD_POST_NAPI_SCHEDULE
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
  # Use kernel strlcpy() implementation instead of one, defined in bcmstdlib_s.c
  DHDCFLAGS += -DBCM_USE_PLATFORM_STRLCPY
else # for module DHD driver
  DHDCFLAGS += -DBCMDHD_MODULAR
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
endif # for builtin DHD driver

DHDOFILES := dhd_pno.o dhd_common.o dhd_ip.o dhd_custom_gpio.o \
    dhd_linux.o dhd_linux_sched.o dhd_linux_wq.o aiutils.o \
    bcmevent.o bcmutils.o bcmwifi_channels.o hndpmu.o linux_osl.o linux_pkt.o \
    sbutils.o siutils.o  dhd_linux_exportfs.o wl_roam.o \
    wldev_common.o wl_linux_mon.o dhd_linux_platdev.o \
    dhd_rtt.o dhd_linux_pktdump.o wl_cfg_btcoex.o hnd_pktq.o \
    hnd_pktpool.o bcmxtlv.o bcm_app_utils.o dhd_debug.o \
    dhd_debug_linux.o dhd_mschdbg.o bcmbloom.o dhd_dbg_ring.o bcmstdlib_s.o frag.o \
    wl_cfgnan.o wl_cfgscan.o hw_wifi_freq_ctrl.o

DHDOFILES += $(BCMINTERNAL_DHDOFILES)

ifneq ($(CONFIG_WIRELESS_EXT),)
	DHDOFILES += wl_iw.o
endif

ifeq ($(CONFIG_BUILD_CFG80211),y)
  DHDOFILES += dhd_cfg80211.o wl_cfg80211.o wl_cfgvendor.o wl_cfgp2p.o wl_android.o
endif # CONFIG_BUILD_CFG80211

# extra Source files
DHDOFILES += wl_cfgvif.o wl_cfg_btcoex.o

ifeq ($(CONFIG_BCMDHD_DEBUG),y)
  DHDOFILES += dhd_macdbg.o
  ifeq ($(CONFIG_BCMDHD_PCIE),y)
    DHDOFILES += dhd_timesync.o
  endif # CONFIG_BCMDHD_PCIE
endif # CONFIG_BCMDHD_DEBUG

ifeq ($(CONFIG_ARCH_HISI),y)
  DHDCFLAGS += -DBOARD_MODULAR_INIT
  # Hikey sched is not so optimized and hence need a higher timeout
  DHDCFLAGS += -DWAIT_FOR_DISCONNECT_MAX=20
  DHDOFILES += dhd_custom_hikey.o
else ifneq ($(filter y, $(CONFIG_ARCH_MSM) $(CONFIG_ARCH_QCOM)),)
  DHDCFLAGS += -DCUSTOMER_HW2
  DHDCFLAGS += -DBOARD_MODULAR_INIT
  DHDOFILES += dhd_custom_msm.o
  ifeq ($(CONFIG_ARCH_SDM660),y)
    DHDCFLAGS += -DCONFIG_ARCH_SDM660
  endif # CONFIG_ARCH_SDM660
else ifeq ($(CONFIG_ARCH_EXYNOS),y)
  DHDCFLAGS += -DBOARD_MODULAR_INIT
  DHDOFILES += dhd_custom_exynos.o
else
  # nothing to do here
endif # CONFIG_ARCH_HISI

DHDCFLAGS += -DCUSTOMER_HW2

ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
  DHDOFILES += dhd_custom_memprealloc.o
endif # CONFIG_BROADCOM_WIFI_RESERVED_MEM

ifeq ($(CONFIG_BCMDHD_SDIO),y)
  DHDOFILES += bcmsdh.o bcmsdh_linux.o
  ifeq ($(CONFIG_BCMDHD_SDIO_SDMMC),y)
  DHDOFILES += bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o
  else ifeq ($(CONFIG_BCMDHD_SDIO_SDSTD),y)
  DHDOFILES += bcmsdstd.o bcmsdstd_linux.o
  endif # CONFIG_BCMDHD_SDIO_SDMMC
  DHDOFILES += dhd_cdc.o dhd_wlfc.o dhd_sdio.o
endif # CONFIG_BCMDHD_SDIO

ifeq ($(CONFIG_BCMDHD_PCIE),y)
  DHDOFILES += dhd_pcie.o dhd_pcie_linux.o dhd_msgbuf.o dhd_flowring.o
  DHDOFILES += pcie_core.o
endif # CONFIG_BCMDHD_PCIE

ifneq ($(filter -DDHD_LB, $(DHDCFLAGS)),)
DHDOFILES += dhd_linux_lb.o
endif # DHD_LB

ifneq ($(filter -DDHD_EVENT_LOG_FILTER, $(DHDCFLAGS)),)
DHDOFILES += dhd_event_log_filter.o
endif # DHD_EVENT_LOG_FILTER

ifneq ($(filter -DREAD_CONFIG_FROM_FILE, $(DHDCFLAGS)),)
  DHDOFILES += dhd_config.o
endif # READ_CONFIG_FROM_FILE

##### Finalize
EXTRA_CFLAGS += $(DHDCFLAGS) -g
EXTRA_CFLAGS += -DSRCBASE=\"$(BCMDHD_ROOT)\" -DDHD_COMPILED=\"$(BCMDHD_ROOT)\"
EXTRA_CFLAGS += -I$(BCMDHD_ROOT)/include/ -I$(BCMDHD_ROOT)/
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(CURDIR) -Wno-date-time
EXTRA_LDFLAGS += $(DHDLDFLAGS) -g
# remove static for modular
ifeq ($(CONFIG_BCMDHD),m) # for modular DHD driver
  EXTRA_CFLAGS := $(filter-out -DCONFIG_DHD_USE_STATIC_BUF,$(EXTRA_CFLAGS))
  KBUILD_CFLAGS := $(filter-out -DCONFIG_DHD_USE_STATIC_BUF,$(KBUILD_CFLAGS))
  EXTRA_LDFLAGS := $(filter-out -DCONFIG_DHD_USE_STATIC_BUF,$(EXTRA_LDFLAGS))
endif # modular DHD driver

bcmdhd-objs := $(DHDOFILES)
obj-$(DRIVER_TYPE)   += bcmdhd.o
ccflags-y := $(KBUILD_CFLAGS)
ccflags-y += $(EXTRA_CFLAGS)

ifeq ($(call wlan_version_ge,$(BCM_KVERSIONSTRING),5.3.0),TRUE)
build_command=$(MAKE) --no-print-directory -C ${KERNEL_SRC} M=$(CURDIR) $(KBUILD_OPTIONS)
else # ($(call wlan_version_ge,$(BCM_KVERSIONSTRING),5.3.0),TRUE)
build_command=$(MAKE) --no-print-directory -C ${KERNEL_SRC} SUBDIRS=$(CURDIR) $(KBUILD_OPTIONS)
endif # ($(call wlan_version_ge,$(BCM_KVERSIONSTRING),5.3.0),TRUE)

all:
	@echo ARCH='$(ARCH)'
	@echo CROSS_COMPILE='$(CROSS_COMPILE)'
	@echo LINUXDIR='$(LINUXDIR)'
	@echo ${build_command} modules
	@${build_command} modules
	$(OBJDUMP) -d -S bcmdhd.ko > bcmdhd.debug.dis.src
	$(NM) -n --defined-only bcmdhd.ko > bcmdhd.ko.map
	$(STRIP) -g bcmdhd.ko -o bcmdhd.ko.stripped
	cp -rf bcmdhd.ko  bcmdhd.debug.ko
	cp -rf bcmdhd.ko.stripped  bcmdhd.ko

install modules_install:
	@${build_command} modules_install

clean:
	@rm -rf *.o *.ko *.ko.* *.mod *.mod.c *.dis* *~ .*.cmd *.o.cmd .*.o.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin *.o.ur-safe .*.o.d *.o.ur-detected *.map
	@echo clean done!

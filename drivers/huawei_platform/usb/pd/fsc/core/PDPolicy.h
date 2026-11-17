/*
 * PDPolicy.h
 *
 * PDPolicy driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _PDPOLICY_H_
#define	_PDPOLICY_H_

/////////////////////////////////////////////////////////////////////////////
//                              Required headers
/////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "PD_Types.h"
#include "TypeC_Types.h"
#include "vdm/vdm.h"

// EXTERNS
extern FSC_BOOL                 USBPDTxFlag;                                    // Flag to indicate that we need to send a message (set by device policy manager)
extern sopMainHeader_t          PDTransmitHeader;                               // Definition of the PD packet to send

#ifdef FSC_HAVE_SNK
extern sopMainHeader_t          CapsHeaderSink;                                 // Definition of the sink capabilities of the device
extern doDataObject_t           CapsSink[7];                                    // Power object definitions of the sink capabilities of the device

extern doDataObject_t           SinkRequest;                                    // Sink request message
extern FSC_U32                  SinkRequestMaxVoltage;                          // Maximum voltage that the sink will request
extern FSC_U32                  SinkRequestMaxPower;                            // Maximum power the sink will request (used to calculate current as well)
extern FSC_U32                  SinkRequestOpPower;                             // Operating power the sink will request (used to calculate current as well)
extern FSC_BOOL                 SinkGotoMinCompatible;                          // Whether the sink will respond to the GotoMin command
extern FSC_BOOL                 SinkUSBSuspendOperation;                        // Whether the sink wants to continue operation during USB suspend
extern FSC_BOOL                 SinkUSBCommCapable;                             // Whether the sink is USB communications capable
#endif // FSC_HAVE_SNK

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
extern sopMainHeader_t          CapsHeaderSource;                               // Definition of the source capabilities of the device
extern doDataObject_t           CapsSource[7];                                  // Power object definitions of the source capabilities of the device
#endif // FSC_HAVE_SRC

extern sopMainHeader_t          CapsHeaderReceived;                             // Last capabilities header received (source or sink)
extern doDataObject_t           PDTransmitObjects[7];                           // Data objects to send
extern doDataObject_t           CapsReceived[7];                                // Last power objects received (source or sink)
extern doDataObject_t           USBPDContract;                                  // Current USB PD contract (request object)

#ifdef FSC_DEBUG
extern FSC_BOOL                 SourceCapsUpdated;                              // Flag to indicate whether we have updated the source caps (for the GUI)
/* Tracks seconds elapsed for log timestamp */
extern volatile u16 Timer_S;
/* Tracks tenths of milliseconds elapsed for log timestamp */
extern volatile u16 Timer_tms;
#endif // FSC_DEBUG

extern PolicyState_t            PolicyState;                                    // State variable for Policy Engine
extern FSC_U8                   PolicySubIndex;                                 // Sub index for policy states
extern FSC_BOOL                 PolicyIsSource;                                 // Flag to indicate whether we are acting as a source or a sink
extern FSC_BOOL                 PolicyIsDFP;                                    // Flag to indicate whether we are acting as a UFP or DFP
extern FSC_BOOL                 PolicyHasContract;                              // Flag to indicate whether there is a contract in place
extern FSC_U32                  VbusTransitionTime;                             // Time to wait for VBUS switch to transition

extern sopMainHeader_t          PolicyRxHeader;                                 // Header object for USB PD messages received
extern sopMainHeader_t          PolicyTxHeader;                                 // Header object for USB PD messages to send
extern doDataObject_t           PolicyRxDataObj[7];                             // Buffer for data objects received
extern doDataObject_t           PolicyTxDataObj[7];                             // Buffer for data objects to send

extern TIMER                  NoResponseTimer;                                // Policy engine no response timer

#ifdef FSC_HAVE_VDM
extern TIMER VdmTimer;
extern FSC_BOOL VdmTimerStarted;
extern VdmManager vdmm;
extern FSC_BOOL mode_entered;
extern SvidInfo core_svid_info;
extern FSC_BOOL ExpectingVdmResponse;
#endif /* FSC_HAVE_VDM */

#ifdef FSC_HAVE_DP
extern u32 DpModeEntered;
extern s32 AutoModeEntryObjPos;
#endif /* FSC_HAVE_DP */

extern PolicyState_t originalPolicyState;
/* Set to 1 to enable manual retries */
extern u8 manualRetriesEnabled;
/* Number of tries for manual retry */
extern u8 nRetries;
/* Tracks seconds elapsed for log timestamp */
extern FSC_BOOL g_Idle;
/* Puts state machine into Idle state */
extern USBTypeCCurrent SourceCurrent;
/* TypeC advertised current */
extern u8 DetachThreshold;
extern FSC_BOOL ProtocolCheckRxBeforeTx;
/* Used to count the number of Unattach<->AttachWait loops */
extern u8 loopCounter;

/////////////////////////////////////////////////////////////////////////////
//                            LOCAL PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
void InitializePDPolicyVariables(void);
void USBPDEnable(FSC_BOOL DeviceUpdate, SourceOrSink TypeCDFP);
void USBPDDisable(FSC_BOOL DeviceUpdate);
void USBPDPolicyEngine(void);
void PolicyErrorRecovery(void);

#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
void PolicySourceSendHardReset(void);
void PolicySourceSoftReset(void);
void PolicySourceSendSoftReset(void);
void PolicySourceStartup(void);
void PolicySourceDiscovery(void);
void PolicySourceSendCaps(void);
void PolicySourceDisabled(void);
void PolicySourceTransitionDefault(void);
void PolicySourceNegotiateCap(void);
void PolicySourceTransitionSupply(void);
void PolicySourceCapabilityResponse(void);
void PolicySourceReady(void);
void PolicySourceGiveSourceCap(void);
void PolicySourceGetSourceCap(void);
void PolicySourceGetSinkCap(void);
void PolicySourceSendPing(void);
void PolicySourceGotoMin(void);
void PolicySourceGiveSinkCap(void);
void PolicySourceSendDRSwap(void);
void PolicySourceEvaluateDRSwap(void);
void PolicySourceSendVCONNSwap(void);
void PolicySourceSendPRSwap(void);
void PolicySourceEvaluatePRSwap(void);
void PolicySourceWaitNewCapabilities(void);
void PolicySourceEvaluateVCONNSwap(void);
#endif // FSC_HAVE_SRC

#ifdef FSC_HAVE_SNK
void PolicySinkSendHardReset(void);
void PolicySinkSoftReset(void);
void PolicySinkSendSoftReset(void);
void PolicySinkTransitionDefault(void);
void PolicySinkStartup(void);
void PolicySinkDiscovery(void);
void PolicySinkWaitCaps(void);
void PolicySinkEvaluateCaps(void);
void PolicySinkSelectCapability(void);
void PolicySinkTransitionSink(void);
void PolicySinkReady(void);
void PolicySinkGiveSinkCap(void);
void PolicySinkGetSinkCap(void);
void PolicySinkGiveSourceCap(void);
void PolicySinkGetSourceCap(void);
void PolicySinkSendDRSwap(void);
void PolicySinkEvaluateDRSwap(void);
void PolicySinkEvaluateVCONNSwap(void);
void PolicySinkSendPRSwap(void);
void PolicySinkEvaluatePRSwap(void);
#endif

void PolicyInvalidState(void);
void policyBISTReceiveMode(void);
void policyBISTFrameReceived(void);
void policyBISTCarrierMode2(void);
void policyBISTTestData(void);

FSC_BOOL PolicySendHardReset(PolicyState_t nextState, FSC_U32 delay);
FSC_U8 PolicySendCommand(FSC_U8 Command, PolicyState_t nextState, FSC_U8 subIndex);
FSC_U8 PolicySendCommandNoReset(FSC_U8 Command, PolicyState_t nextState, FSC_U8 subIndex);
FSC_U8 PolicySendData(FSC_U8 MessageType, FSC_U8 NumDataObjects, doDataObject_t* DataObjects, PolicyState_t nextState, FSC_U8 subIndex, SopType sop);
FSC_U8 PolicySendDataNoReset(FSC_U8 MessageType, FSC_U8 NumDataObjects, doDataObject_t* DataObjects, PolicyState_t nextState, FSC_U8 subIndex);
void UpdateCapabilitiesRx(FSC_BOOL IsSourceCaps);

void processDMTBIST(void);

#ifdef FSC_HAVE_VDM
// shim functions for VDM code
void InitializeVdmManager(void);
void convertAndProcessVdmMessage(SopType sop);
void sendVdmMessage(SopType sop, FSC_U32 * arr, FSC_U32 length, PolicyState_t next_ps);
void doVdmCommand(void);
void doDiscoverIdentity(void);
void doDiscoverSvids(void);
void PolicyGiveVdm(void);
void PolicyVdm(void);
void autoVdmDiscovery(void);
#endif // FSC_HAVE_VDM

SopType TokenToSopType(FSC_U8 data);

#ifdef FSC_DEBUG
#if defined(FSC_HAVE_SRC) || (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
void WriteSourceCapabilities(FSC_U8* abytData);
void ReadSourceCapabilities(FSC_U8* abytData);
#endif // FSC_HAVE_SRC

#ifdef FSC_HAVE_SNK
void WriteSinkCapabilities(FSC_U8* abytData);
void WriteSinkRequestSettings(FSC_U8* abytData);
void ReadSinkRequestSettings(FSC_U8* abytData);

#endif // FSC_HAVE_SNK
void ReadSinkCapabilities(FSC_U8* abytData);


void DisableUSBPD(void);
FSC_BOOL GetPDStateLog(FSC_U8 * data);
void ProcessReadPDStateLog(FSC_U8* MsgBuffer, FSC_U8* retBuffer);
void ProcessPDBufferRead(FSC_U8* MsgBuffer, FSC_U8* retBuffer);

#endif // FSC_DEBUG

void EnableUSBPD(void);

void SetVbusTransitionTime(FSC_U32 time_ms);
#define MAX_VDM_LEN 7

#endif	/* _PDPOLICY_H_ */
void SetPDLimitVoltage(int vol);
void SetPolicyState(PolicyState_t state);
extern void pd_dpm_set_max_power(int max_power);

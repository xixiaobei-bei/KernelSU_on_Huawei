#include <linux/i2c.h>
#include "elan_ts.h"

#define ADCHLBTestType

#define PRINTRAWDATA
#define NormalADCSkipFrame 	0x2
#define NormalADCTestFrame	0x3
#define TestADCData		0xc0
#define TestADCData_2	0x40

#define ADCHB	-1
#define ADCLB	1000

#define TxDifferHB	13
#define TxEdgeDifferHB	38
#define TxEdgeTraceCount	1
#define TxOverHBPTRatio	10
#define SpecialTXDifferHB	0
#define SpecialTXOverHBPTRatio	0

#define RxDifferHB	25
#define RxEdgeDifferHB	38
#define RxEdgeTraceCount	1
#define RxOverHBPTRatio	12
#define SpecialRXTraceNumber	0
#define SpecialRXDifferHB	0
#define SpecialRXOverHBPTRatio	0

#define TXTraceTargetValue 8192
#define TXTraceAdjDifferHB_Cond1 28
#define TXTraceEdgeAdjDifferHB_Cond1 28
#define RXTraceAdjDifferHB_Cond1 28
#define RXTraceEdgeAdjDifferHB_Cond1	28
#define RXTraceTargetValue	8192
#define POLL_INT_COUNT	50

#define NoiseTestHB	60
#define NoiseTestHBFailPoint 3
#define NoiseTestLB -120
#define NoiseTestLBFailPoint	15
#define NoiseTestLimitHB	300
#define NoiseTestLimitLB	-400
#define NoiseSkipFrame	2
#define NoiseTestFrame	10
#define TIMEOUT	3
#define DelayTime	300

#define Alg2SetOptionBit		0xfebf
#define Alg3SetOptionBit		0xedad
#define Alg5SetOptionBit		0xffff
#define AlgSetOptionBit		0xecad

#define CMD_HEADER_REK 0x66
#define CMD_DATALEN	37
#define RECV_DATALEN	67
#define WRITE_CMD	0x54
#define READ_CMD	0x53
#define RAW_DATALEN_HIGHBYTE		11
#define RAW_DATALEN_LOWBYTE		12
#define OPTION_HIGHBYTE	6
#define OPTION_LOWBYTE	7
#define VAILD_DATA_LEN	60
#define GET_NORMAL_ADC	0x80
#define	GET_CHECK_ADC	0xc0

#define SEND_CMD_BYTE	7
#define SEND_OPTION_HBYTE 9
#define SEND_OPTION_LBYTE 10
#define PH3 0x15
#define NOISE_ADC_PRINT 0
#define SHORT_DATA_PRINT 1
#define RXTX_DIFF_PRINT 2


int disable_finger_report(void);
int elan_get_set_opcmd(void);
int elan_set_read_ph3(void);
void print_rawdata(const int rx,const int tx,int mode,void *data);
int elan_read_ph(void);
int elan_calibration_base(void);
int elan_noise_test(int rx,int tx);
int elan_get_rawdata(int datalen,bool checkData);
int get_noise_test_data(struct ts_rawdata_info* info);
int adc_mean_low_boundary_test(int datalen);
int enter_get_data_mode(int adc,int frameNum);
int get_normal_adc_data(bool checkData);
int elan_get_check_adc_data(bool btest);
int get_txrx_short_data(int data,int frame,bool checkData);
int elan_tx_open_check(void);
int elan_rx_open_test(void);
int elan_txrx_short_test(void);
int enter_short_test_mode(void);
int alloc_data_buf(void);
void free_data_buf(void);
void show_last_noise_data(struct ts_rawdata_info* info);
void show_normal_adc_data(struct ts_rawdata_info* info);
int elan_rx_open_check(struct ts_rawdata_info* info);
int elan_mean_value_check(char *result);
int elan_txrx_short_check(char *result);

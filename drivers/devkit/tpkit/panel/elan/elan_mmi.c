#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include "elan_mmi.h"

static short *testADCDataBuf=NULL;
static short *testADCDataBuf2=NULL;
static uint8_t *rawdatabuf=NULL;
static short *totlebuf=NULL;
extern struct elan_ktf_ts_data *elan_ts;

/****************HID over I2C send cmd format****************
	hid over i2c cmd 0-4 is head 0x04,0x00,0x23,0x00,0x03;
	6th send valid data len
	after 6th is valid cmd data sent to IC
************************************************************/

static int elan_poll_int(void)
{
	int i=0;
	int ret = 0;
	if (!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,elan_ts is NULL!\n",__func__);
		return -EINVAL;
	}
	for(i=0;i<POLL_INT_COUNT;i++)
	{
		ret = gpio_get_value(elan_ts->int_gpio);
		if (ret == 0) {
			TS_LOG_INFO("[elan]:int is low!count=%d\n",i);
			return NO_ERR;
		} else {
			msleep(5);//IC need
		}
	}
	TS_LOG_INFO("[elan]:int is high!\n");
	return -EINVAL;
}

int alloc_data_buf(void)
{
	int rx=0;
	int tx=0;
	if ((!elan_ts)||(elan_ts->rx_num<=0)||(elan_ts->tx_num<=0))
	{
		TS_LOG_ERR("[elan]:%s,elan_ts is NULL or tx rx num is invalid!\n",__func__);
		return -EINVAL;
	}
	rx=elan_ts->rx_num;
	tx=elan_ts->tx_num;
	rawdatabuf=(uint8_t *)kzalloc(sizeof(uint8_t)*(tx*rx*2), GFP_KERNEL);
	if(rawdatabuf==NULL)
	{
		TS_LOG_ERR("[elan]:%s,alloc mem for rawdata buf fail!\n",__func__);
		return -ENOMEM;
	}
	totlebuf=(short *)kzalloc(sizeof(short)*(tx*rx), GFP_KERNEL);
	if(totlebuf==NULL)
	{
		TS_LOG_ERR("[elan]:%s,alloc mem for totlebuf fail!\n",__func__);
		return -ENOMEM;
	}

	return NO_ERR;
}

void free_data_buf(void)
{
	if(rawdatabuf)
	{
		kfree(rawdatabuf);
		rawdatabuf=NULL;
	}
	if(totlebuf)
	{
		kfree(totlebuf);
		totlebuf=NULL;
	}
	if(testADCDataBuf)
	{
		kfree(testADCDataBuf);
		testADCDataBuf=NULL;
	}
	if(testADCDataBuf2)
	{
		kfree(testADCDataBuf2);
		testADCDataBuf2=NULL;
	}
	return;
}

static int elan_ts_get_data_by_cmd(uint8_t *cmd, uint8_t *buf, size_t w_size,  size_t r_size)
{
	int ret=0;
	if(buf == NULL||cmd==NULL)
	{
		TS_LOG_ERR("[elan]:write and read cmd buf NULL!\n");
		return -EINVAL;
	}
	ret = elan_i2c_write(cmd,w_size);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,write cmd fail!ret=%d\n",__func__,ret);
		return -EINVAL;
	}

	ret=elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s-->elan wait Int low timeout!<--\n",__func__);
		return -EINVAL;
	}
	ret = elan_i2c_read(NULL,0,buf,r_size);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,read data fail!ret=%d\n",__func__,ret);
		return -EINVAL;
	}
	return NO_ERR;
}

int disable_finger_report(void)
{
	int ret = 0;
	uint8_t disable_finger_cmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0xCA,0x00,0x01};
	ret = elan_i2c_write(disable_finger_cmd, sizeof(disable_finger_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm1 cmd fail ret=%d!\n",ret);
	}
	return ret;
}

/*Disable Algorithm Func*/
int elan_get_set_opcmd(void)
{
	int len=0;
	uint8_t buf_recv[RECV_DATALEN]={0};
	uint8_t get_option_cmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x53,0xB1,0x00,0x01};
	uint8_t get_option_cmdt[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x53,0xC1,0x00,0x01};

	len=elan_ts_get_data_by_cmd(get_option_cmd, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send or send op cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
			buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],buf_recv[4],buf_recv[5],buf_recv[6],buf_recv[7],buf_recv[8],buf_recv[9]);//operation databuf
	}

	get_option_cmd[SEND_CMD_BYTE]=WRITE_CMD;
	get_option_cmd[SEND_OPTION_HBYTE]=(((buf_recv[OPTION_HIGHBYTE]<<8)|buf_recv[OPTION_LOWBYTE])&Alg2SetOptionBit)>>8;
	get_option_cmd[SEND_OPTION_LBYTE]=(((buf_recv[OPTION_HIGHBYTE]<<8)|buf_recv[OPTION_LOWBYTE])&Alg2SetOptionBit)&0xff;

	len = elan_i2c_write(get_option_cmd , sizeof(get_option_cmd));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm2 cmd fail,len=%d!\n",len);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]i2c send Disable Algorithm2 cmd suceed!\n");
	}

	len = elan_ts_get_data_by_cmd(get_option_cmdt, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send or send op cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
			buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],buf_recv[4],buf_recv[5],buf_recv[6],buf_recv[7],buf_recv[8],buf_recv[9]);
	}

	get_option_cmdt[SEND_CMD_BYTE]=WRITE_CMD;
	get_option_cmdt[SEND_OPTION_HBYTE]=(((buf_recv[OPTION_HIGHBYTE]<<8)|buf_recv[OPTION_LOWBYTE])&Alg3SetOptionBit)>>8;
	get_option_cmdt[SEND_OPTION_LBYTE]=(((buf_recv[OPTION_HIGHBYTE]<<8)|buf_recv[OPTION_LOWBYTE])&Alg3SetOptionBit)&0xff;

	len = elan_i2c_write(get_option_cmdt , sizeof(get_option_cmdt));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send Disable Algorithm3 cmd fail,len=%d!\n",len);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:i2c send Disable Algorithm3 cmd suceed!\n");
	}
	msleep(20); //mmi test need
	return NO_ERR;
}

/*set capacitance charger time*/
int elan_set_read_ph3(void)
{
	int len=0;
	uint8_t setTPParameterCmd[CMD_DATALEN]={0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0x2d,0x00,0x01};
	uint8_t ph3CmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0xC7,0x00,PH3};
	/* 0th-5th is head,6th send byte len,7th read or write cmd*/
	uint8_t res_buf[RECV_DATALEN]={0};
	len=elan_i2c_write(ph3CmdBuf , sizeof(ph3CmdBuf));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send ph3CmdBuf cmd fail!,len=%d\n",len);
		return -EINVAL;
	}
	len = elan_i2c_write(setTPParameterCmd , sizeof(setTPParameterCmd));
	if (len) {
		TS_LOG_ERR("[elan]:i2c send setTPParameter cmd fail!len=%d\n",len);
		return -EINVAL;
	}
	msleep(DelayTime); //mmi test need
	ph3CmdBuf[7]=READ_CMD; //this 7th is write or read byte
	len = elan_i2c_read(ph3CmdBuf,sizeof(ph3CmdBuf),res_buf,sizeof(res_buf));
	if (len||(res_buf[7]!=PH3)) {				//7th is read ph3 byte
		TS_LOG_ERR("[elan]:read ph3 fail!len=%d\n",len);
		return -EINVAL;
	}
	return NO_ERR;
}

void print_rawdata(const int rx,const int tx,int mode,void *data)
{
	int i=0,j=0;
	short (*databuf)[rx]=NULL;
	switch(mode)
	{
		case NOISE_ADC_PRINT:	//noise adc data print
			for(i=0;i<tx;i++)
			{
				printk("[elan]%02d:",i+1);
				for(j=0;j<rx;j++)
				{
					printk("%04d ",totlebuf[j+i*rx]);
					if(j==rx-1)
					{
						printk("\n");
					}
				}
			}
			break;
		case SHORT_DATA_PRINT:	//short data print
			printk("[elan]:Rx:");
			for(i=0;i<(rx+tx);i++)
			{
				printk("%04d ",totlebuf[i]);
				if(i==rx-1)
				{
					printk("\n");
					printk("[elan]:Tx:");
				}
			}
			printk("\n");
			break;
		case RXTX_DIFF_PRINT:	//tx rx open diff data print
			databuf=(short (*)[rx])data;
			if(databuf==NULL)
			{
				TS_LOG_ERR("[elan]:%s,databuf is null\n",__func__);
				return;
			}
			for(i=0;i<tx;i++)
			{
				printk("[elan]%02d:",i+1);
				for(j=0;j<rx;j++)
				{
					for(j=0;j<rx;j++)
					{
						printk("%04d ",abs(databuf[i][j]));
						if(j==rx-1)
							printk("\n");
					}
				}
			}
			break;
		default:
			TS_LOG_DEBUG("[elan]:unknow print rwadata mode!\n");
	}
	return;
}

/*read capacitance charger arg*/
int elan_read_ph(void)
{
	int len=0;
	uint8_t buf_recv[RECV_DATALEN]={0};
	uint8_t ph1CmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x53,0xC5,0x00,0x01};
	uint8_t ph2CmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x53,0xC6,0x00,0x01};
	uint8_t ph3CmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x53,0xC7,0x00,0x01};

	len=elan_ts_get_data_by_cmd(ph1CmdBuf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph1 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph1:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
			buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],buf_recv[4],buf_recv[5],buf_recv[6],buf_recv[7]); //0-3th hid head data 4-7 ph1 data 
	}

	len=elan_ts_get_data_by_cmd(ph2CmdBuf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph2 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph2:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
			buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],buf_recv[4],buf_recv[5],buf_recv[6],buf_recv[7]); //0-3th hid head data 4-7 ph2 data 
	}

	len = elan_ts_get_data_by_cmd(ph3CmdBuf, buf_recv, CMD_DATALEN, RECV_DATALEN);
	if (len) {
		TS_LOG_ERR("[elan]:i2c send read ph3 cmd fail!\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ph3:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", \
			buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],buf_recv[4],buf_recv[5],buf_recv[6],buf_recv[7]);//0-3th hid head data 4-7 ph3 data 
	}
	return NO_ERR;
}

int elan_calibration_base(void)
{
	int ret=0;
	uint8_t buff[RECV_DATALEN]={0};
	uint8_t rsp_buf[4]={CMD_HEADER_REK,CMD_HEADER_REK,CMD_HEADER_REK,CMD_HEADER_REK};
	uint8_t CmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0xC0,0xE1,0x5A};//flash_key
	uint8_t RekCmdBuf[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0x29,0x00,0x01};

	ret = elan_i2c_write(CmdBuf,sizeof(CmdBuf));
	if (ret) {
		TS_LOG_ERR("elanRekTp send cmd fail!ret=%d\n",ret);
		return -EINVAL;
	}

	ret = elan_ts_get_data_by_cmd(RekCmdBuf,buff,CMD_DATALEN,RECV_DATALEN);
	if (ret) {
		TS_LOG_ERR("elanRekTp elan_ts_get_data_by_cmd fail!\n");
		return -EINVAL;
	}
	ret = memcmp(buff+4,rsp_buf,sizeof(rsp_buf));//rsp_buf 4th-7th is rek data(right 0x66)
	if (ret) {
		TS_LOG_ERR("[elan]:unexpected calibration response\n");
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:calibration succeed!\n");
	}
	msleep(DelayTime);	//mmi test need
	return NO_ERR;
}

static int enter_test_mode(void){
	int ret=0;
	uint8_t enterTestModeCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x55,0x55,0x55,0x55};
	ret=elan_i2c_write(enterTestModeCmd , sizeof(enterTestModeCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send enter test mode cmd fail!ret=%d\n",ret);
	} else {
		TS_LOG_INFO("[elan]:enter test mode succeed!\n");
	}
	return ret;
}

static int exit_test_mode(void){
	int ret=0;
	uint8_t exitTestModeCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0xa5,0xa5,0xa5,0xa5};
	ret=elan_i2c_write(exitTestModeCmd , sizeof(exitTestModeCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c send exit test mode cmd fail!ret=%d\n",ret);
	} else {
		TS_LOG_INFO("[elan]:exit test mode succeed!\n");
	}
	return ret;
}

int elan_noise_test(int rx,int tx)
{
	int i=0,j=0;
	int OverHB=0,UnderLB=0;
	short (*dv)[rx]=(short (*)[rx])totlebuf;
	if (totlebuf==NULL) {
		TS_LOG_ERR("[elan]:%s,totlebuf or info is null\n",__func__);
		return -EINVAL;
	}
	for(i=0;i<tx;i++)
	{
		for(j=0;j<rx;j++)
		{
			if (dv[i][j]>NoiseTestLimitHB||dv[i][j]<NoiseTestLimitLB) {
				TS_LOG_ERR("[elan]:noise %d,%d,%d\n",i,j,dv[i][j]);
				return -EINVAL;
			} else if (dv[i][j]>NoiseTestHB) {
				OverHB++;
				TS_LOG_INFO("[elan]:noise %d,%d,%d\n",i,j,dv[i][j]);
			} else if (dv[i][j]<NoiseTestLB) {
				UnderLB++;
				TS_LOG_INFO("[elan]:noise %d,%d,%d\n",i,j,dv[i][j]);
			}
		}
	}

	if (OverHB>NoiseTestHBFailPoint||UnderLB>NoiseTestLBFailPoint) {
		TS_LOG_ERR("[elan]:Noise Over HB or LB Fail,Fail HBnum=%d,LBnum=%d\n",OverHB,UnderLB);
		return -EINVAL;
	}
	return NO_ERR;
}

int elan_get_rawdata(int datalen,bool checkData)
{
	int i=0,j=0;
	int ret = 0;
	int recv_num = 0;
	uint8_t databuf[RECV_DATALEN]={0};
	recv_num = datalen/VAILD_DATA_LEN+((datalen%VAILD_DATA_LEN)!= 0);//receive data times
	for(i = 0;i < recv_num;i++)
	{
		ret = elan_i2c_read(NULL,0,databuf,sizeof(databuf));
		if (ret) {
			TS_LOG_ERR("[elan]:%s,i2c_read fail!ret=%d\n",__func__,ret);
			return -EINVAL;
		}
		if(checkData){
			if ((i==(recv_num-1))&&((datalen%VAILD_DATA_LEN)!= 0)){
				memcpy(rawdatabuf+VAILD_DATA_LEN*i,databuf+7,datalen%VAILD_DATA_LEN);   //0-6 head data
			} else {
				memcpy(rawdatabuf+VAILD_DATA_LEN*i,databuf+7,VAILD_DATA_LEN); //0-6 head data
			}
		}
	}
	if(checkData){
		for(i=0;i<datalen;i=i+2)
		{
			totlebuf[j]=(rawdatabuf[i]<<8)|rawdatabuf[i+1];
			j++;
		}
	}
	return NO_ERR;
}

void show_last_noise_data(struct ts_rawdata_info* info)
{
	int i = 0;
	int tx=0;
	int rx=0;
	if(!info||!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,info or elan_ts is null\n",__func__);
		return;
	}

	tx=elan_ts->tx_num;
	rx=elan_ts->rx_num;
	for(i=0;i<tx*rx;i++)
	{
		info->buff[2+tx*rx+i]=(int)totlebuf[i];
	}
	info->used_size=tx*rx+2;
}

int get_noise_test_data(struct ts_rawdata_info* info)
{
	int ret=0,i=0;
	int tx=0,rx=0;
	int datalength=0;
	uint8_t GetDvdataCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x06,0x58,0x04,0x00,0x00,0x00,0x00};

	if(!info||!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,info or elan_ts is null\n",__func__);
		return -EINVAL;
	}

	tx=elan_ts->tx_num;
	rx=elan_ts->rx_num;
	datalength=tx*rx*2;

	GetDvdataCmd[RAW_DATALEN_HIGHBYTE]=(datalength & 0xff00) >> 8;
	GetDvdataCmd[RAW_DATALEN_LOWBYTE]=datalength&0x00ff;

	for(i=0;i<NoiseSkipFrame+NoiseTestFrame;i++)
	{
		ret=enter_test_mode();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,can't enter test mode!\n",__func__);
			return -EINVAL;
		}
		ret=elan_i2c_write(GetDvdataCmd,sizeof(GetDvdataCmd));
		if (ret) {
			TS_LOG_ERR("[elan]:%s,send get dv data cmd fail!ret=%d\n",__func__,ret);
			return -EINVAL;
		}
		ret = elan_poll_int();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,poll int fail\n",__func__);
			return -EINVAL;
		}

		if (i >= NoiseSkipFrame){
			ret=elan_get_rawdata(datalength,true);
			if (ret) {
				TS_LOG_ERR("[elan]:%s,get check data  fail\n",__func__);
				return -EINVAL;
			}
		}else{
			ret=elan_get_rawdata(datalength,false);
			if (ret) {
				TS_LOG_ERR("[elan]:%s,get skip data  fail\n",__func__);
				return -EINVAL;
			}
		}

		ret=exit_test_mode();
		if (ret) {
			TS_LOG_ERR("[elan]:%s,can't exit test mode!\n",__func__);
			return -EINVAL;
		}
		if (i >= NoiseSkipFrame) {
			ret = elan_noise_test(rx,tx);
			if (ret) {
				TS_LOG_ERR("[elan]:noise test fail and exit!\n");
				goto NoiseTestOut;
			}
		}
	}
NoiseTestOut:
	show_last_noise_data(info);
#ifdef PRINTRAWDATA
	TS_LOG_INFO("------->>elan Get Noise Data Start<<-------\n");
	print_rawdata(rx,tx,NOISE_ADC_PRINT,NULL);
#endif
	return ret;
}

int adc_mean_low_boundary_test(int datalen)
{
	int i=0,meauAdc=0,totleAdc=0;
	for(i=0;i<datalen;i++)
	{
		totleAdc=totleAdc+totlebuf[i];
	}
	if (datalen>0) {
		meauAdc=totleAdc/datalen;
		TS_LOG_INFO("[elan]:meau adc=%d\n",meauAdc);
		if(meauAdc<ADCLB){
			TS_LOG_ERR("[elan]:meauAdc<%d\n",ADCLB);
			return -EINVAL;
		}
	} else {
		TS_LOG_ERR("[elan]:data len is no right!\n");
		return -EINVAL;
	}
	return NO_ERR;
}

int enter_get_data_mode(int adc,int frameNum)
{
	int ret=0;
	uint8_t buf_recv[RECV_DATALEN]={0};
	uint8_t EnterGetDataModeCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0xcd,adc,frameNum};
	uint8_t ReadyAck[4]={0x42,0x41,0x53,0x45};
	ret=elan_ts_get_data_by_cmd(EnterGetDataModeCmd, buf_recv,CMD_DATALEN,RECV_DATALEN);
	if (ret) {
		TS_LOG_ERR("[elan]:send get data mode cmd fail!\n");
		return -EINVAL;
	}
	ret = memcmp(buf_recv+4,ReadyAck,sizeof(ReadyAck));//buf_recv 4th-7th is ack data
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get ACK fail!\n",__func__);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:%s,get ACK succeed\n",__func__);
	}
	return NO_ERR;
}

/*get normal ADC rawdata*/
int get_normal_adc_data(bool checkData)  //checkData true is test ,false is skipframe
{
	int ret=0;
	int tx=0,rx=0;
	int datalength=0;
	uint8_t DataModeCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x06,0x58,0x02,0x00,0x00,0x00,0x00};

	if(!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,elan_ts is null\n",__func__);
		return -EINVAL;
	}

	tx=elan_ts->tx_num;
	rx=elan_ts->rx_num;
	datalength=tx*rx*2;
	DataModeCmd[RAW_DATALEN_HIGHBYTE]=(datalength & 0xff00) >> 8;
	DataModeCmd[RAW_DATALEN_LOWBYTE]=datalength&0x00ff;

	ret=enter_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't enter test mode!\n",__func__);
		return -EINVAL;
	}
	ret = elan_i2c_write(DataModeCmd, sizeof(DataModeCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send data mode cmd fail!ret=%d\n",ret);
		return -EINVAL;
	}
	ret = elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,poll int fail\n",__func__);
		return -EINVAL;
	}
	ret=elan_get_rawdata(datalength,checkData);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get normal adc rawdata fail!\n",__func__);
		return -EINVAL;
	}
	ret=exit_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't exit test mode!\n",__func__);
		return -EINVAL;
	}
#ifdef PRINTRAWDATA
	if(checkData){
		TS_LOG_INFO("---->elan: Get Normal ADC Data_[2] Start<----\n");
		print_rawdata(rx,tx,NOISE_ADC_PRINT,NULL);
	}
#endif
	return NO_ERR;
}

int enter_short_test_mode(void)
{
	int ret=0;
	uint8_t ShortTestCmd[CMD_DATALEN]={0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0x33,0x10,0x00};

	ret=elan_i2c_write(ShortTestCmd, sizeof(ShortTestCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send open short cmd fail!ret=%d\n",ret);
		return ret;
	} else {
		TS_LOG_INFO("[elan]:send open short cmd succeed!\n");
	}
	return ret;
}

/*get txrx short data*/
int get_txrx_short_data(int data,int frame,bool checkData)
{
	int ret=0;
	int tx=0,rx=0;
	int datalength=0;
	uint8_t EnterGetDataModeCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x04,0x54,0x34,0x80,data|frame};
	uint8_t GetShortCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x06,0x58,0x01,0x00,0x00,0x00,0x00};
	uint8_t DummyCmd[CMD_DATALEN] = {0x04,0x00,0x23,0x00,0x03,0x00,0x06,0x58,0xc2,0x00,0x00,0x00,0x00};

	if(!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,elan_ts is null\n",__func__);
		return -EINVAL;
	}

	tx=elan_ts->tx_num;
	rx=elan_ts->rx_num;
	datalength=tx*rx*2;

	GetShortCmd[RAW_DATALEN_HIGHBYTE]=(datalength & 0xff00) >> 8;
	GetShortCmd[RAW_DATALEN_LOWBYTE]=datalength&0x00ff;

	ret = elan_i2c_write(EnterGetDataModeCmd, sizeof(EnterGetDataModeCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send enter get data mode Cmd fail!ret=%d\n",ret);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:send get data cmd:%02x,%02x,%02x,%02x succeed\n",EnterGetDataModeCmd[7], \
			EnterGetDataModeCmd[8],EnterGetDataModeCmd[9],EnterGetDataModeCmd[10]);
	}

	ret=enter_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't enter test mode!\n",__func__);
		return -EINVAL;
	}

	ret = elan_i2c_write(GetShortCmd, sizeof(GetShortCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send get open short cmd fail!ret=%d\n",ret);
		return -EINVAL;
	}
	ret = elan_poll_int();
	if (ret) {
		TS_LOG_ERR("[elan]:%s poll int fail!\n",__func__);
		return -EINVAL;
	}
	ret=elan_get_rawdata(datalength,checkData);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get tx rx short rawdata fail!\n",__func__);
		return -EINVAL;
	}
	ret=elan_i2c_write(DummyCmd, sizeof(DummyCmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send dummy cmd fail!ret=%d\n",ret);
		return -EINVAL;
	}
	ret=exit_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,can't exit test mode!\n",__func__);
		return -EINVAL;
	}

	if(checkData)
	{
		if(data==TestADCData)
		{
			testADCDataBuf=(short *)kzalloc(sizeof(short)*(tx+rx),GFP_KERNEL);
			if(testADCDataBuf==NULL)
			{
				TS_LOG_ERR("[elan]:testADCDataBuf is NULL\n");
				return -ENOMEM;
			}
			memcpy(testADCDataBuf,totlebuf,sizeof(short)*(tx+rx));
		}
		else if(data==TestADCData_2)
		{
			testADCDataBuf2=(short *)kzalloc(sizeof(short)*(tx+rx),GFP_KERNEL);
			if(testADCDataBuf2==NULL)
			{
				TS_LOG_ERR("[elan]:testADCDataBuf2 is NULL\n");
				return -ENOMEM;
			}
			memcpy(testADCDataBuf2,totlebuf,sizeof(short)*(tx+rx));
		}
#ifdef PRINTRAWDATA
		TS_LOG_INFO("--->elan: Check Open Short Data[2] Start<---\n");
		print_rawdata(rx,tx,SHORT_DATA_PRINT,NULL);
#endif
	}
	return NO_ERR;
}

int elan_tx_open_check(void)
{
	int i=0,j=0;
	int overHb=0;
	int edgeoverHb=0;
	int specialoverHb=0;
	int ret=NO_ERR;
	const int rx=elan_ts->rx_num,tx=elan_ts->tx_num;
	short (*databuf)[rx]=(short (*)[rx])totlebuf;
	short (*txDiffData)[rx]=(short (*)[rx])kzalloc(sizeof(short)*(tx-1)*rx,GFP_KERNEL);
	if(!txDiffData)
	{
		TS_LOG_ERR("[elan]:%s txDiffData buf is null!\n",__func__);
		return -EINVAL;
	}
	if(!totlebuf)
	{
		TS_LOG_ERR("[elan]:%s data buf is null!\n",__func__);
		kfree(txDiffData);
		txDiffData=NULL;
		return -EINVAL;
	}

	for(i=0;i<tx-1;i++)
	{
		for(j=0;j<rx;j++)
		{
			if(databuf[i][j]<0||databuf[i+1][j]<0)
			{
				TS_LOG_ERR("[elan]:%s,databuf[%d][%d]=%d,databuf[%d][%d]=%d\n", \
					__func__,i,j,databuf[i][j],i+1,j,databuf[i+1][j]);
				ret=-EINVAL;
				goto tx_open_test_exit;
			}
			txDiffData[i][j]=databuf[i][j]-databuf[i+1][j];
			if(!((j>=TxEdgeTraceCount)&&(j<=rx-TxEdgeTraceCount)&&(i>=TxEdgeTraceCount)&&(i<=tx-1-TxEdgeTraceCount)))
			{
				if(abs(txDiffData[i][j])>(databuf[i][j]*TxEdgeDifferHB/100))
				{
					TS_LOG_INFO("[overHbelan]:Edge====i=%d,j=%d,txDiffData=%d\n",i,j,txDiffData[i][j]);
					edgeoverHb++;
				}
			}else{
				if(abs(txDiffData[i][j])>(databuf[i][j]*TxDifferHB/100))
				{
					TS_LOG_INFO("[overHbelan]:====i=%d,j=%d,txDiffData=%d\n",i,j,txDiffData[i][j]);
					overHb++;
				}
			}
			if((SpecialTXDifferHB>0)&&(abs(txDiffData[i][j])>totlebuf[j+i*rx]*SpecialTXDifferHB/100))
			{
				specialoverHb++;
				TS_LOG_INFO("[elan]:specialoverHb=%d\n",specialoverHb);
			}
		}
	}
#ifdef PRINTRAWDATA
		TS_LOG_INFO("---->[elan] TX Diff Data<----\n");
		print_rawdata(rx,tx-1,RXTX_DIFF_PRINT,txDiffData);
#endif

	TS_LOG_INFO("[elan]:TxDiffData edgeoverHb=%d,overHb=%d,specialoverHb=%d\n",edgeoverHb,overHb,specialoverHb);
	if((edgeoverHb>(rx*TxOverHBPTRatio/100))||((SpecialTXDifferHB>0)&&(specialoverHb>rx*SpecialTXOverHBPTRatio/100))||(overHb>rx*TxOverHBPTRatio/100))
	{
		TS_LOG_ERR("[elan]:edgeoverHb is over spec!\n");
		ret=-EINVAL;
		goto tx_open_test_exit;
	}

tx_open_test_exit:
	if(txDiffData)
	{
		kfree(txDiffData);
		txDiffData=NULL;
	}
	return ret;
}

void show_normal_adc_data(struct ts_rawdata_info* info)
{
	int i = 0;
	int rx=0,tx=0;
	if((!info)||(!elan_ts))
	{
		TS_LOG_ERR("[elan]:%s,info is null\n",__func__);
		return;
	}
	rx=elan_ts->rx_num;
	tx=elan_ts->tx_num;
	for(i=0;i<tx*rx;i++)
	{
		info->buff[2+i]=(int)totlebuf[i];
	}
	info->used_size=tx*rx*2+2;   //platform rx tx is 0-1
}

int elan_rx_open_test(void)
{
	int i=0,j=0;
	int overHb=0;
	int edgeoverHb=0;
	int specialoverHb=0;
	int ret=NO_ERR;
	int rx=elan_ts->rx_num,tx=elan_ts->tx_num;
	short (*databuf)[rx]=(short (*)[rx])totlebuf;
	short (*rxDiffData)[rx-1]=(short (*)[rx-1])kzalloc(sizeof(short)*tx*(rx-1),GFP_KERNEL);

	if(!rxDiffData)
	{
		TS_LOG_ERR("[elan]:alloc rxDiffData buf fail\n");
		return -EINVAL;
	}
	if(!totlebuf)
	{
		TS_LOG_ERR("[elan]:%s,data buf is null\n",__func__);
		kfree(rxDiffData);
		rxDiffData=NULL;
		return -EINVAL;
	}

	for(i=0;i<tx;i++)
	{
		for(j=0;j<rx-1;j++)
		{
			if(databuf[i][j]<0||databuf[i][j+1]<0)
			{
				TS_LOG_ERR("[elan]:%s,databuf[%d][%d]=%d,databuf[%d][%d]=%d\n", \
					__func__,i,j,databuf[i][j],i+1,j,databuf[i+1][j]);
				ret=-EINVAL;
				goto rx_open_test_exit;
			}
			rxDiffData[i][j]=abs(databuf[i][j]-databuf[i][j+1]);
			if(!((j>=RxEdgeTraceCount)&&(j<=rx-1-RxEdgeTraceCount)&&(i>=RxEdgeTraceCount)&&(i<=tx-RxEdgeTraceCount)))
			{
				if(rxDiffData[i][j]>(databuf[i][j]*RxEdgeDifferHB/100))
				{
					TS_LOG_INFO("[overHbelan]:i=%d,j=%d\n",i,j);
					edgeoverHb++;
				}
			}else{
				if(rxDiffData[i][j]>(databuf[i][j]*RxDifferHB/100))
				{
					TS_LOG_INFO("[overHbelan]:i=%d,j=%d\n",i,j);
					overHb++;
				}
			}
			if((SpecialRXDifferHB>0)&&(rxDiffData[i][j]>databuf[i][j]*SpecialRXDifferHB/100))
			{
				specialoverHb++;
			}
		}
	}
#ifdef PRINTRAWDATA
	TS_LOG_INFO("----->[elan] RX Diff Data<----\n");
	print_rawdata(rx-1,tx,RXTX_DIFF_PRINT,rxDiffData);
#endif

	TS_LOG_INFO("[elan]:RxDiffData edgeoverHb=%d,overHb=%d\n",edgeoverHb,overHb);
	if((edgeoverHb>tx*RxOverHBPTRatio/100)||((SpecialRXDifferHB>0)&&(specialoverHb>tx*SpecialRXOverHBPTRatio/100))||(overHb>tx*RxOverHBPTRatio/100))
	{
		TS_LOG_ERR("[elan]:edgeoverHb is over spec!\n");
		ret=-EINVAL;
	}

rx_open_test_exit:
	if(rxDiffData)
	{
		kfree(rxDiffData);
		rxDiffData=NULL;
	}
	return ret;
}

int elan_txrx_short_test(void)
{
	int i=0,ret=0;
	int overHB=0;
	bool txrx_short_test_result=true;
	int rx=0,tx=0;
	short *differData=testADCDataBuf;
	TS_LOG_INFO("[elan]:Rx diff:");
	if(!testADCDataBuf||!testADCDataBuf2||!elan_ts)
	{
		TS_LOG_ERR("[elan]:%s,data buf is null\n",__func__);
		return -EINVAL;
	}
	rx=elan_ts->rx_num;
	tx=elan_ts->tx_num;
	for(i=0;i<(rx+tx);i++)
	{
		if(testADCDataBuf[i]==0||testADCDataBuf2[i]==0){
			txrx_short_test_result=false;
			TS_LOG_ERR("[elan]:testADCDataBuf[%d]=%d,testADCDataBuf2[%d]=%d",i,testADCDataBuf[i],i,testADCDataBuf2[i]);
		}
		differData[i]=abs(testADCDataBuf[i]-testADCDataBuf2[i]);
		printk("%03d ",differData[i]);
		if(i==rx-1)
		{
			TS_LOG_INFO("\n");
			TS_LOG_INFO("[elan]:Tx diff:");
		}
	}

	TS_LOG_INFO("[elan]:RX Short Test Adjacent Differ Data:");
	for(i=0;i<(rx-1);i++)
	{
		differData[i]=abs(differData[i]-differData[i+1]);
		printk("%d ",differData[i]);
		if((i>0)&&(i<rx-2)){
			if(differData[i]>(TXTraceTargetValue*TXTraceAdjDifferHB_Cond1/100))//100 is percent
			{
				overHB++;
				TS_LOG_ERR("[elan]:%s,overHB=%d\n",__func__,overHB);
			}
		}else{
			if(differData[i]>(TXTraceTargetValue*TXTraceEdgeAdjDifferHB_Cond1/100))//100 is percent
			{
				overHB++;
				TS_LOG_ERR("[elan]:%s,overHB=%d\n",__func__,overHB);
			}
		}
	}

	if(overHB>0)
	{
		TS_LOG_ERR("[elan]:RX Short Test Fail!\n");
		txrx_short_test_result=false;
	}
	else{
		TS_LOG_INFO("[elan]:RX Short Test Pass!\n");
	}

	overHB=0;
	TS_LOG_INFO("[elan]:TX Short Test Adjacent Differ Data:");
	for(i=rx;i<(rx+tx-1);i++)
	{
		differData[i]=abs(differData[i]-differData[i+1]);
		printk("%d ",differData[i]);
		if((i>rx)&&(i<rx+tx-2)){
			if((differData[i]>RXTraceTargetValue*RXTraceAdjDifferHB_Cond1/100))
			{
				overHB++;
			}
		}else{
			if(differData[i]>RXTraceTargetValue*RXTraceEdgeAdjDifferHB_Cond1/100)
			{
				overHB++;
			}
		}
	}

	if (overHB > 0) {
		txrx_short_test_result=false;
	}

	if (!txrx_short_test_result) {
		TS_LOG_ERR("[elan]:TX Short Test Fail!\n");
		ret=-EINVAL;
	}
	else{
		TS_LOG_INFO("[elan]:TX Short Test Pass!\n");
		ret=NO_ERR;
	}
	return ret;
}

/*get tx rx short data*/
static int elan_txrx_short(void){
	int ret=0;
	ret=get_txrx_short_data(TestADCData,NormalADCSkipFrame,false);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx skip frame data1 fail!\n",__func__);
		return -EINVAL;
	}
	ret=get_txrx_short_data(TestADCData,NormalADCTestFrame,true);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx test frame data1 fail!\n",__func__);
		return -EINVAL;
	}

	ret=get_txrx_short_data(TestADCData_2,NormalADCSkipFrame,false);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx skip frame data2 fail!\n",__func__);
		return -EINVAL;
	}
	ret=get_txrx_short_data(TestADCData_2,NormalADCTestFrame,true);
	if (ret) {
		TS_LOG_ERR("[elan]:%s,get short txrx test frame data2 fail!\n",__func__);
		return -EINVAL;
	}
	return NO_ERR;
}

static int normal_adc_get_data(void)
{
	int ret = 0;
	ret = enter_get_data_mode(GET_NORMAL_ADC,NormalADCSkipFrame);
	if (ret) {
		TS_LOG_ERR("[elan]:enter skip frame adc get data mode fail\n");
		return -EINVAL;
	}
	ret = get_normal_adc_data(false);
	if (ret) {
		TS_LOG_ERR("[elan]:get ship frame normal adc fail\n");
		return -EINVAL;
	}
	ret = enter_get_data_mode(GET_NORMAL_ADC,NormalADCTestFrame);
	if (ret) {
		TS_LOG_ERR("[elan]:enter test frame adc get data mode fail\n");
		return -EINVAL;
	}
	ret = get_normal_adc_data(true);
	if (ret) {
		TS_LOG_ERR("[elan]:get test frame normal adc fail\n");
		return -EINVAL;
	}
	return NO_ERR;
}

int elan_rx_open_check(struct ts_rawdata_info* info)
{
	int ret=0;
	if(!info)
	{
		TS_LOG_ERR("[elan]:%s,info is null\n",__func__);
		return -EINVAL;
	}
	ret = normal_adc_get_data();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,normal adc get data fail!\n",__func__);
		return -EINVAL;
	}
	if(info)
	{
		show_normal_adc_data(info);
	}
	ret = elan_rx_open_test();
	if (ret) {
		TS_LOG_ERR("[elan]:rx open test fail!\n");
		return 1;
	} else {
		TS_LOG_INFO("[elan]:rx open test pass!\n");
	}
	return NO_ERR;
}

int elan_mean_value_check(char *result){
	int ret=0;
	ret = elan_set_read_ph3();
	if (ret) {
		TS_LOG_ERR("[elan]:set ph3 fail\n");
		return -EINVAL;
	}

	ret = normal_adc_get_data();
	if (ret) {
		TS_LOG_ERR("[elan]:get check data fail\n");
		return -EINVAL;
	}
	ret = adc_mean_low_boundary_test(elan_ts->tx_num*elan_ts->rx_num);
	if (ret) {
		TS_LOG_ERR("[elan]:ADC Mean Low Boundary Test Fail\n");
		strncat(result,"1F-",strlen("1F-"));
	} else {
		TS_LOG_INFO("[elan]:ADC Mean Low Boundary Test Pass\n");
		strncat(result,"1P-",strlen("1P-"));
	}
	return NO_ERR;
}

int elan_txrx_short_check(char *result){
	int ret=0;
	ret = enter_short_test_mode();
	if (ret) {
		TS_LOG_ERR("[elan]:%s,enter short txrx mode fail!\n",__func__);
		return -EINVAL;
	}
	ret = elan_txrx_short();
	if (ret) {
		TS_LOG_ERR("[elan]:tx rx short test get data fail!\n");
		return -EINVAL;
	}
	ret = elan_txrx_short_test();
	if (ret) {
		TS_LOG_ERR("[elan]:tx rx short test Fail!\n");
		strncat(result,"4F;",strlen("4F;"));
	} else {
		TS_LOG_INFO("[elan]:tx rx short test Pass!\n");
		strncat(result,"4P;",strlen("4P;"));
	}
	return NO_ERR;
}


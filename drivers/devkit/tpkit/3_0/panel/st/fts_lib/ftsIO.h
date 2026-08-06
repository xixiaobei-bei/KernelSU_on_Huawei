/*
  *
  **************************************************************************
  **                        STMicroelectronics				**
  **************************************************************************
  **                        marco.cali@st.com				**
  **************************************************************************
  *                                                                        *
  *                     I2C/SPI Communication				*
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */
/*!
  * \file ftsIO.h
  * \brief Contains all the definitions and prototypes used and implemented in
  * ftsIO.c
  */

#ifndef FTS_IO_H
#define FTS_IO_H

#include "ftsSoftware.h"

#define I2C_RETRY		3	/* /< number of retry in case of i2c
					 * failure */
#define I2C_WAIT_BEFORE_RETRY	2	/* /< wait in ms before retry an i2c
					 * transaction */

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
struct i2c_client *getClient(void);


int openChannel(void *clt);
struct device *getDev(void);



/*************** NEW I2C API ****************/
int changeSAD(u8 sad);

int fts_read(u8 *outBuf, int byteToRead);
int fts_writeRead(u8 *cmd, int cmdLength, u8 *outBuf, int byteToRead);
int fts_write(u8 *cmd, int cmdLength);
int fts_writeFwCmd(u8 *cmd, int cmdLenght);
int fts_write_then_write_read(u8 *write_cmd, int write_cmd_length,
	u8 *read_cmd, int read_cmd_length, u8 *out_buf, int byte_to_read);
int fts_writeU8UX(u8 cmd, AddrSize addrSize, u64 address, u8 *data, int
		  dataSize);
int fts_writeReadU8UX(u8 cmd, AddrSize addrSize, u64 address, u8 *outBuf, int
		      byteToRead, int hasDummyByte);
int fts_chunk_write(u8 cmd_first, AddrSize addr_size_first, u8 cmd_second,
	AddrSize addr_size_second, u64 address, u8 *data, int data_size);
int fts_chunk_write_read(u8 cmd_first, AddrSize addr_size_first,
	u8 cmd_second, AddrSize addr_size_second, u64 address,
	u8 *out_buf, int byte_to_read, int has_dummy_byte);
#endif

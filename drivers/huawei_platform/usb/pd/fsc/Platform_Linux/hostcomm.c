/*
 * hostcomm.c
 *
 * hostcomm driver
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

#ifdef FSC_DEBUG

#include "fusb30x_global.h"
#include "FSCTypes.h"
#include "platform_helpers.h"
#include "../core/core.h"
#include "hostcomm.h"

/*
 * Function:fusb_hc_handle_i2c_read
 * inBuf:Input buffer to parse
 * inBuf[0]:Addr, other bytes ignored
 * outBuf:I2C read result stored in outBuf[0]
 * Return:none
 * Description:Read an unsigned byte from the I2C peripheral
 */
static void fusb_hc_handle_i2c_read(u8 *in_buf, u8 *out_buf)
{
	if (!fusb_I2C_ReadData(in_buf[1], out_buf))
		pr_err("FUSB %s Error:Could not read I2C Data\n", __func__);
}

/*
 * Function:fusb_hc_handle_i2c_write
 * inBuf:Input buffer to parse
 * inBuf[1]:Start of address range to write to
 * inBuf[2]:Num bytes to write
 * inBuf[3.. inBuf[1]]:Data to write
 * outBuf:Output buffer populated with result
 * Return:none
 * Description:Write an unsigned byte to the I2C peripheral
 */
static void fusb_hc_handle_i2c_write(u8 *in_buf, u8 *out_buf)
{
	if (!fusb_I2C_WriteData(in_buf[1], in_buf[2], &in_buf[3])) {
		pr_err("FUSB %s Error:Could not write I2C Data\n", __func__);
		out_buf[0] = 0; /* Notify failure */
	} else {
		out_buf[0] = 1; /* Notify success */
	}
}

/*
 * Function:fusb_hc_get_vbus_5v
 * Input:none
 * outBuf:Output buffer populated with result
 * Success:outBuf[0] = 1, outBuf[1] = value
 * Failure:outBuf[0] = 0
 * Return:none
 * Description:Get the value of VBus 5V
 */
static void fusb_hc_get_vbus_5v(u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = fusb_GPIO_Get_VBus5v() ? 1 : 0;
}

/*
 * Function:fusb_hc_set_vbus_5v
 * in_msg_buffer:in_msg_buffer[1] = [1 || 0], value to write
 * out_msg_buffer:output buffer populated with result:
 * Success:out_msg_buffer[0] = 1, out_msg_buffer[1] = value that was set
 * Failure:out_msg_buffer[0] = 0
 * Return:none
 * Description:Set the value of VBus 5V
 */
static void fusb_hc_set_vbus_5v(u8 *in_msg_buffer, u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	fusb_GPIO_Set_VBus5v(in_msg_buffer[1]);
	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = fusb_GPIO_Get_VBus5v() ? 1 : 0;
}

/*
 * Function:fusb_hc_get_intn
 * Input:none
 * out_msg_buffer:output buffer populated with result:
 * Success:out_msg_buffer[0] = 1, out_msg_buffer[1] = value
 * Failure:out_msg_buffer[0] = 0
 * Return:none
 * Description:Get the value of Int_N
 */
static void fusb_hc_get_intn(u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = fusb_GPIO_Get_IntN() ? 1 : 0;
}

/*
 * Function:fusb_hc_get_timer_ticks
 * Input:none
 * out_msg_buffer:Output buffer populated with result
 * out_msg_buffer[0]:1 on success, 0 on failure
 * out_msg_buffer[1]:Number of timer ticks passed
 * out_msg_buffer[2]:Number of times the timer tick ticker rolled over
 * Return:none
 * Description:[FSC_DEBUG]Get the number of timer ticks that have passed
 */
static void fusb_hc_get_timer_ticks(u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = chip->dbgTimerTicks;
	out_msg_buffer[2] = chip->dbgTimerRollovers;
}

/*
 * Function:fusb_hc_get_sm_ticks
 * Input:none
 * out_msg_buffer:Output buffer populated with result
 * out_msg_buffer[0]:1 on success, 0 on failure
 * out_msg_buffer[1]:Number of SM ticks passed
 * out_msg_buffer[2]:Number of times the SM tick ticker rolled over
 * Return:none
 * Description:[FSC_DEBUG]Get the number of SM ticks that have passed
 */
static void fusb_hc_get_sm_ticks(u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = chip->dbgSMTicks;
	out_msg_buffer[2] = chip->dbgSMRollovers;
}

/*
 * Function: fusb_hc_get_gpio_sm_toggle
 * Input:none
 * out_msg_buffer:Output buffer populated with result
 * Success:out_msg_buffer[0] = 1
 * out_msg_buffer[1] = value
 * Failure:out_msg_buffer[0] = 0
 * Return:none
 * Description:Get the value of the SM toggle GPIO
 */
static void fusb_hc_get_gpio_sm_toggle(u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = dbg_fusb_GPIO_Get_SM_Toggle() ? 1 : 0;
}

/*
 * Function:fusb_hc_set_gpio_sm_toggle
 * in_msg_buffer:in_msg_buffer[1] = [1 || 0], value to write
 * Output buffer populated with result
 * Success:out_msg_buffer[0] = 1
 * out_msg_buffer[1] = value that was set
 * Failure:out_msg_buffer[0] = 0
 * Return:none
 * Description:Set the value of the SM toggle GPIO
 */
static void fusb_hc_set_gpio_sm_toggle(u8 *in_msg_buffer,
	u8 *out_msg_buffer)
{
	struct fusb30x_chip *chip = fusb30x_GetChip();

	if (!chip) {
		pr_err("FUSB %s Error:Chip structure is NULL\n", __func__);
		out_msg_buffer[0] = 0;
		return;
	}

	dbg_fusb_GPIO_Set_SM_Toggle(in_msg_buffer[1]);
	out_msg_buffer[0] = 1;
	out_msg_buffer[1] = dbg_fusb_GPIO_Get_SM_Toggle() ? 1 : 0;
}

/*
 * Function:fusb_process_msg
 * in_msg_buffer: Input array to process
 * out_msg_buffer: Result is stored here
 * Return:none
 * Description:Processes the input array as a command with optional data.
 * Provides a debugging interface to the caller.
 */
void fusb_process_msg(u8 *in_msg_buffer, u8 *out_msg_buffer)
{
	u8 i;

	if (!in_msg_buffer || !out_msg_buffer)
		return;

	/* Echo the request */
	out_msg_buffer[0] = in_msg_buffer[0];

	switch (in_msg_buffer[0]) {
	case CMD_GETDEVICEINFO:
		if (in_msg_buffer[1] != 0) {
			/* Return that the version is not recognized */
			out_msg_buffer[1] = 0x01;
		} else {
			/* [1,4] Return that the command was recognized */
			out_msg_buffer[1] = 0x00;
			out_msg_buffer[4] = MY_MCU;
			/* [5,6,7] USB to I2C device with USB PD and Type C */
			out_msg_buffer[5] = MY_DEV_TYPE;

			out_msg_buffer[6] = core_get_rev_lower();
			out_msg_buffer[7] = core_get_rev_upper();

			/* [9:8] make up the board configuration used,0xFF:mask */
			out_msg_buffer[8] = 0xFF & MY_BC;
			out_msg_buffer[9] = 0xFF & (MY_BC >> 8);

			/* 16:Loop through all the serial number bytes */
			for (i = 0; i < 16; i++)
				/* 10:return each of bytes from EEPROM data */
				out_msg_buffer[i + 10] = 0x00;

			/* 26:Reports if this firmware runs the test suite */
			out_msg_buffer[26] = TEST_FIRMWARE;
		}
		break;
	case CMD_USBPD_BUFFER_READ:
		/* Grab as many bytes from the USB PD buffer as possible */
		core_process_pd_buffer_read(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_USBPD_STATUS:
		/* Get the status of the PD state machines */
		core_process_typec_pd_status(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_USBPD_CONTROL:
		/* Update the PD state machine settings */
		core_process_typec_pd_control(in_msg_buffer, out_msg_buffer);
		break;
#ifdef FSC_HAVE_SRC
	case CMD_GET_SRC_CAPS:
		core_get_source_caps(out_msg_buffer);
		break;
#endif /* FSC_HAVE_SRC */
#ifdef FSC_HAVE_SNK
	case CMD_GET_SINK_CAPS:
		core_get_sink_caps(out_msg_buffer);
		break;
	case CMD_GET_SINK_REQ:
		core_get_sink_req(out_msg_buffer);
		break;
#endif /* FSC_HAVE_SNK */
	case CMD_ENABLE_PD:
		core_enable_pd(TRUE);
		out_msg_buffer[0] = CMD_ENABLE_PD;
		out_msg_buffer[1] = 1;
		break;
	case CMD_DISABLE_PD:
		core_enable_pd(FALSE);
		out_msg_buffer[0] = CMD_DISABLE_PD;
		out_msg_buffer[1] = 0;
		break;
	case CMD_GET_ALT_MODES:
		out_msg_buffer[0] = CMD_GET_ALT_MODES;
		out_msg_buffer[1] = core_get_alternate_modes();
		break;
	case CMD_GET_MANUAL_RETRIES:
		out_msg_buffer[0] = CMD_GET_MANUAL_RETRIES;
		out_msg_buffer[1] = core_get_manual_retries();
		break;
	case CMD_SET_STATE_UNATTACHED:
		core_set_state_unattached();
		out_msg_buffer[0] = CMD_SET_STATE_UNATTACHED;
		out_msg_buffer[1] = 0;
		break;
	case CMD_ENABLE_TYPEC_SM:
		core_enable_typec(TRUE);
		out_msg_buffer[0] = CMD_ENABLE_TYPEC_SM;
		out_msg_buffer[1] = 0;
		break;
	case CMD_DISABLE_TYPEC_SM:
		core_enable_typec(FALSE);
		out_msg_buffer[0] = CMD_DISABLE_TYPEC_SM;
		out_msg_buffer[1] = 0;
		break;
	case CMD_DEVICE_LOCAL_REGISTER_READ:
		core_process_local_register_request(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_SET_STATE:
		core_process_set_typec_state(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_READ_STATE_LOG:
		core_process_read_typec_state_log(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_READ_PD_STATE_LOG:
		core_process_read_pd_state_log(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_READ_I2C:
		fusb_hc_handle_i2c_read(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_WRITE_I2C:
		fusb_hc_handle_i2c_write(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_GET_VBUS5V:
		fusb_hc_get_vbus_5v(out_msg_buffer);
		break;
	case CMD_SET_VBUS5V:
		fusb_hc_set_vbus_5v(in_msg_buffer, out_msg_buffer);
		break;
	case CMD_GET_INTN:
		fusb_hc_get_intn(out_msg_buffer);
		break;
	case CMD_SEND_HARD_RESET:
		core_send_hard_reset();
		out_msg_buffer[0] = CMD_SEND_HARD_RESET;
		out_msg_buffer[1] = 0;
		break;
#ifdef FSC_DEBUG
	case CMD_GET_TIMER_TICKS:
		fusb_hc_get_timer_ticks(out_msg_buffer);
		break;
	case CMD_GET_SM_TICKS:
		fusb_hc_get_sm_ticks(out_msg_buffer);
		break;
	case CMD_GET_GPIO_SM_TOGGLE:
		fusb_hc_get_gpio_sm_toggle(out_msg_buffer);
		break;
	case CMD_SET_GPIO_SM_TOGGLE:
		fusb_hc_set_gpio_sm_toggle(in_msg_buffer, out_msg_buffer);
		break;
#endif /* FSC_DEBUG */
	default:
		/* Return that the request is not implemented */
		out_msg_buffer[1] = 0x01;
		break;
	}
}

#endif /* FSC_DEBUG */

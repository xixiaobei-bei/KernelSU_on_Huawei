/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mipi switch between AP and MCU
 * Author: lijiawen
 * Create: 2020-04-20
 */
#include "lcdkit_mipi_switch.h"
#include "lcdkit_panel.h"
#include "lcdkit_dbg.h"
#include "lcdkit_disp.h"
#include "hisi_mipi_dsi.h"
#include "securec.h"
#include "lcdkit_ext.h"
#ifdef CONFIG_EXT_INPUTHUB
#include "ext_sensorhub_api.h"
#endif
#define TIMEOUT 500
#define EXPECTVALUE0A 0x9c
#define EXPECTVALUE0A_ILDE 0xdc
#define EXPECTVALUE0D 0
#define EXPECTVALUE0E 0x80
#define LCD_RESET_COMMAND_ID 0xA3
#define AP_MCU_SERVICE_ID 0x01
#define WAKE_LOCK_TIME_MS 1500

struct display_switch {
	struct platform_device *pdev;
	int mipi_request_gpio;
	int mipi_switch_gpio;
	int i2c_switch_gpio;
	int mipi_switch_irq;
	int i2c_switch_irq;
	struct work_struct sw;
	struct work_struct i2csw;
	int i2c_irq_value;
};

struct display_switch *g_display_switch = NULL;
bool g_mipi_ap = false;
extern struct hisifb_esd;
static int g_i2c_irq_value = 0;
static int g_mcu_reboot_flag = 0;
void lcdkit_recovery_display(struct hisi_fb_data_type *hisifd);
static int g_recovery_state = 0;
struct semaphore sr_wake_sema;
struct wakeup_source wake_lock;

int get_watch_recovery_state(void)
{
	return g_recovery_state;
}

void request_mipi_gpio_operate(bool pull_up)
{
	if (pull_up) {
		gpio_direction_output(g_display_switch->mipi_request_gpio, 1);
		LCDKIT_DEBUG("mipi_request_gpio power_on\n");
	} else {
		gpio_direction_output(g_display_switch->mipi_request_gpio, 0);
		LCDKIT_DEBUG("mipi_request_gpio power_off\n");
	}
}

static int request_common_gpio(
	int *gpio, const char *compat, const char *name)
{
	struct device_node *np = NULL;
	np = of_find_compatible_node(NULL, NULL, compat);
	if (np == NULL) {
		LCDKIT_ERR("%s: node not found\n", __func__);
		return -ENODEV;
	}

	*gpio = of_get_named_gpio(np, name, 0);
	if (*gpio < 0) {
		LCDKIT_ERR("%s:%d.\n", name, *gpio);
			return -ENODEV;
	}

	if (gpio_request(*gpio, name) < 0) {
		LCDKIT_ERR("Failed to request gpio %d for %s\n", *gpio, name);
		return -ENODEV;
	}
	return 0;
}


static irqreturn_t display_switch_handler(int irq, void *arg)
{
	schedule_work(&g_display_switch->sw);
	return IRQ_HANDLED;
}

static irqreturn_t i2c_switch_handler(int irq, void *arg)
{
	schedule_work(&g_display_switch->i2csw);
	return IRQ_HANDLED;
}

static void do_display_switch(struct work_struct *work)
{
	struct platform_device *pdev = NULL;
	struct hisi_fb_data_type* hisifd = NULL;
	int req_display = gpio_get_value(g_display_switch->mipi_request_gpio);
	int irq_value = gpio_get_value(g_display_switch->mipi_switch_gpio);

	LCDKIT_INFO("mipi recieve irq! req_display=%d, irq_value=%d\n", req_display, irq_value);
	lcdkit_get_pdev(&pdev);
	if (pdev == NULL)
		return;
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL)
		return;
	if (!irq_value) {
		hisifd->panel_info.esd_enable = false;
		g_mipi_ap = false;
		if (req_display)
			g_mcu_reboot_flag = 1;
	} else {
		g_mipi_ap = true;
		lcdkit_info.panel_infos.mipi_aod_esd_switch = 1;

		if (!hisifd->aod_function) {
			if (hisifd->esd_ctrl.esd_check_wq == NULL) {
				LCDKIT_INFO("esd disable\n");
				return;
			}
			LCDKIT_INFO("start esd\n");
			hisifd->panel_info.esd_enable = true;
			queue_work(hisifd->esd_ctrl.esd_check_wq,
				&(hisifd->esd_ctrl.esd_check_work));
			hrtimer_start(&hisifd->esd_ctrl.esd_hrtimer,
				ktime_set(ESD_CHECK_TIME_PERIOD / 1000,
				(ESD_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);
		} else {
			if (req_display && g_mcu_reboot_flag) {
				mutex_lock(&lcdkit_info.panel_infos.switch_lock);
				lcdkit_recovery_display(hisifd);
				mutex_unlock(&lcdkit_info.panel_infos.switch_lock);
			}
		}
		g_mcu_reboot_flag = 0;
	}
}

static void do_i2c_switch(struct work_struct *work)
{
	int i2c_irq_value = gpio_get_value(g_display_switch->i2c_switch_gpio);
	int data_for_ts_resume;
	int data_for_ts_suspend;
	LCDKIT_INFO("i2c recieve irq! i2c_irq_value=%d\n", i2c_irq_value);
	if (i2c_irq_value && !g_i2c_irq_value) {
		lcdkit_notifier_call_chain(LCDKIT_TS_I2C_TO_AP,
				&data_for_ts_resume);
		data_for_ts_resume = LCDKIT_NO_SYNC_TIMEOUT;
		lcdkit_notifier_call_chain(LCDKIT_TS_RESUME_DEVICE,
			&data_for_ts_resume);
		lcdkit_notifier_call_chain(LCDKIT_TS_AFTER_RESUME,
			&data_for_ts_resume);
	} else if (!i2c_irq_value && g_i2c_irq_value) {
		data_for_ts_suspend = LCDKIT_NO_SYNC_TIMEOUT;
		lcdkit_notifier_call_chain(LCDKIT_TS_BEFORE_SUSPEND,
				&data_for_ts_suspend);
		lcdkit_notifier_call_chain(LCDKIT_TS_SUSPEND_DEVICE,
				&data_for_ts_suspend);
		lcdkit_notifier_call_chain(LCDKIT_TS_I2C_TO_MCU,
				&data_for_ts_suspend);
	} else {
		LCDKIT_INFO("i2c irq value same as last, do nothing\n");
	}
	g_i2c_irq_value = i2c_irq_value;
}

int i2c_switch_ap(void)
{
	g_display_switch->i2c_irq_value = gpio_get_value(g_display_switch->i2c_switch_gpio);
	LCDKIT_INFO("i2c_switch_ap i2c_irq_value=%d\n", g_display_switch->i2c_irq_value);
	return g_display_switch->i2c_irq_value;
}

int get_i2c_switch_value(void)
{
	LCDKIT_INFO("get_i2c_switch_value i2c_irq_value=%d\n", g_display_switch->i2c_irq_value);
	return g_display_switch->i2c_irq_value;
}

static int init_ts_gpio(struct platform_device *pdev)
{
	int status;
	int retval;
	LCDKIT_INFO("+. \n");
	if (pdev == NULL) {
		LCDKIT_ERR("pdev is NULL");
		return -EINVAL;
	}

	status = request_common_gpio(&g_display_switch->i2c_switch_gpio,
		"i2c,switchselect", "i2c_switch");
	if (status != 0) {
		LCDKIT_ERR("init_gpio i2c status = %d", status);
		return -EINVAL;
	}
	if (gpio_direction_input(g_display_switch->i2c_switch_gpio) < 0) {
		LCDKIT_ERR("Failed to set dir %d for i2c_switch_gpio\n",
			g_display_switch->i2c_switch_gpio);
		return -EINVAL;
	}
	g_i2c_irq_value = gpio_get_value(g_display_switch->i2c_switch_gpio);
	LCDKIT_INFO("init_gpio i2c i2c_irq_value=%d\n", g_i2c_irq_value);
	g_display_switch->i2c_switch_irq = gpio_to_irq(g_display_switch->i2c_switch_gpio);
	retval = request_irq(g_display_switch->i2c_switch_irq, i2c_switch_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
		"mipi sw irq", g_display_switch->pdev);
	if (retval < 0) {
		LCDKIT_ERR("Couldn't acquire i2c_switch_irq reval = %d\n", retval);
		return -EINVAL;
	}
	INIT_WORK(&g_display_switch->i2csw, do_i2c_switch);
	LCDKIT_INFO("-. \n");
	return status;
}

static int init_display_gpio(struct platform_device *pdev)
{
	int status;
	int retval;
	int irq_value;
	LCDKIT_INFO("+. \n");
	if (pdev == NULL) {
		LCDKIT_ERR("pdev is NULL");
		return -EINVAL;
	}

	status = request_common_gpio(&g_display_switch->mipi_switch_gpio,
		"mipi,switchselect", "mipi_switch");
	if (status != 0) {
		LCDKIT_ERR("init_gpio mipi status = %d", status);
		return -EINVAL;
	}
	if (gpio_direction_input(g_display_switch->mipi_switch_gpio) < 0) {
		LCDKIT_ERR("Failed to set dir %d for mipi_switch_gpio\n",
			g_display_switch->mipi_switch_gpio);
		return -EINVAL;
	}
	irq_value = gpio_get_value(g_display_switch->mipi_switch_gpio);
	LCDKIT_INFO("init_gpio mipi irq_value=%d\n", irq_value);
	g_display_switch->mipi_switch_irq = gpio_to_irq(g_display_switch->mipi_switch_gpio);
	retval = request_irq(g_display_switch->mipi_switch_irq, display_switch_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
		"mipi sw irq", g_display_switch->pdev);
	if (retval < 0) {
		LCDKIT_ERR("Couldn't acquire mipi_switch_irq reval = %d\n", retval);
		return -EINVAL;
	}

	status = request_common_gpio(&g_display_switch->mipi_request_gpio,
		"mipi,requestdisplay", "mipi_request");
	INIT_WORK(&g_display_switch->sw, do_display_switch);
	LCDKIT_INFO("-. \n");
	return status;
}

void mipi_to_ap(struct hisi_fb_data_type *hisifd)
{
	int try_times = 0;

	LCDKIT_INFO("set request display gpio up!\n");
	request_mipi_gpio_operate(1);
	while (!g_mipi_ap) {
		mdelay(1);
		if (++try_times > TIMEOUT) {
			LCDKIT_ERR("wait for irq_mipi_on timeout!\n");
			break;
		}
	}
}

void mipi_to_mcu(struct hisi_fb_data_type *hisifd)
{
	int try_times = 0;

	LCDKIT_INFO("set request display gpio down!\n");
	request_mipi_gpio_operate(0);
	while (g_mipi_ap) {
		mdelay(1);
		if (++try_times > TIMEOUT) {
			LCDKIT_ERR("wait for irq_mipi_off timeout!\n");
			break;
		}
	}
}

int check_display_on(struct hisi_fb_data_type *hisifd)
{
	int ret = 1;
	uint32_t read_value[MAX_REG_READ_COUNT] = {0};
	int i;
	char* expect_ptr = NULL;

	if (lcdkit_info.panel_infos.check_reg_value.buf == NULL) {
		LCDKIT_ERR("check_reg_value buf is null pointer \n");
		return -EINVAL;
	}

	expect_ptr = lcdkit_info.panel_infos.check_reg_value.buf;
	lcdkit_dsi_rx(hisifd, read_value, 1, &lcdkit_info.panel_infos.check_reg_cmds);
	for (i = 0; i < lcdkit_info.panel_infos.check_reg_cmds.cmd_cnt; i++) {
		if ((char)read_value[i] != expect_ptr[i]) {
			ret = 0;
			LCDKIT_INFO("read_value[%u] = 0x%x, but expect_ptr[%u] = 0x%x!\n",
				i, read_value[i], i, expect_ptr[i]);
			break;
		}
		LCDKIT_INFO("read_value[%u] = 0x%x same with expect value!\n",
				i, read_value[i]);
	}

	return ret;
}

void mipi_switch_release(struct platform_device* pdev)
{
	if (g_display_switch == NULL) {
		LCDKIT_ERR("g_display_switch is null pointer\n");
		return;
	}
	if (g_display_switch->mipi_request_gpio != 0)
		gpio_free(g_display_switch->mipi_request_gpio);
	if (g_display_switch->mipi_switch_gpio != 0)
		gpio_free(g_display_switch->mipi_switch_gpio);
	if (g_display_switch->mipi_switch_irq != 0)
		free_irq(g_display_switch->mipi_switch_irq, pdev);
	if (g_display_switch != NULL)
		kfree(g_display_switch);
	g_display_switch = NULL;
}

static int lcdkit_sr_notify(struct notifier_block *notify_block,
			  unsigned long mode, void *unused)
{
	switch (mode) {
	case PM_POST_SUSPEND:
		LCDKIT_INFO("get in PM_POST_SUSPEND, resume");
		up(&sr_wake_sema);
		break;
	case PM_SUSPEND_PREPARE:
		LCDKIT_INFO("get in PM_SUSPEND_PREPARE, suspend");
		if (down_interruptible(&sr_wake_sema))
			LCDKIT_ERR("down sr wake sema error");
		break;
	default:
	    break;
	}
	return 0;
}

static struct notifier_block g_lcdkit_sr_notifier = {
	.notifier_call = lcdkit_sr_notify,
	.priority = INT_MIN,
};

void mipi_switch_init(struct platform_device* pdev)
{
	int ret;

	LCDKIT_INFO("mipi_switch_init\n");
	wakeup_source_init(&wake_lock, "lcdkit_recovery_display");
	sema_init(&sr_wake_sema, 1);
	ret = register_pm_notifier(&g_lcdkit_sr_notifier);
	if (ret < 0)
		LCDKIT_ERR("register lcd pm_notifier failed, ret = %d!", ret);
	g_display_switch = kzalloc(sizeof(struct display_switch), GFP_KERNEL);
	g_display_switch->pdev = pdev;
	init_display_gpio(pdev);
	init_ts_gpio(pdev);
}

void lcd_frame_refresh(struct hisi_fb_data_type *hisifd)
{
#define BUF_LEN 64
	char *envp[2] = {NULL};
	char buf[BUF_LEN];

	snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "Refresh=1");
	envp[0] = buf;
	envp[1] = NULL;

	kobject_uevent_env(&(hisifd->fbi->dev->kobj), KOBJ_CHANGE, envp);
	LCDKIT_INFO("refresh=1!\n");
}

void lcdkit_recovery_display(struct hisi_fb_data_type *hisifd)
{
	uint32_t bl_level_cur;
	if (hisifd == NULL) {
		LCDKIT_ERR("hisifd is null\n");
		return;
	}
	LCDKIT_INFO("enter!\n");
	down(&sr_wake_sema);
	__pm_wakeup_event(&wake_lock, WAKE_LOCK_TIME_MS);
	up(&sr_wake_sema);

	down(&hisifd->brightness_esd_sem);
	bl_level_cur = hisifd->bl_level;
	up(&hisifd->brightness_esd_sem);
	/* lcd panel off */
	g_recovery_state = 1; // do not pulldown request gpio
	if (hisi_fb_blank_sub(FB_BLANK_POWERDOWN, hisifd->fbi))
		LCDKIT_ERR("lcd panel off error!\n");
	g_recovery_state = 0;
	msleep(100);

	lcdkit_info.panel_infos.mipi_aod_esd_switch = 1;
	/* lcd panel on */
	if (hisi_fb_blank_sub(FB_BLANK_UNBLANK, hisifd->fbi))
		LCDKIT_ERR("lcd panel on error!\n");
	/* refresh frame */
	lcd_frame_refresh(hisifd);
	/* backlight on */
	down(&hisifd->brightness_esd_sem);
	if (!bl_level_cur)
		bl_level_cur = lcdkit_info.panel_infos.watch_aod_normal_brightness;
	hisifb_set_backlight(hisifd, bl_level_cur, false);
	up(&hisifd->brightness_esd_sem);
	__pm_relax(&wake_lock);
	LCDKIT_INFO("--!\n");
}

#ifdef CONFIG_EXT_INPUTHUB
int ext_reset_lcd_command(void)
{
	int ret;
	struct command send_lcd_reset = {0};
	unsigned char buff = 0;
	send_lcd_reset.service_id = AP_MCU_SERVICE_ID;
	send_lcd_reset.command_id = LCD_RESET_COMMAND_ID;
	send_lcd_reset.send_buffer = &buff;
	send_lcd_reset.send_buffer_len = 1;

	ret = send_command(UPGRADE_CHANNEL, &send_lcd_reset, false, NULL);
	LCDKIT_INFO("send_command ret %d\n", ret);
	if (ret < 0)
		LCDKIT_ERR("send reset cmd failed\n");

	return ret;
}
#endif

void read_watch_reg_value(struct hisi_fb_data_type *hisifd)
{
	char read0a[] = {0x0a};
	char read0d[] = {0x0d};
	char read0e[] = {0x0e};
	uint32_t read_value = 0xff;
	int try_times = 3;
	int res = 0;
	struct lcdkit_dsi_cmd_desc check_reg_dsi_cmd1 = {DTYPE_DCS_READ, 1, 0, 0, 10, LCDKIT_WAIT_TYPE_US, 1, read0a};
	struct lcdkit_dsi_panel_cmds lcd_check_reg1 = {NULL, 0, &check_reg_dsi_cmd1, 1, 0, 0};

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		up(&hisifd->blank_sem);
		LCDKIT_INFO("lcd has been powered off, do nothing\n");
		return;
	}
	for (int i = 0; i < try_times; i++) {
		mdelay(2); // delay 2ms
		LCDKIT_PANEL_CMD_REQUEST();
		hisifb_activate_vsync(hisifd);
		lcdkit_dsi_rx(hisifd, &read_value, 1, &lcd_check_reg1);
		if (read_value != EXPECTVALUE0A && read_value != EXPECTVALUE0A_ILDE) {
			LCDKIT_ERR("read_reg_value, 0a:0x%x\n", read_value);
			hisifb_deactivate_vsync(hisifd);
			LCDKIT_PANEL_CMD_RELEASE();
			break;
		}

		read_value = 0xff;
		struct lcdkit_dsi_cmd_desc check_reg_dsi_cmd3 = {DTYPE_DCS_READ, 1, 0, 0, 10, LCDKIT_WAIT_TYPE_US, 1, read0d};
		struct lcdkit_dsi_panel_cmds lcd_check_reg3 = {NULL, 0, &check_reg_dsi_cmd3, 1, 0, 0};
		lcdkit_dsi_rx(hisifd, &read_value, 1, &lcd_check_reg3);
		if (read_value != EXPECTVALUE0D) {
			LCDKIT_ERR("read_reg_value, 0d:0x%x\n", read_value);
			hisifb_deactivate_vsync(hisifd);
			LCDKIT_PANEL_CMD_RELEASE();
			break;
		}

		read_value = 0xff;
		struct lcdkit_dsi_cmd_desc check_reg_dsi_cmd4 = {DTYPE_DCS_READ, 1, 0, 0, 10, LCDKIT_WAIT_TYPE_US, 1, read0e};
		struct lcdkit_dsi_panel_cmds lcd_check_reg4 = {NULL, 0, &check_reg_dsi_cmd4, 1, 0, 0};
		lcdkit_dsi_rx(hisifd, &read_value, 1, &lcd_check_reg4);
		if (read_value != EXPECTVALUE0E) {
			LCDKIT_ERR("read_reg_value, 0e:0x%x\n", read_value);
			/* ignore dsi */
			if ((read_value & 0xF0) != (EXPECTVALUE0E & 0xF0)) {
				hisifb_deactivate_vsync(hisifd);
				LCDKIT_PANEL_CMD_RELEASE();
				break;
			}
		}

		hisifb_deactivate_vsync(hisifd);
		LCDKIT_PANEL_CMD_RELEASE();
		res = 1;
	}
	up(&hisifd->blank_sem);
#ifdef CONFIG_EXT_INPUTHUB
	if (!res)
		ext_reset_lcd_command();
#endif
}


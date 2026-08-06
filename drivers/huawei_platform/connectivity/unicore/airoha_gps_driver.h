#ifndef _AIROHA_GPS_DRIVER_H
#define _AIROHA_GPS_DRIVER_H

#include <linux/cdev.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>

static int airoha_gps_open(struct inode *inode, struct file *file_p);
static ssize_t airoha_gps_read(struct file *file_p, char __user *user, size_t len, loff_t *offset);
static ssize_t airoha_gps_write(struct file *file_p, const char __user *user, size_t len, loff_t *offset);
static long airoha_gps_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int airoha_gps_release(struct inode *inode, struct file *file_p);
static int airoha_gps_power_init(void);
static void airoha_gps_power_exit(void);

/*
 * voltage regulator information required for configuring the
 * lbs chipset
 */
struct lbs_power_vreg_data {
	/* voltage regulator handle */
	struct regulator *reg;
	/* regulator name */
	const char *name;
	/* voltage levels to be set */
	unsigned int low_vol_level;
	unsigned int high_vol_level;
	/* current level to be set */
	unsigned int load_uA;
	/*
	 * is set voltage supported for this regulator?
	 * false => set voltage is not supported
	 * true  => set voltage is supported
	 *
	 * Some regulators (like gpio-regulators, LVS (low voltage swtiches)
	 * PMIC regulators) dont have the capability to call
	 * regulator_set_voltage or regulator_set_optimum_mode
	 * Use this variable to indicate if its a such regulator or not
	 */
	bool set_voltage_sup;
	/* is this regulator enabled? */
	bool is_enabled;
};

/*
 * Platform data for the lbs power driver.
 */
struct lbs_power_platform_data {
	/* VDD_DIG digital voltage regulator */
	struct lbs_power_vreg_data *lbs_vdd_mpmu;
	struct lbs_power_vreg_data *lbs_vdd_spmu;

	/* Optional: lbs power setup function */
	int (*lbs_power_setup)(int id);
};

#endif

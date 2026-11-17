#ifndef _HISI_PMIC_VIBRATOR_H
#define _HISI_PMIC_VIBRATOR_H

#define HISI_PMIC_VIBRATOR_DEFAULT_NAME "vibrator"
#define HISI_PMIC_VIBRATOR_CDEVIE_NAME "haptics"

#define PMIC_VIBRATOR_BRAKE_TIME_OUT 100
#define PMIC_VIBRATOR_HAP_BUF_LENGTH 16
#define PMIC_VIBRATOR_POWER_ON 1
#define PMIC_VIBRATOR_POWER_OFF 0

#define HISI_PMIC_VIBRATOR_HAPTIC_1   1
#define HISI_PMIC_VIBRATOR_HAPTIC_2   2
#define HISI_PMIC_VIBRATOR_HAPTIC_3   3
#define HISI_PMIC_VIBRATOR_HAPTIC_4   4
#define HISI_PMIC_VIBRATOR_HAPTIC_5   5
#define HISI_PMIC_VIBRATOR_HAPTIC_6   6
#define HISI_PMIC_VIBRATOR_HAPTIC_7   7
#define HISI_PMIC_VIBRATOR_HAPTIC_8   8
#define HISI_PMIC_VIBRATOR_HAPTIC_9   9
#define HISI_PMIC_VIBRATOR_HAPTIC_10  10

#define EXT_VIBRATOR_SID             0x01
#define EXT_VIBRATOR_WORK_CID        0x93
#define EXT_VIBRATOR_ON              0x01
#define EXT_VIBRATOR_OFF             0x02

#define EXT_VIBR_HAP_CID             0x94
#define EXT_VIBR_HAPTICS_1           0x01
#define EXT_VIBR_HAPTICS_2           0x02
#define EXT_VIBR_HAPTICS_3           0x03
#define EXT_VIBR_HAPTICS_4           0x04
#define EXT_VIBR_HAPTICS_5           0x05
#define EXT_VIBR_HAPTICS_6           0x06
#define EXT_VIBR_HAPTICS_7           0x07
#define EXT_VIBR_HAPTICS_8           0x08
#define EXT_VIBR_HAPTICS_9           0x09
#define EXT_VIBR_HAPTICS_10          0x0A

#define array_size(a) (sizeof(a) / sizeof(a[0]))

struct ext_vibrator_haptics_type {
	const int haptics_num;
	const unsigned char command;
};

static const struct ext_vibrator_haptics_type g_haptics_table[] = {
	{ HISI_PMIC_VIBRATOR_HAPTIC_1,       EXT_VIBR_HAPTICS_1 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_2,       EXT_VIBR_HAPTICS_2 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_3,       EXT_VIBR_HAPTICS_3 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_4,       EXT_VIBR_HAPTICS_4 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_5,       EXT_VIBR_HAPTICS_5 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_6,       EXT_VIBR_HAPTICS_6 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_7,       EXT_VIBR_HAPTICS_7 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_8,       EXT_VIBR_HAPTICS_8 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_9,       EXT_VIBR_HAPTICS_9 },
	{ HISI_PMIC_VIBRATOR_HAPTIC_10,      EXT_VIBR_HAPTICS_10 }
};

#endif

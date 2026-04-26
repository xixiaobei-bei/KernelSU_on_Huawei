#include "dubai_sensorhub_stats.h"

#include <linux/slab.h>
#include <linux/types.h>

#include <chipset_common/dubai/dubai_ioctl.h>
#include <huawei_platform/inputhub/sensorhub.h>
#include <inputhub/default/protocol.h>

#include "utils/buffered_log_sender.h"
#include "utils/dubai_utils.h"

#define IOC_AOD_GET_DURATION DUBAI_SENSORHUB_DIR_IOC(R, 1, uint64_t)
#define IOC_SENSORHUB_TYPE_LIST DUBAI_SENSORHUB_DIR_IOC(W, 2, long long)
#define IOC_SENSOR_TIME_GET DUBAI_SENSORHUB_DIR_IOC(WR, 3, struct sensor_time_transmit)
#define IOC_FP_ICON_STATS DUBAI_SENSORHUB_DIR_IOC(R, 4, uint32_t)
#define IOC_SWING_GET DUBAI_SENSORHUB_DIR_IOC(WR, 5, struct swing_data)
#define IOC_TP_GET DUBAI_SENSORHUB_DIR_IOC(R, 6, struct tp_duration)
#define IOC_LCD_STATE_DURATION_GET DUBAI_SENSORHUB_DIR_IOC(R, 7, struct lcd_state_duration)

struct dubai_sensorhub_type_info {
	int sensorhub_type;
	int data[SENSORHUB_TAG_NUM_MAX];
} __packed;

struct dubai_sensorhub_type_list {
	long long timestamp;
	int size;
	int tag_count;
	struct dubai_sensorhub_type_info data[0];
} __packed;

extern int is_sensorhub_disabled(void);
extern int iomcu_dubai_log_fetch(uint32_t event_type, void* data, uint32_t length);

#ifdef CONFIG_INPUTHUB_20
static int dubai_get_aod_duration(void __user *argp);
static int dubai_get_tp_duration(void __user *argp);
static int dubai_get_sensorhub_type_list(void __user * argp);
static int dubai_get_all_sensor_stats(void __user *argp);
static int dubai_get_fp_icon_stats(void __user *argp);
static int dubai_get_swing_data(void __user *argp);
static int dubai_get_lcd_state_duration(void __user *argp);
#endif /* !CONFIG_INPUTHUB_20 */

long dubai_ioctl_sensorhub(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_AOD_GET_DURATION:
		rc = dubai_get_aod_duration(argp);
		break;
	case IOC_SENSORHUB_TYPE_LIST:
		rc = dubai_get_sensorhub_type_list(argp);
		break;
	case IOC_SENSOR_TIME_GET:
		rc = dubai_get_all_sensor_stats(argp);
		break;
	case IOC_FP_ICON_STATS:
		rc = dubai_get_fp_icon_stats(argp);
		break;
	case IOC_SWING_GET:
		rc = dubai_get_swing_data(argp);
		break;
	case IOC_TP_GET:
		rc = dubai_get_tp_duration(argp);
		break;
	case IOC_LCD_STATE_DURATION_GET:
		rc = dubai_get_lcd_state_duration(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int dubai_fetch_sensorhub_data(uint32_t event_type, void* data, uint32_t length)
{
	int ret;

	if (is_sensorhub_disabled()) {
		dubai_err("Sensorhub is disabled");
		return -EOPNOTSUPP;
	}

	ret = iomcu_dubai_log_fetch(event_type, data, length);
	if (ret < 0) {
		dubai_err("Failed to fetch sensorhub data: %u", event_type);
		return -EFAULT;
	}

	return 0;
}

static int dubai_get_sensorhub_type_list(void __user * argp)
{
	int i, j, ret = 0, count = 0;
	long long timestamp, size = 0;
	struct dubai_sensorhub_type_list *transmit = NULL;
	struct buffered_log_entry *log_entry = NULL;
	const struct app_link_info *info = NULL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		return ret;
	}

	size = cal_log_entry_len(struct dubai_sensorhub_type_list, struct dubai_sensorhub_type_info,
		SENSORHUB_TYPE_END - SENSORHUB_TYPE_BEGIN);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_SENSORHUB_TYPE_LIST);
	if (log_entry == NULL) {
		dubai_err("Failed to create buffered log entry");
		ret = -ENOMEM;
		return ret;
	}

	transmit = (struct dubai_sensorhub_type_list *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->tag_count = SENSORHUB_TAG_NUM_MAX;
	for (i = SENSORHUB_TYPE_BEGIN; i < SENSORHUB_TYPE_END; i++) {
		info = get_app_link_info(i);
		if (info && info->used_sensor_cnt > 0 && info->used_sensor_cnt <= SENSORHUB_TAG_NUM_MAX) {
			transmit->data[count].sensorhub_type = info->hal_sensor_type;
			for (j = 0; j < info->used_sensor_cnt; j++) {
				transmit->data[count].data[j] = info->used_sensor[j];
			}
			count++;
		}
	}
	if (count == 0) {
		dubai_err("No data");
		ret = -EINVAL;
		goto exit;
	}
	transmit->size = count;
	log_entry->length = cal_log_entry_len(struct dubai_sensorhub_type_list,
				struct dubai_sensorhub_type_info, transmit->size);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		dubai_err("Failed to send sensorhub type list entry");

exit:
	free_buffered_log_entry(log_entry);

	return ret;
}

static int dubai_get_aod_duration(void __user *argp)
{
	int ret;
	uint64_t duration = 0;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_AOD_TIME_STATISTICS, &duration, sizeof(duration));
	if (ret < 0)
		return ret;

	if (copy_to_user(argp, &duration, sizeof(uint64_t)))
		return -EFAULT;

	return 0;
}

static int dubai_get_fp_icon_stats(void __user *argp)
{
	int ret;
	uint32_t fp_icon_stats = 0;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_FINGERPRINT_ICON_COUNT, &fp_icon_stats, sizeof(fp_icon_stats));
	if (ret < 0)
		return ret;

	if (copy_to_user(argp, &fp_icon_stats, sizeof(fp_icon_stats)))
		return -EFAULT;

	return 0;
}

static int dubai_get_all_sensor_stats(void __user *argp)
{
	int ret;
	size_t count, packets, i;
	uint32_t stats = 0;
	uint8_t *sensor_data = NULL;
	struct sensor_time_transmit transmit;
	int32_t read_len, remain_len;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_ALL_SENSOR_STATISTICS, &stats, sizeof(stats));
	if (ret < 0)
		return ret;

	count = (size_t)(stats & 0xFF);
	count = (count <= PHYSICAL_SENSOR_TYPE_MAX) ? count : PHYSICAL_SENSOR_TYPE_MAX;
	packets = (count * sizeof(struct sensor_time) + MAX_PKT_LENGTH - 1) / MAX_PKT_LENGTH;
	remain_len = count * sizeof(struct sensor_time);
	sensor_data = (uint8_t *)(transmit.data);
	for (i = 0; i < packets && remain_len > 0; i++) {
		read_len = (remain_len < MAX_PKT_LENGTH) ? remain_len : MAX_PKT_LENGTH;
		ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_ALL_SENSOR_TIME, sensor_data, read_len);
		if (ret < 0)
			return ret;

		sensor_data += read_len;
		remain_len -= read_len;
	}
	transmit.count = (int)count;

	if (copy_to_user(argp, &transmit, sizeof(struct sensor_time_transmit)))
		return -EFAULT;

	return 0;
}

static int dubai_get_swing_data(void __user *argp)
{
	int ret;
	struct swing_data data;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_SWING, &data, sizeof(struct swing_data));
	if (ret < 0)
		return ret;

	if (copy_to_user(argp, &data, sizeof(struct swing_data)))
		return -EFAULT;

	return 0;
}

static int dubai_get_tp_duration(void __user *argp)
{
	int ret;
	struct tp_duration stat;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_TP, &stat, sizeof(struct tp_duration));
	if (ret < 0)
		return ret;

	if (copy_to_user(argp, &stat, sizeof(struct tp_duration)))
		return -EFAULT;

	return 0;
}

static int dubai_get_lcd_state_duration(void __user *argp)
{
	int ret;
	struct lcd_state_duration stat;

	ret = dubai_fetch_sensorhub_data(DUBAI_EVENT_LCD_STATUS_TIMES,
		&stat, sizeof(struct lcd_state_duration));
	if (ret < 0)
		return ret;
	if (copy_to_user(argp, &stat, sizeof(struct lcd_state_duration)))
		return -EFAULT;

	return 0;
}

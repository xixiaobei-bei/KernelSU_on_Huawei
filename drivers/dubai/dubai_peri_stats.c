#include "dubai_peri_stats.h"

#include <linux/errno.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/dubai_utils.h"

#define IOC_GET_PERI_VOLT_CNT DUBAI_PERI_DIR_IOC(R, 1, int)
#define IOC_GET_PERI_VOLT_TIME DUBAI_PERI_DIR_IOC(R, 2, struct dev_transmit_t)
#define IOC_GET_PERI_IP_CNT DUBAI_PERI_DIR_IOC(R, 3, int)
#define IOC_GET_PERI_IP_STATS DUBAI_PERI_DIR_IOC(R, 4, struct dev_transmit_t)

static int nr_volt = -1;
static int nr_ip = -1;
static struct dubai_peri_volt_time curr_volt_times[PERI_VOLT_CNT_MAX];
static struct dubai_peri_volt_time last_volt_times[PERI_VOLT_CNT_MAX];
static struct dubai_peri_ip_stats curr_ip_stats[PERI_IP_CNT_MAX];
static struct dubai_peri_ip_stats last_ip_stats[PERI_IP_CNT_MAX];
static struct dubai_peri_stats_ops *stats_op = NULL;

static inline uint64_t dubai_get_delta_value(uint64_t curr, uint64_t last)
{
	return (curr >= last) ? (curr - last) : curr;
}

static bool dubai_peri_check_stats(void)
{
	int32_t ret;
	static bool inited = false;

	if (inited) {
		return true;
	}

	if (!stats_op || !stats_op->get_volt_cnt || !stats_op->get_ip_cnt ||
		!stats_op->get_volt_time || !stats_op->get_ip_stats) {
		dubai_err("Invalid stats operations");
		return false;
	}

	nr_volt = stats_op->get_volt_cnt();
	nr_ip = stats_op->get_ip_cnt();
	if ((nr_volt <= 0) || (nr_ip <= 0)) {
		dubai_err("Invalid volt or ip cnt, volt cnt: %d, ip cnt: %d", nr_volt, nr_ip);
		return false;
	}

	ret = stats_op->get_volt_time(PERI_VOLT_CNT_MAX, last_volt_times);
	if (ret < 0) {
		dubai_err("Failed to get peri volt times");
		return false;
	}

	ret = stats_op->get_ip_stats(PERI_IP_CNT_MAX, last_ip_stats);
	if (ret < 0) {
		dubai_err("Failed to get peri ip stats");
		return false;
	}

	inited = true;

	return true;
}

static int dubai_peri_get_volt_time(void __user *argp)
{
	int i, ret, data_size;
	struct dev_transmit_t *transmit = NULL;
	struct dubai_peri_volt_time *delta = NULL;

	if (!dubai_peri_check_stats()) {
		dubai_err("Failed to check peri stats");
		return -1;
	}

	data_size = sizeof(struct dubai_peri_volt_time) * nr_volt;
	transmit = dubai_alloc_transmit(data_size);
	if (transmit == NULL)
		return -ENOMEM;
	delta = (struct dubai_peri_volt_time *)transmit->data;
	ret = stats_op->get_volt_time(PERI_VOLT_CNT_MAX, curr_volt_times);
	if (ret < 0) {
		dubai_err("Failed to get peri volt times %d", ret);
		ret = -EFAULT;
		goto EXIT;
	}
	for (i = 0; i < nr_volt; i++) {
		delta->volt = curr_volt_times[i].volt;
		delta->time = dubai_get_delta_value(curr_volt_times[i].time, last_volt_times[i].time);
	}
	memcpy(last_volt_times, curr_volt_times, sizeof(struct dubai_peri_volt_time) * PERI_VOLT_CNT_MAX);
	memset(curr_volt_times, 0, sizeof(curr_volt_times));

	if (copy_to_user(argp, transmit, dubai_get_transmit_size(transmit)))
		ret = -EFAULT;

EXIT:
	dubai_free_transmit(transmit);

	return ret;
}

static int dubai_peri_get_ip_stats(void __user *argp)
{
	int i, j, ret, data_size;
	struct dev_transmit_t *transmit = NULL;
	struct dubai_peri_ip_stats *delta = NULL;
	struct dubai_peri_ip_volt_time *last = NULL;
	struct dubai_peri_ip_volt_time *curr = NULL;

	if (!dubai_peri_check_stats()) {
		dubai_err("Failed to check peri stats");
		return -1;
	}

	data_size = sizeof(struct dubai_peri_ip_stats) * nr_ip;
	transmit = dubai_alloc_transmit(data_size);
	if (transmit == NULL)
		return -ENOMEM;
	delta = (struct dubai_peri_ip_stats *)transmit->data;
	ret = stats_op->get_ip_stats(PERI_IP_CNT_MAX, curr_ip_stats);
	if (ret < 0) {
		dubai_err("Failed to get peri ip stats %d", ret);
		ret = -EFAULT;
		goto EXIT;
	}

	for (i = 0; i < nr_ip; i++) {
		strncpy(delta->ip, curr_ip_stats[i].ip, PERI_IP_NAME_LEN - 1);
		for (j = 0; j < nr_volt; j++) {
			last = &last_ip_stats[i].volt_time[j];
			curr = &curr_ip_stats[i].volt_time[j];
			delta->volt_time[j].volt = dubai_get_delta_value(curr->volt, last->volt);
			delta->volt_time[j].active_time = dubai_get_delta_value(curr->active_time, last->active_time);
			delta->volt_time[j].idle_time = dubai_get_delta_value(curr->idle_time, last->idle_time);
			delta->volt_time[j].off_time = dubai_get_delta_value(curr->off_time, last->off_time);
		}
		delta++;
	}
	memcpy(last_ip_stats, curr_ip_stats, sizeof(struct dubai_peri_ip_stats) * PERI_IP_CNT_MAX);
	memset(curr_ip_stats, 0, sizeof(curr_ip_stats));

	if (copy_to_user(argp, transmit, dubai_get_transmit_size(transmit)))
		ret = -EFAULT;

EXIT:
	dubai_free_transmit(transmit);

	return ret;
}

static int dubai_get_volt_cnt(void __user *argp)
{
	if (!dubai_peri_check_stats()) {
		dubai_err("Failed to check peri stats");
		return -1;
	}

	if (copy_to_user(argp, &nr_volt, sizeof(nr_volt))) {
		return -EFAULT;
	}

	return 0;
}

static int dubai_get_ip_cnt(void __user *argp)
{
	if (!dubai_peri_check_stats()) {
		dubai_err("Failed to check peri stats");
		return -1;
	}

	if (copy_to_user(argp, &nr_ip, sizeof(nr_ip))) {
		return -EFAULT;
	}

	return 0;
}

long dubai_ioctl_peri(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_GET_PERI_VOLT_CNT:
		rc = dubai_get_volt_cnt(argp);
		break;
	case IOC_GET_PERI_VOLT_TIME:
		rc = dubai_peri_get_volt_time(argp);
		break;
	case IOC_GET_PERI_IP_CNT:
		rc = dubai_get_ip_cnt(argp);
		break;
	case IOC_GET_PERI_IP_STATS:
		rc = dubai_peri_get_ip_stats(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void dubai_reset_peri_stats(void)
{
	stats_op = NULL;
}

int dubai_peri_register_ops(struct dubai_peri_stats_ops *op)
{
	if (!stats_op)
		stats_op = op;
	return 0;
}

int dubai_peri_unregister_ops(void)
{
	if (stats_op)
		stats_op = NULL;
	return 0;
}

void dubai_peri_stats_init(void)
{
	memset(curr_volt_times, 0, sizeof(curr_volt_times));
	memset(last_volt_times, 0, sizeof(last_volt_times));
	memset(curr_ip_stats, 0, sizeof(curr_ip_stats));
	memset(last_ip_stats, 0, sizeof(last_ip_stats));
}

void dubai_peri_stats_exit(void)
{
	dubai_reset_peri_stats();
}

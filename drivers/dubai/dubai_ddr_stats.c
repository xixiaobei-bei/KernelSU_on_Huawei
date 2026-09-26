#include "dubai_ddr_stats.h"

#include <linux/errno.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/dubai_utils.h"

#define IOC_GET_FREQ_CNT DUBAI_DDR_DIR_IOC(R, 1, int)
#define IOC_GET_TIME_IN_STATE DUBAI_DDR_DIR_IOC(R, 2, struct dev_transmit_t)
#define IOC_GET_IP_CNT DUBAI_DDR_DIR_IOC(R, 3, int)
#define IOC_GET_IP_STATS DUBAI_DDR_DIR_IOC(R, 4, struct dev_transmit_t)

static int nr_freq = -1;
static int nr_ip = -1;
static struct dubai_ddr_time_in_state curr_time_in_state;
static struct dubai_ddr_time_in_state last_time_in_state;
static struct dubai_ddr_ip_stats curr_ip_stats[DDR_IP_CNT_MAX];
static struct dubai_ddr_ip_stats last_ip_stats[DDR_IP_CNT_MAX];
static struct dubai_ddr_stats_ops *stats_op = NULL;

static bool dubai_ddr_check_stats(void)
{
	int32_t ret;
	static bool inited = false;

	if (inited) {
		return true;
	}

	if (!stats_op || !stats_op->get_freq_cnt || !stats_op->get_ip_cnt ||
		!stats_op->get_time_in_state || !stats_op->get_ip_stats) {
		dubai_err("Invalid stats operations");
		return false;
	}

	nr_freq = stats_op->get_freq_cnt();
	nr_ip = stats_op->get_ip_cnt();
	if ((nr_freq <= 0) || (nr_ip <= 0)) {
		dubai_err("Invalid freq or ip cnt, freq cnt: %d, ip cnt: %d", nr_freq, nr_ip);
		return false;
	}

	ret = stats_op->get_time_in_state(&last_time_in_state);
	if (ret < 0) {
		dubai_err("Failed to get ddr time in state %d", ret);
		return false;
	}

	ret = stats_op->get_ip_stats(DDR_IP_CNT_MAX, last_ip_stats);
	if (ret < 0) {
		dubai_err("Failed to get ddr ip stats %d", ret);
		return false;
	}

	inited = true;

	return true;
}

static int dubai_ddr_get_freq_cnt(void __user *argp)
{
	if (!dubai_ddr_check_stats()) {
		dubai_err("Failed to check ddr stats");
		return -1;
	}

	if (copy_to_user(argp, &nr_freq, sizeof(nr_freq))) {
		return -EFAULT;
	}

	return 0;
}

static int dubai_ddr_get_ip_cnt(void __user *argp)
{
	if (!dubai_ddr_check_stats()) {
		dubai_err("Failed to check ddr stats");
		return -1;
	}

	if (copy_to_user(argp, &nr_ip, sizeof(nr_ip))) {
		return -EFAULT;
	}

	return 0;
}

static inline uint64_t dubai_get_delta_value(uint64_t curr, uint64_t last)
{
	return (curr >= last) ? (curr - last) : curr;
}

static int dubai_ddr_get_time_in_state(void __user *argp)
{
	int i, ret, data_size;
	struct dev_transmit_t *transmit = NULL;
	struct dubai_ddr_time_in_state *delta = NULL;

	if (!dubai_ddr_check_stats()) {
		dubai_err("Failed to check ddr stats");
		return -1;
	}

	data_size = sizeof(struct dubai_ddr_time_in_state);
	transmit = dubai_alloc_transmit(data_size);
	if (transmit == NULL)
		return -ENOMEM;
	delta = (struct dubai_ddr_time_in_state *)transmit->data;
	ret = stats_op->get_time_in_state(&curr_time_in_state);
	if (ret < 0) {
		dubai_err("Failed to get ddr time in state %d", ret);
		goto EXIT;
	}
	delta->sr_time = dubai_get_delta_value(curr_time_in_state.sr_time, last_time_in_state.sr_time);
	delta->pd_time = dubai_get_delta_value(curr_time_in_state.pd_time, last_time_in_state.pd_time);
	for (i = 0; i < nr_freq; i++) {
		delta->freq_time[i].freq = curr_time_in_state.freq_time[i].freq;
		delta->freq_time[i].time = dubai_get_delta_value(curr_time_in_state.freq_time[i].time,
			last_time_in_state.freq_time[i].time);
	}
	memcpy(&last_time_in_state, &curr_time_in_state,
		sizeof(struct dubai_ddr_time_in_state));
	memset(&curr_time_in_state, 0, sizeof(curr_time_in_state));
	if (copy_to_user(argp, transmit, dubai_get_transmit_size(transmit)))
		ret = -EFAULT;

EXIT:
	dubai_free_transmit(transmit);

	return ret;
}

static int dubai_ddr_get_ip_stats(void __user *argp)
{
	int i, j, ret, data_size;
	struct dev_transmit_t *transmit = NULL;
	struct dubai_ddr_ip_stats *delta = NULL;

	if (!dubai_ddr_check_stats()) {
		dubai_err("Failed to check ddr stats");
		return -1;
	}

	data_size = sizeof(struct dubai_ddr_ip_stats) * nr_ip;
	transmit = dubai_alloc_transmit(data_size);
	if (transmit == NULL)
		return -ENOMEM;
	delta = (struct dubai_ddr_ip_stats *)transmit->data;
	ret = stats_op->get_ip_stats(DDR_IP_CNT_MAX, curr_ip_stats);
	if (ret < 0) {
		dubai_err("Failed to get ddr ip stats %d", ret);
		goto EXIT;
	}
	for (i = 0; i < nr_ip; i++) {
		strncpy(delta->ip, curr_ip_stats[i].ip, DDR_IP_NAME_LEN - 1);
		for (j = 0; j < nr_freq; j++) {
			delta->freq_data[j].freq = curr_ip_stats[i].freq_data[j].freq;
			delta->freq_data[j].data = dubai_get_delta_value(curr_ip_stats[i].freq_data[j].data,
				last_ip_stats[i].freq_data[j].data);
		}
		delta++;
	}
	memcpy(last_ip_stats, curr_ip_stats, sizeof(struct dubai_ddr_ip_stats) * DDR_IP_CNT_MAX);
	memset(curr_ip_stats, 0, sizeof(curr_ip_stats));

	if (copy_to_user(argp, transmit, dubai_get_transmit_size(transmit)))
		ret = -EFAULT;

EXIT:
	dubai_free_transmit(transmit);

	return ret;
}

long dubai_ioctl_ddr(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_GET_FREQ_CNT:
		rc = dubai_ddr_get_freq_cnt(argp);
		break;
	case IOC_GET_TIME_IN_STATE:
		rc = dubai_ddr_get_time_in_state(argp);
		break;
	case IOC_GET_IP_CNT:
		rc = dubai_ddr_get_ip_cnt(argp);
		break;
	case IOC_GET_IP_STATS:
		rc = dubai_ddr_get_ip_stats(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void dubai_reset_ddr_stats(void)
{
	stats_op = NULL;
}

int dubai_ddr_register_ops(struct dubai_ddr_stats_ops *op)
{
	if (!stats_op)
		stats_op = op;
	return 0;
}

int dubai_ddr_unregister_ops(void)
{
	if (stats_op)
		stats_op = NULL;
	return 0;
}

void dubai_ddr_stats_init(void)
{
	memset(&curr_time_in_state, 0, sizeof(curr_time_in_state));
	memset(&last_time_in_state, 0, sizeof(last_time_in_state));
	memset(curr_ip_stats, 0, sizeof(curr_ip_stats));
	memset(last_ip_stats, 0, sizeof(last_ip_stats));
}

void dubai_ddr_stats_exit(void)
{
	dubai_reset_ddr_stats();
}

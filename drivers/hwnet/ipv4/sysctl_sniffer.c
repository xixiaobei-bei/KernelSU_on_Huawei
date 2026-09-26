

#include "sysctl_sniffer.h"
#include <linux/mutex.h>

#ifdef CONFIG_HW_SNIFFER

#define LARGE_AMSDU_FRAME_THRESHOLD 1700
#define SMALL_AMSDU_FRAME_THRESHOLD 30
#define CHANNEL_CATEGORY_ARRAY_DIMENSION_ONE_NUM 4
#define CHANNEL_CATEGORY_ARRAY_DIMENSION_TWO_NUM 7
#define CHANNEL_NUM_5 5
#define CHANNEL_NUM_9 9
#define TX_DISASSOC_LENGTH 26
#define MAX_PARAM_LEN 5000

proc_file_config_struct g_configs = {0, TRIM_DATA_PAYLOAD_LEN,
	PCAP_FILE_NUM, PCAP_FILE_LEN, {0}, {NULL}, {0}};
static DEFINE_SPINLOCK(proc_wrlock);
int g_sniffer_mode = 0;
int g_wifi_chip_type = CHIPTYPE_1103; // default is 1103
int g_ofdma_aid = 0;

static DEFINE_MUTEX(handler_mutex);

drive_wal_hipriv_cmd_func g_hipriv_func = NULL;

struct static_key wifi_sniffer_enable __read_mostly = STATIC_KEY_INIT_FALSE;

static void proc_set_cur_file_index_to_first(void)
{
	g_configs.cur_file = 0;
}

static void proc_init_file_content(int file_index, int file_len)
{
	memset(g_configs.buffer[file_index], 0, file_len);
	g_configs.curpos[file_index] = 0;
	g_configs.file_full[file_index] = 0;
	g_configs.gmttime[file_index] = 0;
	g_configs.microtime[file_index] = 0;
	g_configs.packetcount[file_index] = 0;
}

static int proc_sniffer_reset_buff(int old_file_num, int new_file_num,
	int new_file_len)
{
	int i;

	for (i = 0; i < old_file_num; i++) {
		if (sniffer_file_buffer(i)) {
			vfree(sniffer_file_buffer(i));
			sniffer_file_buffer(i) = NULL;
		}
	}

	for (i = 0; i < new_file_num; i++) {
		sniffer_file_buffer(i) = (char *)vmalloc(new_file_len);
		if (!sniffer_file_buffer(i))
			return -ENOMEM;

		proc_init_file_content(i, new_file_len);
	}

	proc_set_cur_file_index_to_first();
	return 0;
}

static void proc_sniffer_init_buff(void)
{
	if (proc_sniffer_reset_buff(g_configs.max_file_num,
		g_configs.max_file_num, g_configs.max_file_len))
		printk(KERN_ERR "%s ERROR : sniffer_reset error", __func__);
}

static void proc_cur_file_move_to_next(void)
{
	g_configs.file_full[g_configs.cur_file] = 1;
	g_configs.cur_file = (g_configs.cur_file + 1) % g_configs.max_file_num;
}

static void proc_init_file_to_save_packet(unsigned int packet_len)
{
	/*
	 * current file is not full,
	 * and will be full if write one more packet
	 */
	if ((!SNIFFER_IS_CUR_FULL && !sniffer_enough_to_save(packet_len)) ||
		SNIFFER_IS_CUR_FULL) {
		proc_cur_file_move_to_next();
		proc_init_file_content(g_configs.cur_file,
			g_configs.max_file_len);
	}
}

static void proc_increase_cur_file_pos(int len)
{
	g_configs.curpos[g_configs.cur_file] += len;
}

static int proc_write_pcap_hdr_for_empty_file(int flag_rx_tx)
{
	oam_pcap_filehdr_stru *pcap_filehdr =
		(oam_pcap_filehdr_stru *)SNIFFER_CUR_FILE_BUFFER;

	if (!SNIFFER_IS_CUR_FILE_EMPTY)
		return 0;
	pcap_filehdr->magic = PCAP_PKT_HDR_MAGIC;
	pcap_filehdr->version_major = PCAP_PKT_HDR_VER_MAJ;
	pcap_filehdr->version_minor = PCAP_PKT_HDR_VER_MIN;
	pcap_filehdr->thiszone = PCAP_PKT_HDR_THISZONE;
	pcap_filehdr->sigfigs = PCAP_PKT_HDR_SIGFIGS;
	pcap_filehdr->snaplen = PCAP_PKT_HDR_SNAPLEN;
	if (flag_rx_tx != DIRECTION_MONITOR_MODE) {
		printk(KERN_ERR "{%s:%d} ERROR : without radiotap flag_rx_tx = %d\n", __func__, __LINE__, flag_rx_tx);
		pcap_filehdr->linktype = PCAP_PKT_HDR_LINKTYPE_802_11;
	} else {
		pcap_filehdr->linktype = PCAP_PKT_HDR_LINKTYPE_802_11_RADIOTAP;
	}

	proc_increase_cur_file_pos(sizeof(oam_pcap_filehdr_stru));
	return sizeof(oam_pcap_filehdr_stru);
}

static void proc_update_packet_pcap_header(char *file_buffer, int packet_len, int orig_len)
{
	struct timeval tv;
	oam_pcap_pkthdr_stru st_pcap_pkthdr = { { 0, 0 }, 0, 0 };

	do_gettimeofday(&tv);
	st_pcap_pkthdr.time.gmt_time = tv.tv_sec;
	st_pcap_pkthdr.time.micro_time = tv.tv_usec;
	st_pcap_pkthdr.caplen = packet_len;
	st_pcap_pkthdr.len = orig_len;

	memcpy(file_buffer, (char *)&st_pcap_pkthdr,
		sizeof(oam_pcap_pkthdr_stru));
}

static int solve_packet_filter(const st_packet_filter *packet_filter,
	int loop, const char *p_llc_hdr)
{
	int port_loop;
	unsigned short ipv4_src_port = get_ipv4_src_port(p_llc_hdr);
	unsigned short ipv4_dst_port = get_ipv4_dst_port(p_llc_hdr);
	for (port_loop = 0; port_loop < packet_filter[loop].port_count;
		port_loop++) {
		if (ipv4_src_port == (ipv4_src_port &
			get_filter_sport(loop, port_loop)) &&
			ipv4_dst_port == (ipv4_dst_port &
			get_filter_dport(loop, port_loop)))
			return TRIM_NOT_NEEDED;
	}
	return TRIM_NEEDED;
}

/* filter ARP/DHCP/DNS/EAP/ICMP for packet trimming */
static int proc_packet_get_trim_flag(const char *p_llc_hdr)
{
	int loop;
	/*lint -e446*/
	const st_packet_filter packet_filter[] = {
		{ETH_PKT_TYPE_IPV4, ETH_PKT_TYPE_IPV4_UDP, 4,
		{{ETH_PKT_TYPE_IPV4_UDP_DHCP_A, ETH_PKT_TYPE_IPV4_UDP_DHCP_B},
		{ETH_PKT_TYPE_IPV4_UDP_DHCP_B, ETH_PKT_TYPE_IPV4_UDP_DHCP_A},
		{ETH_PKT_TYPE_IPV4_UDP_DNS, PORT_ANY},
		{PORT_ANY, ETH_PKT_TYPE_IPV4_UDP_DNS},
		{PORT_ANY, PORT_ANY}}},
		{ETH_PKT_TYPE_IPV4, ETH_PKT_TYPE_IPV4_ICMP, 0,
		{{PORT_ANY, PORT_ANY}}},
		{ETH_PKT_TYPE_ARP, PROTOCOL_ANY, 0, {{PORT_ANY, PORT_ANY}}},
		{ETH_PKT_TYPE_EAP, PROTOCOL_ANY, 0, {{PORT_ANY, PORT_ANY}}}
	};
	/*lint +e446*/
	const int types_count =
		sizeof(packet_filter) / sizeof(packet_filter[0]);

	if (get_llc_type(p_llc_hdr) == ETH_PKT_TYPE_IPV6)
		return TRIM_NOT_NEEDED;

	for (loop = 0; loop < types_count; loop++) {
		if (packet_filter[loop].llc_type == get_llc_type(p_llc_hdr) &&
			get_ipv4_protocol_type(p_llc_hdr) ==
			(packet_filter[loop].protocol_type &
			get_ipv4_protocol_type(p_llc_hdr))) {
			if (packet_filter[loop].port_count == 0)
				return TRIM_NOT_NEEDED;

			if (solve_packet_filter(packet_filter, loop,
				p_llc_hdr) == TRIM_NOT_NEEDED)
				return TRIM_NOT_NEEDED;
		}
	}
	return TRIM_NEEDED;
}

static unsigned int proc_get_trimed_packet_len_monitor(const char *frame_buff,
	unsigned int frame_len, unsigned int packet_len_trim)
{
	unsigned int real_len = frame_len;
	mac_header_control_stru *p_frame_type = NULL;
	p_frame_type = (mac_header_control_stru *)frame_buff;
	if (IEEE80211_FRAME_TYPE_DATA == p_frame_type->bit_type) {
		if (packet_len_trim + sizeof(mac_ieee80211_qos_stru) < frame_len)
			real_len = packet_len_trim + sizeof(mac_ieee80211_qos_stru);
	}
	return real_len;
}

static int proc_trim_packet_len_hybrid(const char *header_buff, int *p_header_len,
	const char *frame_buff, unsigned int *p_frame_len, unsigned int packet_len_trim)
{
	int trim_flag;
	char *p_llc_hdr = NULL;
	mac_header_control_stru *p_frame_type = NULL;

	if (header_buff == NULL) {
		*p_header_len = 0;
		p_frame_type = (mac_header_control_stru *)frame_buff;
		p_llc_hdr = (char *)(frame_buff + IEEE80211_MAC_HDR_LEN);
	} else {
		p_frame_type = (mac_header_control_stru *)header_buff;
		p_llc_hdr = (char *)frame_buff;
	}

	if (p_frame_type->bit_type == IEEE80211_FRAME_TYPE_DATA) {
		trim_flag = proc_packet_get_trim_flag(p_llc_hdr);
		if (trim_flag == TRIM_NOT_NEEDED)
			return p_frame_type->bit_type;

		if ((*p_frame_len + *p_header_len) > (packet_len_trim +
			IEEE80211_MAC_HDR_LEN) && (*p_header_len < (packet_len_trim +
			IEEE80211_MAC_HDR_LEN)))
			*p_frame_len = packet_len_trim +
				IEEE80211_MAC_HDR_LEN - *p_header_len;
	}
	return p_frame_type->bit_type;
}

static void proc_set_rx_packet_to_no_proection(int flag_rx_tx, int frame_type,
	char *dst_buff)
{
	if ((flag_rx_tx == DIRECTION_RX) &&
		(frame_type == IEEE80211_FRAME_TYPE_DATA))
		((mac_header_control_stru *)dst_buff)->bit_protected_frame = 0;

	if ((DIRECTION_TX == flag_rx_tx) &&
		(IEEE80211_FRAME_TYPE_DATA == frame_type) &&
		(((mac_header_control_stru *)dst_buff)->bit_from_ds == 1)) {
		((mac_header_control_stru *)dst_buff)->bit_sub_type = IEEE80211_FRAME_SUBTYPE_QOS;
		// disable amsdu bit; what about NON-QOS data, or 4-address data
		((mac_ieee80211_qos_stru *)dst_buff)->bit_qc_amsdu = 0;
	}
}

static void proc_write_packet_to_file(char *dst_buff, const char *header_buff,
	int header_len, const char *frame_buff, int frame_len, int flag_rx_tx, int orig_len)
{
	proc_update_packet_pcap_header(dst_buff, frame_len + header_len, orig_len + header_len);
	if (header_buff != NULL)
		memcpy(dst_buff + sizeof(oam_pcap_pkthdr_stru), header_buff,
			header_len);

	memcpy(dst_buff + sizeof(oam_pcap_pkthdr_stru) + header_len, frame_buff,
		frame_len);
}

static void record_cur_file_time(const oam_pcap_pkthdr_stru *packet_header)
{
	g_configs.gmttime[g_configs.cur_file] = packet_header->time.gmt_time;
	g_configs.microtime[g_configs.cur_file] = packet_header->time.micro_time;
}

int proc_sniffer_write_file(const char *header_buff, unsigned int header_len,
	const char *frame_buff, unsigned int frame_len, int flag_rx_tx)
{
	char *dst_buff = NULL;
	unsigned long flag;
	int frame_type = 0;
	int orig_len;

	if (!SNIFFER_START)
		return 0;

	// to avoid non-radiotap packet in monitor mode sniffer
	if (g_sniffer_mode == CMD_ENABLE_MONITOR_MODE) {
		if (flag_rx_tx != DIRECTION_MONITOR_MODE)
			return 0;
	} else {
		if (flag_rx_tx == DIRECTION_MONITOR_MODE)
			return 0;
	}

	/* debugging the issue of not capturing tx disassoc */
	if (!frame_buff || (frame_len + header_len) < TX_DISASSOC_LENGTH) {
		printk(KERN_ERR "wrong (frame_len + header_len)(%d) < 26 \n", (frame_len + header_len));
		return -1;
	}

	/* what if a large AMSDU frame? */
	if ((flag_rx_tx != DIRECTION_MONITOR_MODE) &&
		((frame_len + header_len) >= 5000)) {
		printk(KERN_ERR "wrong (frame_len + header_len)(%d) >=5000 \n",
			(frame_len + header_len));
		return -1;
	}

	spin_lock_irqsave(&proc_wrlock, flag);
	if (!SNIFFER_CUR_FILE_BUFFER)
		proc_sniffer_init_buff();

	orig_len = frame_len;
	if (flag_rx_tx != DIRECTION_MONITOR_MODE) {
		frame_type = proc_trim_packet_len_hybrid(header_buff, &header_len, frame_buff,
			&frame_len, SNIFFER_MAX_PACKET_LEN);
	} else {
		// DEBUG: temp not trim length temperially for full mode
		frame_len = proc_get_trimed_packet_len_monitor(frame_buff, frame_len, SNIFFER_MAX_PACKET_LEN);
	}

	proc_init_file_to_save_packet(sizeof(oam_pcap_pkthdr_stru) +
		frame_len + header_len);
	proc_write_pcap_hdr_for_empty_file(flag_rx_tx);

	dst_buff = SNIFFER_CUR_FILE_BUFF_POS;
	proc_increase_cur_file_pos(sizeof(oam_pcap_pkthdr_stru) +
		frame_len + header_len);
	proc_write_packet_to_file(dst_buff, header_buff, header_len,
		frame_buff, frame_len, flag_rx_tx, orig_len);
	record_cur_file_time((oam_pcap_pkthdr_stru *)dst_buff);
	g_configs.packetcount[g_configs.cur_file] += 1;

	// this is to fix some bugs in hybrid sniffer mode
	if (flag_rx_tx != DIRECTION_MONITOR_MODE)
		proc_set_rx_packet_to_no_proection(flag_rx_tx, frame_type, dst_buff + sizeof(oam_pcap_pkthdr_stru));

	spin_unlock_irqrestore(&proc_wrlock, flag);

	return (sizeof(oam_pcap_pkthdr_stru) + frame_len + header_len);
}

int proc_sniffer_read(int file_index, int write, char __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	size_t len;
	char *file_data = NULL;
	int file_pos;
	ctl_cmd_stru cmd2_parse;
	/* solve the issue of rebooting after get_proc.bat when PCAP_FILE_NUM < 5 */
	/* file_index = 0,1,2,3,4 */
	/* parameter validation check */
	if (file_index < 0 || file_index >= PCAP_FILE_NUM ||
		buffer == NULL || lenp == NULL || ppos == NULL) {
		if (lenp != NULL)
			*lenp = 0;
		return 0;
	}
	file_data = sniffer_file_buffer(file_index);
	file_pos = sniffer_file_pos(file_index);
	if (sizeof(ctl_cmd_hdr_stru) <= *lenp) {
		if (copy_from_user(&cmd2_parse, buffer, sizeof(ctl_cmd_stru)))
			return -EFAULT;
		if (cmd2_parse.ctl_cmd_hdr.i_magic_num == CMD_BUFF_MAGIC_NUM)
			return proc_handle_commands(file_index, buffer);
	}

	if (file_data == NULL || !*lenp) {
		*lenp = 0;
		return 0;
	}

	if (write)
		return 0;

	/* can not read data behind the current file_content_pos */
	if (*ppos >= file_pos) {
		*lenp = 0;
		printk(KERN_ERR "{%s:%d} ERROR:Invalid ppos parameter:0x%x, current file position:0x%x\n",
			__FUNCTION__, __LINE__, *(unsigned int *)ppos, file_pos);
		return 0;
	}

	len = file_pos - *ppos;
	if (len > *lenp)
		len = *lenp;

	if (copy_to_user(buffer, file_data + *ppos, len))
		return -EFAULT;

	*lenp = len;
	*ppos += len;

	return 0;
}

static int get_channel_type(int channel, char channel_type)
{
	int i_loop, j_loop;
	int channel_category[CHANNEL_CATEGORY_ARRAY_DIMENSION_ONE_NUM][CHANNEL_CATEGORY_ARRAY_DIMENSION_TWO_NUM] = {
		{36, 52, 100, 116, 132, 149, 184},
		{40, 56, 104, 120, 136, 153, 188},
		{44, 60, 108, 124, 140, 157, 192},
		{48, 64, 112, 128, 144, 161, 196}
	};
	for (i_loop = 0; i_loop < CHANNEL_CATEGORY_ARRAY_DIMENSION_ONE_NUM; i_loop++) {
		for (j_loop = 0; j_loop < CHANNEL_CATEGORY_ARRAY_DIMENSION_TWO_NUM; j_loop++) {
			if (channel_category[i_loop][j_loop] != channel)
				continue;
			if (channel_type == 'W' || channel_type == 'w')
				return i_loop + (j_loop % 2) * 4; // 4:20/40/80/160MHz
			else
				return i_loop;
		}
	}
	return 0;
}

static char *get_mode_string(const int expand_type, const int chip_type)
{
	switch (expand_type) { // 2.4G expand
	case EXPAND_LOW:
		if (chip_type == CHIPTYPE_1105) {
			return "11ax2g40minus";
		} else {
			return "11ac2g40minus";
		}
	case EXPAND_HIGH:
		if (chip_type == CHIPTYPE_1105) {
			return "11ax2g40plus";
		} else {
			return "11ac2g40plus";
		}
	case EXPAND_11B:
		return "11b";
	default:
	case EXPAND_NONE:
		if (chip_type == CHIPTYPE_1105) {
			return "11ax2g20";
		} else {
			return "11ac2g20";
		}
	}
}

static int is_only_support_20(const int expand_type, const int channel)
{
	// not allowed for 40Mhz
	if (expand_type == EXPAND_LOW && channel < CHANNEL_NUM_5)
		return true;

	if (expand_type == EXPAND_HIGH && channel > CHANNEL_NUM_9)
		return true;

	if (expand_type == EXPAND_NONE)
		return true;

	return false;
}

static int get_expand_type(const char channel_type, const int channel)
{
	int expand_type = EXPAND_NONE;
	switch (channel_type) {
	case 'L':
	case 'l':
		expand_type = EXPAND_LOW;
		break;
	case 'H':
	case 'h':
		expand_type = EXPAND_HIGH;
		break;
	case ' ':
	default:
		expand_type = EXPAND_NONE;
		break;
	}

	if (is_only_support_20(expand_type, channel))
		expand_type = EXPAND_NONE;

	if (channel >= 14)
		expand_type = EXPAND_11B;

	return expand_type;
}

static int convert_mode_cmd_str(char *cmd_str, const int channel,
	const char channel_type)
{
	char new_cmds[50] = {0};
	char * channel_str_5[] = {"11ac80plusplus", "11ac80plusminus", "11ac80minusplus", "11ac80minusminus"};
	char * channel_str160_5[] = {
		"11ac160plusplusplus",   "11ac160plusplusminus", "11ac160plusminusplus",
		"11ac160plusminusminus", "11ac160minusplusplus", "11ac160minusplusminus",
		"11ac160minusminusplus", "11ac160minusminusminus"
	};
	char * channel_str_6[] = {"11ax5g80plusplus", "11ax5g80plusminus", "11ax5g80minusplus", "11ax5g80minusminus"};
	char * channel_str160_6[] = {
		"11ax5g160plusplusplus",   "11ax5g160plusplusminus", "11ax5g160plusminusplus",
		"11ax5g160plusminusminus", "11ax5g160minusplusplus", "11ax5g160minusplusminus",
		"11ax5g160minusminusplus", "11ax5g160minusminusminus"
	};
	char **channel_str = channel_str_5;
	char **channel_str160 = channel_str160_5;

	if (channel < 36) {
		int expand_type = get_expand_type(channel_type, channel);
		snprintf(new_cmds, sizeof(new_cmds), cmd_str, get_mode_string(expand_type, g_wifi_chip_type));
	} else {
		if (g_wifi_chip_type == CHIPTYPE_1105) {
			channel_str = channel_str_6;
			channel_str160 = channel_str160_6;
		}

		if (channel == 165) { // ch#165 only support VHT20
			if (g_wifi_chip_type == CHIPTYPE_1105) {
				snprintf(new_cmds, sizeof(new_cmds), cmd_str, "11ax5g20");
			} else {
				snprintf(new_cmds, sizeof(new_cmds), cmd_str, "11ac20");
			}
		} else {
			if ((channel_type == 'W' || channel_type == 'w') && (channel <= 128)) // VHT160 support
				snprintf(new_cmds, sizeof(new_cmds), (const char *)cmd_str,
					channel_str160[get_channel_type(channel, channel_type) % 8]);
			else
				snprintf(new_cmds, sizeof(new_cmds), (const char *)cmd_str,
					channel_str[get_channel_type(channel, ' ') % 4]);
		}
	}

	strncpy(cmd_str, new_cmds, sizeof(new_cmds));
	return 0;
}

static int convert_freq_cmd_str(char *cmd_str, int channel)
{
	char new_cmds[50] = {0};
	snprintf(new_cmds, sizeof(new_cmds), cmd_str, channel);
	strncpy(cmd_str, new_cmds, sizeof(new_cmds));
	return 0;
}

static int convert_sniffer_cmd_str(char *cmd_str, const char *mac_filter)
{
	char new_cmds[50] = {0};
	snprintf(new_cmds, sizeof(new_cmds), cmd_str, mac_filter[0], mac_filter[1],
		mac_filter[2], mac_filter[3], mac_filter[4], mac_filter[5]);
	strncpy(cmd_str, new_cmds, sizeof(new_cmds));
	return 0;
}

static int execute_shell(const char *cmd_str)
{
	char * envp[] = { "HOME=/", "PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin", NULL};
	char **argv = NULL;
	char *argv1[] = { "/system/bin/ifconfig", "wlan0", "down", NULL };
	char *argv2[] = { "/system/bin/sh", "/system/lib/hipriv.sh", "'Hisilicon0 create wlan0 sta'", NULL };
	char *argv3[] = { "/system/bin/ifconfig", "wlan0", "hw", "ether", "00:E0:52:22:22:14", NULL };
	char *argv4[] = { "/system/bin/ifconfig", "wlan0", "up", NULL };

	// select the corrrect commands.
	if (strstr(cmd_str, "down") != NULL) {
		argv = argv1;
	} else if (strstr(cmd_str, "create") != NULL) {
		argv = argv2;
	} else if (strstr(cmd_str, "ether") != NULL) {
		argv = argv3;
	} else { // make sure argv is not NULL
		argv = argv4;
	}

	return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
}

/**
adb shell cd /system/lib
adb shell "cd /system/lib && sh hipriv.sh 'wlan0 mode 11ac2g20'"
REM adb shell "cd /system/lib && sh hipriv.sh 'wlan0 mode 11ng20'"
adb shell "cd /system/lib && sh hipriv.sh 'wlan0 freq 1'"

adb shell "cd /system/lib && sh hipriv.sh 'wlan0 sniffer 1 ff:ff:ff:ff:ff:ff'"

adb shell "cd /system/lib && sh hipriv.sh 'wlan0 al_rx 1'"
adb shell "cd /system/lib && sh hipriv.sh 'wlan0 phy_debug rssi 1'"

adb shell "cd /system/lib && sh hipriv.sh 'wlan0 pm_switch 0'"

adb shell "cd /system/lib && sh hipriv.sh 'wlan0 monitor 1'"
REM adb shell "cd /system/lib && sh hipriv.sh 'wlan0 regwrite mac 0x40080420 0x1'"

adb shell "cd /system/lib && sh hipriv.sh 'wlan0 rx_fcs_info 1 2'"

**/
static int set_monitor_mode_param(const int sniffer_mode, const int channel, const char channel_type,
	const char *mac_filter)
{
	int i_loop;
	proc_cmd_execution_struct cmds_off[] = {
		{"wlan0 mode %s", 100, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 freq %d", 100, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 al_rx 0", 50, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 pm_switch 1", 50, CMD_EXECUTE_TYPE_DRIVER}, // re-enable PS
		{"wlan0 monitor 0", 50, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 sniffer 0 ff:ff:ff:ff:ff:ff", 50, CMD_EXECUTE_TYPE_DRIVER}
	};

	proc_cmd_execution_struct cmds_on[] = {
		{"wlan0 mode %s", 100, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 freq %d", 100, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 sniffer 1 %02x:%02x:%02x:%02x:%02x:%02x", 50, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 al_rx 1", 50, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 pm_switch 0", 50, CMD_EXECUTE_TYPE_DRIVER},
		{"wlan0 monitor 1", 50, CMD_EXECUTE_TYPE_DRIVER}
	};

	int count = sizeof(cmds_on) / sizeof(cmds_on[0]);
	int count_off_mode = sizeof(cmds_off) / sizeof(cmds_off[0]);
	proc_cmd_execution_struct *cmds = cmds_on;

	if (sniffer_mode != CMD_ENABLE_MONITOR_MODE) {
		count = count_off_mode;
		cmds = cmds_off;
	}

	// update mode cmd by channel and ch_type
	convert_mode_cmd_str(cmds[CMD_MODE_INDEX].cmds_str, channel, channel_type);
	convert_freq_cmd_str(cmds[CMD_FREQ_INDEX].cmds_str, channel);
	convert_sniffer_cmd_str(cmds[CMD_MAC_INDEX].cmds_str, mac_filter);

	for (i_loop = 0; i_loop < count; i_loop++) {
		if (g_hipriv_func) {
			if (cmds[i_loop].execute_type == CMD_EXECUTE_TYPE_SHELL) {
				execute_shell(cmds[i_loop].cmds_str);
			} else {
				g_hipriv_func(cmds[i_loop].cmds_str);
			}
			msleep(cmds[i_loop].delay_time);
		} else {
			printk(KERN_ERR "ERROR : g_hipriv_func is NULL \n");
		}
	}
	printk(KERN_INFO "ERROR : {%s:%d}%d \n", __FUNCTION__, __LINE__, count);
	return 0;
}

// Need to support two schema, use 0xff to terminated to avoid 52=='4', 56=='8'
static int parse_channel_info(int *p_channel, char *p_ch_type, const char *p_param)
{
	int loop = 0;

	(*p_channel) = *((const int *)p_param);
	(*p_ch_type) = ' ';

	if (!(((*p_channel >= 1 && *p_channel <= 14) ||
		(*p_channel >= 36 && *p_channel <= 196)) &&
		p_param[1] == 0x00 && p_param[2] == 0x00)) {
		(*p_channel) = simple_strtol(p_param, NULL, 10);
		for (loop = 0; loop < 4; loop++) {
			// for the first non-digial char
			if (p_param[loop] < '0' || p_param[loop] > '9') {
				(*p_ch_type) = p_param[loop];
				break;
			}
		}
	}

	printk(KERN_INFO "{%s:%d}: channel:%d, p_ch_type:%c\n", __FUNCTION__, __LINE__, *p_channel, *p_ch_type);
	return 0;
}

static void proc_handle_command_sniffer_enable(ctl_cmd_stru *p_cmd2parse)
{
	// 0: close, 1:hybrid, 2:monitor
	int sniffer_mode = p_cmd2parse->i_paras[0];
	int channel = 36;
	char channel_type = ' ';
	char filter[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int mac_loop;

	/* parameter validation check */
	if ((sniffer_mode < 0) || (sniffer_mode > CMD_ENABLE_MONITOR_MODE)) {
		printk(KERN_ERR "{%s:%d} Invalid parameter:  sniffer_mode = %d \n",
			__FUNCTION__, __LINE__, sniffer_mode);
		return;
	}
	if (sniffer_mode) {
		if ((!SNIFFER_CUR_FILE_BUFFER) ||
			(g_sniffer_mode != 0 && g_sniffer_mode != sniffer_mode)) {
			// sniffer_mode changed. reset all files.
			printk(KERN_INFO "ERROR : Init proc_sniffer_init_buff \n");
			proc_sniffer_init_buff();
		}
		if (sniffer_mode == CMD_ENABLE_MONITOR_MODE) {
			/* sniffer_mode(1)+channel(1)+mac(6) */
			if (p_cmd2parse->ctl_cmd_hdr.i_para_num != CMD_ENABLE_MONITOR_MODE_PARANUM) {
				printk(KERN_ERR "{%s:%d} Invalid parameter:  i_para_num = %d \n",
					__FUNCTION__, __LINE__, p_cmd2parse->ctl_cmd_hdr.i_para_num);
				return;
			}
			parse_channel_info(&channel, &channel_type, (char *)&(p_cmd2parse->i_paras[1]));
			for (mac_loop = 0; mac_loop < MAC_ADDR_LEN; mac_loop++) {
				if (p_cmd2parse->i_paras[2 + mac_loop] < 0 ||
					p_cmd2parse->i_paras[2 + mac_loop] > 0xFF) {
					printk(KERN_ERR "{%s:%d} ERROR:Invalid parameter!\n", __FUNCTION__, __LINE__);
					return;
				}
				filter[mac_loop] = p_cmd2parse->i_paras[2 + mac_loop];
			}

			// send commands to wifi driver to enable it
			set_monitor_mode_param(sniffer_mode, channel, channel_type, filter);
		}
		// create a new file
		proc_init_file_content(g_configs.cur_file, g_configs.max_file_len);
		static_key_enable(&wifi_sniffer_enable);
	} else {
		static_key_disable(&wifi_sniffer_enable);
		// need to preserve the size of the file
		// previous mode is monitor mode, need to send extra commands to close it.
		if (g_sniffer_mode == CMD_ENABLE_MONITOR_MODE)
			set_monitor_mode_param(sniffer_mode, channel, channel_type, filter);
	}

	/* updated the sniffer status into this global variable at last. */
	g_sniffer_mode = sniffer_mode;

	printk(KERN_INFO "sniffer_mode:%d, channel:%d, \n", sniffer_mode, channel);
	return;
}

static void proc_handle_command_cur_file_len(ctl_cmd_stru *pCmd2Parse, char __user *buffer, int file_index)
{
	pCmd2Parse->i_paras[0] = sniffer_file_pos(file_index);
	// return the time of the last packet in the file
	pCmd2Parse->i_paras[1] = sniffer_file_gmttime(file_index);
	pCmd2Parse->i_paras[2] = sniffer_file_microtime(file_index);
	// return packet count
	pCmd2Parse->i_paras[3] = sniffer_file_packetcount(file_index);

	if (pCmd2Parse->ctl_cmd_hdr.i_para_num >= 4) {
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), 4 * sizeof(int));
	} else {
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), sizeof(int));
	}

	return;
}

static int proc_handle_command_file_spec(ctl_cmd_stru *pCmd2Parse, char __user *buffer)
{
	int old_file_num;
	int old_file_len;

	/* allow config file number and file length only when capture switch is off */
	if (SNIFFER_START) {
		printk(KERN_ERR "{%s:%d} Should Stop Capturing First! \n", __FUNCTION__, __LINE__);
		return -1;
	}
	if (pCmd2Parse->i_paras[0] == -1 || pCmd2Parse->i_paras[0] == 0xff ||
		pCmd2Parse->i_paras[1] == -1 || pCmd2Parse->i_paras[1] == 0xff) { // it is a read command
		pCmd2Parse->i_paras[0] = SNIFFER_MAX_FILE_NUM;
		pCmd2Parse->i_paras[1] = SNIFFER_MAX_FILE_LEN / (1024 * 1024);
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), 2 * sizeof(int));
		printk(KERN_INFO "READ FILE_NUME=%d SNIFFER_MAX_FILE_LEN=%d\n",
			pCmd2Parse->i_paras[0], pCmd2Parse->i_paras[1]);
		return 0;
	}

	if ((pCmd2Parse->i_paras[0] < PCAP_FILE_NUM_MIN) ||
		(pCmd2Parse->i_paras[0] > PCAP_FILE_NUM_MAX)) {
		printk(KERN_ERR "{%s:%d} Invalid Parameter max_file_num (%d) \n",
			__FUNCTION__, __LINE__, pCmd2Parse->i_paras[0]);
		return -1;
	}

	if (pCmd2Parse->i_paras[1] < 500 &&
		pCmd2Parse->i_paras[1] > 0) { // data from APK is in MB unit
		pCmd2Parse->i_paras[1] = pCmd2Parse->i_paras[1] * 1024 * 1024;
	}
	if ((pCmd2Parse->i_paras[1] < PCAP_FILE_LEN_MIN) ||
		(pCmd2Parse->i_paras[1] > PCAP_FILE_LEN_MAX)) {
		printk(KERN_ERR "{%s:%d} Invalid Parameter g_max_file_len (%d) \n",
			__FUNCTION__, __LINE__, pCmd2Parse->i_paras[1]);
		return -1;
	}
	printk(KERN_INFO "FILE_NUME=%d SNIFFER_MAX_FILE_LEN=%d\n",
		pCmd2Parse->i_paras[0], pCmd2Parse->i_paras[1]);
	old_file_num = SNIFFER_MAX_FILE_NUM;
	old_file_len = SNIFFER_MAX_FILE_LEN;
	SNIFFER_MAX_FILE_NUM = pCmd2Parse->i_paras[0];
	SNIFFER_MAX_FILE_LEN = pCmd2Parse->i_paras[1];
	proc_sniffer_reset_buff(old_file_num, SNIFFER_MAX_FILE_NUM,
		SNIFFER_MAX_FILE_LEN);
	return 0;
}

static int proc_handle_command_data_payload(ctl_cmd_stru *pCmd2Parse, char __user *buffer)
{
	/* allow config 802.11 payload length only when capture switch is off */
	if (SNIFFER_START) {
		printk(KERN_ERR "{%s:%d} Should Stop Capturing First! \n", __FUNCTION__,
			__LINE__);
		return -1;
	}

	if (pCmd2Parse->i_paras[0] == -1) { // it is a read command
		pCmd2Parse->i_paras[0] = SNIFFER_MAX_PACKET_LEN;
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), 1 * sizeof(int));
	} else {
		if (pCmd2Parse->i_paras[0] < 0 || pCmd2Parse->i_paras[0] > MAX_PARAM_LEN) {
			printk(KERN_ERR "i_paras[0]=%d\n", pCmd2Parse->i_paras[0]);
			return -1;
		}
		SNIFFER_MAX_PACKET_LEN = pCmd2Parse->i_paras[0];
	}
	printk(KERN_INFO "i_paras[0]=%d, SNIFFER_MAX_PACKET_LEN=%d\n", pCmd2Parse->i_paras[0], SNIFFER_MAX_PACKET_LEN);
	return 0;
}

static void proc_handle_command_mumimo_aid(ctl_cmd_stru *pCmd2Parse, char __user *buffer)
{
	if (-1 == pCmd2Parse->i_paras[0]) {
		pCmd2Parse->i_paras[0] = g_ofdma_aid;
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), 1*sizeof(int));
	} else {
	    g_ofdma_aid = pCmd2Parse->i_paras[0];
	}
	printk(KERN_INFO "ERROR : i_paras[0] = %d , g_ofdma_aid=%d\n", pCmd2Parse->i_paras[0], g_ofdma_aid);
	return;
}

static void proc_handle_command_wifi6_enable(ctl_cmd_stru *pCmd2Parse, char __user *buffer)
{
	if (-1 == pCmd2Parse->i_paras[0]) {
		pCmd2Parse->i_paras[0] = g_wifi_chip_type;
		copy_to_user((char __user *)&(((ctl_cmd_stru *)buffer)->i_paras[0]),
			(char *)&(pCmd2Parse->i_paras[0]), 1*sizeof(int));
	} else if (pCmd2Parse->i_paras[0] == 6) { // WiFi6
		g_wifi_chip_type = CHIPTYPE_1105;
	} else {
		g_wifi_chip_type = CHIPTYPE_1103;
	}
	printk(KERN_INFO "i_paras[0] = %d g_wifi_chip_type=%d\n", pCmd2Parse->i_paras[0], g_wifi_chip_type);
	return;
}

int proc_handle_commands_check_param(int file_index, ctl_cmd_stru *cmd2_parse)
{
	if ((file_index < 0) || (file_index >= PCAP_FILE_NUM)) {
		printk(KERN_ERR "{%s:%d} ERROR : Invalid file_index (%d) \n", __func__, __LINE__, file_index);
		return -1;
	}

	if ((cmd2_parse->ctl_cmd_hdr.i_cmd_id <= CMD_BUFF_CMDID_TOP)
			|| (cmd2_parse->ctl_cmd_hdr.i_cmd_id >= CMD_BUFF_CMDID_BUTTOM)) {
		printk(KERN_ERR "{%s:%d} ERROR : Invalid Command ID (0x%08x) \n",
				__FUNCTION__, __LINE__, cmd2_parse->ctl_cmd_hdr.i_cmd_id);
		return -1;
	}

	/* GET all parameters */
	if (CMD_BUFF_RDWR_READ == cmd2_parse->ctl_cmd_hdr.i_rd_wr) {
		printk(KERN_ERR "{%s:%d} TODO : GET all paras \n", __FUNCTION__, __LINE__);
		return 0;
	}

	if ((cmd2_parse->ctl_cmd_hdr.i_para_num < 0) || (cmd2_parse->ctl_cmd_hdr.i_para_num > CMD_BUF_MAX_PARAM))
	{
		printk(KERN_ERR "{%s:%d}: Invalid Para Num (0x%08x) \n",
			__FUNCTION__, __LINE__, cmd2_parse->ctl_cmd_hdr.i_para_num);
		return -1;
	}

	return 1;
}


/* return 0 , if the userBuffer is a valid command buffer. */
int proc_handle_commands(int file_index, char __user *buffer)
{
	/* 0 sniffer my own packets, 1 sniffer all packets */
	static int i_sniff_mode = -1;
	static int i_channel = -1; /* channel number */
	static int i_band_width = -1; /* 20/40/80/160 */
	static int i_proto_type = -1; /* a/b/g/n/ac/ax(1~6 respectively) */

	int ret;
	ctl_cmd_stru cmd2_parse;
	if (copy_from_user(&cmd2_parse, buffer, sizeof(ctl_cmd_stru)))
		return -EFAULT;
	ret = proc_handle_commands_check_param(file_index, &cmd2_parse);
	if (ret == -1 || ret == 0)
		return ret;

	/* SET all parameters */
	switch (cmd2_parse.ctl_cmd_hdr.i_cmd_id) {
	case CMD_BUFF_CMDID_SNIFFER_ENABLE:
		proc_handle_command_sniffer_enable(&cmd2_parse);
		break;

	case CMD_BUFF_CMDID_SNIFFER_MODE:
		i_sniff_mode = cmd2_parse.i_paras[0];
		break;

	case CMD_BUFF_CMDID_CHANNDEL:
		i_channel = cmd2_parse.i_paras[0];
		break;

	case CMD_BUFF_CMDID_BANDWIDTH:
		i_band_width = cmd2_parse.i_paras[0];
		break;

	case CMD_BUFF_CMDID_PROTOTYPE:
		i_proto_type = cmd2_parse.i_paras[0];
		break;

	case CMD_BUFF_CMDID_CURFILELEN:
		proc_handle_command_cur_file_len(&cmd2_parse, buffer, file_index);
		break;
	case CMD_BUFF_CMDID_FILESPEC:
		ret = proc_handle_command_file_spec(&cmd2_parse, buffer);
		if (ret == -1)
			return -1;
		break;
	case CMD_BUFF_CMDID_DATAPAYLOAD:
		ret = proc_handle_command_data_payload(&cmd2_parse, buffer);
		if (ret == -1)
			return -1;
		break;
	case CMD_BUFF_CMDID_MUMIMO_AID:
		proc_handle_command_mumimo_aid(&cmd2_parse, buffer);
		break;
	case CMD_BUFF_CMDID_WIFI6_ENABLE:
		proc_handle_command_wifi6_enable(&cmd2_parse, buffer);
		break;
	default:
		printk(KERN_ERR "{%s:%d}: Invalid Command ID (0x%08x) \n",
			__FUNCTION__, __LINE__, cmd2_parse.ctl_cmd_hdr.i_cmd_id);
		return -1;
	}
	return 0;
}

int proc_set_hipriv_func(void *p_func)
{
	g_hipriv_func = (drive_wal_hipriv_cmd_func)p_func;
	return 0;
}

int proc_sniffer_read_01(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	mutex_lock(&handler_mutex);
	int ret = proc_sniffer_read(0, FILE_READ, (char __user *)buffer, lenp,
		ppos);
	mutex_unlock(&handler_mutex);
	return ret;
}

int proc_sniffer_read_02(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	return proc_sniffer_read(1, FILE_READ, (char __user *)buffer, lenp,
		ppos);
}

int proc_sniffer_read_03(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	return proc_sniffer_read(2, FILE_READ, (char __user *)buffer, lenp,
		ppos);
}

int proc_sniffer_read_04(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	return proc_sniffer_read(3, FILE_READ, (char __user *)buffer, lenp,
		ppos);
}

int proc_sniffer_read_05(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos)
{
	return proc_sniffer_read(4, FILE_READ, (char __user *)buffer, lenp,
		ppos);
}

#endif

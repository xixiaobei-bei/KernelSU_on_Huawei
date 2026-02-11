


#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/kernel.h>/* add for log */
#include <linux/ctype.h>/* add for tolower */
#include <linux/slab.h>
#include <linux/jhash.h>
#include <linux/spinlock.h> /* add for spinlock */

#include "nf_hw_common.h"
#include "nf_app_dl_monitor.h"

struct appdl_rule {
	struct list_head list;
	unsigned int uid;
};

static struct list_head g_appdlhead[MAX_HASH];
static spinlock_t g_appdllock[MAX_HASH];/* lock for g_appdlhead array*/
static unsigned int g_dlid;
static struct list_head g_dl_wait;
static spinlock_t g_wait_lock;/* lock for g_appdlhead array*/
static struct timer_list g_timer;
static unsigned int g_waitcount;
static spinlock_t g_count_lock;
unsigned int g_wcount;

void add_appdl_uid(unsigned int uid)
{
	struct appdl_rule *node = NULL;
	unsigned int hid = get_hash_id(uid);

	if (hid >= MAX_HASH) {
		pr_info("hwad:add_appdl_uid>=MAX_HASH  hid=%u\n", hid);
		return;
	}
	spin_lock_bh(&g_appdllock[hid]);
	list_for_each_entry(node, &g_appdlhead[hid], list) {
		if (node->uid == uid) {
			pr_info("hwad:id=%u uid=%u alreadyin\n", hid, uid);
			spin_unlock_bh(&g_appdllock[hid]);
			return;
		}
	}
	pr_info("hwad:add_appdl_uid hid=%u uid=%u new\n", hid, uid);
	node = kmalloc(sizeof(struct appdl_rule), GFP_ATOMIC);/* new 201 */
	if (node) {
		node->uid = uid;
		list_add(&node->list, &g_appdlhead[hid]);
	}
	spin_unlock_bh(&g_appdllock[hid]);
}

void clear_appdl_uid(unsigned int uid)
{
	struct appdl_rule *node = NULL;
	unsigned int hid = get_hash_id(uid);

	if (hid >= MAX_HASH) {
		pr_info("hwad:clear_appdl_uid>=MAX_HASH id=%u\n", hid);
		return;
	}
	spin_lock_bh(&g_appdllock[hid]);
	list_for_each_entry(node, &g_appdlhead[hid], list) {
		if (node->uid == uid) {
			pr_info("hwad:clr_dl_uid uid=%u\n", uid);
			list_del(&node->list);
			spin_unlock_bh(&g_appdllock[hid]);
			kfree(node);/* free 201 */
			return;
		}
	}
	spin_unlock_bh(&g_appdllock[hid]);
}

void add_appdl_rule(const char *rule, bool bReset)
{
	/* 1001;10004;10064; */
	const char *p = NULL;
	const char *temp = NULL;
	int uid;

	if (rule == NULL)
		return;
	temp = rule;
	p = rule;
	if (bReset)
		clear_appdl_rule(1, "");
	while (*p != 0) {
		p = strstr(temp, ";");
		if (p) {
			uid = simple_strtoull(temp, NULL, 10);
			add_appdl_uid(uid);
			temp = p + 1;
		} else {
			break;
		}
	}
}

void clear_appdl_rule_string(const char *rule)
{
	/* 1001;10004;10064 */
	const char *p = NULL;
	const char *temp = NULL;
	int uid = 0;

	if (rule == NULL)
		return;
	temp = rule;
	p = rule;
	while (p && *p != 0) {
		p = strstr(temp, ";");
		if (p) {
			uid = simple_strtoull(temp, NULL, 10);
			clear_appdl_uid(uid);
			temp = p + 1;
		}
	}
}

void clear_appdl_rule(int opt, const char *rule)
{
	int i = 0;
	struct appdl_rule *node = NULL;
	struct appdl_rule *next = NULL;

	if (0 == opt) {
		clear_appdl_rule_string(rule);
		return;
	}
	pr_info("hwad:clear_appdl_rule\n");
	for (i = 0; i < MAX_HASH; i++) {
		spin_lock_bh(&g_appdllock[i]);
		list_for_each_entry_safe(node, next, &g_appdlhead[i], list) {
			list_del(&node->list);
			kfree(node);/*free 201*/
		}
		spin_unlock_bh(&g_appdllock[i]);
	}
}

void output_appdl_rule(void)
{
	int i = 0;
	struct appdl_rule *node = NULL;

	pr_info("hwad: output_appdl_rule start\n");
	for (i = 0; i < MAX_HASH; i++) {
		spin_lock_bh(&g_appdllock[i]);
		list_for_each_entry(node, &g_appdlhead[i], list) {
			pr_info("hwad:output_appdl_rule uid=%d\n",
				node->uid);
		}
		spin_unlock_bh(&g_appdllock[i]);
	}
	pr_info("hwad:output_appdl_rule end\n");

}

bool match_appdl_uid(unsigned int uid)
{
	struct appdl_rule *node = NULL;
	unsigned int hid = get_hash_id(uid);

	if (hid >= MAX_HASH) {
		pr_info("hwad:match_appdl_uid>=MAX_HASH hid=%u\n", hid);
		return false;
	}
	spin_lock_bh(&g_appdllock[hid]);
	list_for_each_entry(node, &g_appdlhead[hid], list) {
		if (node->uid == uid) {
			spin_unlock_bh(&g_appdllock[hid]);
			return true;
		}
	}
	spin_unlock_bh(&g_appdllock[hid]);
	return false;
}

bool match_appdl_url(const char *data, int datalen)
{
	bool bret = false;
	char *urlpath = get_url_path(data, datalen);

	if (urlpath) {
		if (strfindpos(urlpath, ".apk", strlen(urlpath))) {
			bret = true;
			pr_info("hwad:match dlurl true urlpath=%s\n", urlpath);
		}
		kfree(urlpath);
	}
	return bret;
}


char *get_report_msg(unsigned int dlid, unsigned int uid,
		     const char *url,
		     char *ip)
{
	char *urlhex = NULL;
	char *buf = NULL;
	int len = 0;
	char *temp = NULL;
	unsigned int blen;

	if (NULL == url)
		return NULL;
	len = strlen(url);
	temp = kmalloc(len + 64 + 1, GFP_ATOMIC);/* new 1001   */
	if (NULL == temp)
		return NULL;
	snprintf(temp, len + 64 + 1, "%s", url);
	len = strlen(temp);
	if (len >= 4000)/*if the url over 4000, it will be miss the last*/
		len = 4000;
	urlhex = kmalloc(len * 2 + 1, GFP_ATOMIC);/* new 701   */
	if (NULL == urlhex) {
		kfree(temp);
		pr_info("hwad:report_msg new urlhex failure\n");
		return NULL;
	}
	byte_to_hex(temp, len, urlhex, len * 2 + 1);
	urlhex[len * 2] = 0;
	blen = len * 2 + 1 + 32;
	buf = kmalloc(blen, GFP_ATOMIC);/* new 801   */
	if (NULL == buf) {
		kfree(temp);
		kfree(urlhex);
		return NULL;
	}
	memset(buf, 0, blen);
	snprintf(buf, blen, "%u %u %s", dlid, uid, urlhex);
	len = strlen(buf);
	pr_info("hwad:report_msg dlid=%u len=%d buf=%s\n", dlid, len, buf);
	kfree(urlhex); /* free 701   */
	kfree(temp); /* free 1001   */
	return buf;
}

struct dl_info *new_dl_monitor(unsigned int uid, unsigned int dlid, const char *url)
{
	struct dl_info *newnode;
	int len = strlen(url);

	if (len < 0 || len > MAX_URL_LENGTH)
		return NULL;
	newnode = kmalloc(sizeof(struct dl_info), GFP_ATOMIC);/* new 301 */
	if (NULL == newnode)
		return NULL;
	newnode->dlid = dlid;
	newnode->action = DLST_WAIT;/* default not block */
	newnode->url = kmalloc(len + 1, GFP_ATOMIC);/* new 901 */
	if (NULL == newnode->url) {
		kfree(newnode);
		return NULL;
	}
	memcpy(newnode->url, url, len);
	newnode->url[len] = 0;
	newnode->len = len;
	newnode->uid = uid;
	newnode->bwait = false;

	return newnode;
}

void free_download_monitor(struct dl_info *node)
{
	kfree(node->url);/* free 901 */
	kfree(node);/* free 301 */
}

/*0:not in list 1: waiting 2:timeout or accept 3:drop */
int get_select(struct sk_buff *skb)
{
	struct dl_info *tm = NULL;
	struct dl_info *nd = NULL;
	bool bfind = false;

	spin_lock_bh(&g_wait_lock);
	list_for_each_entry(tm, &g_dl_wait, list) {
		if (tm->skb == skb) {
			bfind = true;
			nd = tm;
		}
	}
	spin_unlock_bh(&g_wait_lock);
	if ((!bfind) || (NULL == nd))
		return DLST_NOT;
	if (get_cur_time() - nd->start_time > MAX_WAIT &&
	    nd->action == DLST_WAIT) {
		nd->action = DLST_ALLOW;
		return DLST_ALLOW;
	}
	if (nd->action == DLST_WAIT) {
		pr_info("hwad:get_select DLST_WAIT %p dlid=%d\n",
			skb, nd->dlid);
	} else {
		pr_info("hwad:get_select NF_%d %p dlid=%d\n",
			nd->action, skb, nd->dlid);
	}
	return nd->action;
}

static void dl_timer(unsigned long dlid)
{
	struct dl_info *node = NULL;
	struct dl_info *next = NULL;

	if (dlid == 0) {
		spin_lock_bh(&g_wait_lock);
		list_for_each_entry_safe(node, next, &g_dl_wait, list) {
			if (get_cur_time() - node->start_time > 120000) {
				list_del(&node->list);
				pr_info("hwad:dl_timer free dlid=%d url=%s\n",
					node->dlid, node->url);
				free_download_monitor(node);
			}
		}
		spin_unlock_bh(&g_wait_lock);
	}
	/* restart timer */
	g_timer.expires = jiffies + 180 * HZ;
	add_timer(&g_timer);
}

struct dl_info *get_download_monitor(struct sk_buff *skb, unsigned int uid,
				     const char *url)
{
	struct dl_info *nd = NULL;
	struct dl_info *tm = NULL;
	struct dl_info *td = NULL;
	unsigned int dlid = 0;
	int l = strlen(url);

	spin_lock_bh(&g_wait_lock);
	dlid = g_dlid++;
	if (g_dlid > ID_MAXID)
		g_dlid = ID_START;
	nd = new_dl_monitor(uid, dlid, url);
	if (NULL == nd) {
		spin_unlock_bh(&g_wait_lock);
		return NULL;
	}
	nd->skb = skb;
	nd->start_time = get_cur_time();
	list_add(&nd->list, &g_dl_wait);
	list_for_each_entry(tm, &g_dl_wait, list) {
		if (nd->uid == uid) {
			if (nd->dlid != tm->dlid) {
				if (tm->len == l) {
					if (memcmp(nd->url, tm->url, l) == 0) {
						nd->bwait = true;
						td = tm;
						dlid = tm->dlid;
						nd->action = tm->action;
						break;
					}
				}
			}
		}
	}
	if(td) {
		if(get_cur_time() - td->start_time > MAX_WAIT)
			nd->bwait = false;
	}
	if (nd->bwait)
		pr_info("hwad:bwait=true dlid=%d oldid=%d\n", nd->dlid, dlid);
	else
		pr_info("hwad:bwait=false dlid=%d\n", nd->dlid);
	spin_unlock_bh(&g_wait_lock);
	return nd;
}

void nofity_loop(unsigned int uid, unsigned int dlid, char *url, int action)
{
	struct dl_info *node = NULL;
	int len = strlen(url);

	list_for_each_entry(node, &g_dl_wait, list) {
		if (uid == node->uid && len == node->len &&
		    0 == memcmp(node->url, url, len) &&
		    dlid != node->dlid) {
			node->action = action;
			pr_info("hwad:nofity_loop uid=%d dlid=%d act=%d\n",
				node->uid,
				node->dlid,
				node->action);
		}
	}
}

void download_notify(int dlid, const char *action)
{
	struct dl_info *ld = NULL;

	if (0 == dlid || dlid < 1000)
		return;
	spin_lock_bh(&g_wait_lock);
	list_for_each_entry(ld, &g_dl_wait, list) {
		if (dlid == ld->dlid) {
			if (memcmp(action, "allow", strlen("allow")) == 0) {
				ld->action = DLST_ALLOW;
				pr_info("hwad:notify dlid=%d allow\n", dlid);
			} else {
				ld->action = DLST_REJECT;
				pr_info("hwad:notify dlid=%d reject\n", dlid);
			}
			nofity_loop(ld->uid, dlid, ld->url, ld->action);
			spin_unlock_bh(&g_wait_lock);
			return;
		}
	}
	spin_unlock_bh(&g_wait_lock);
	pr_info("hwad:notify not found dlid=%d act=%s\n", dlid, action);
}

void free_node(struct dl_info *node)
{
	spin_lock_bh(&g_wait_lock);
	list_del(&node->list);
	spin_unlock_bh(&g_wait_lock);
	free_download_monitor(node);
}

unsigned int  wait_user_event(struct dl_info *node)
{
	/* wait user  confirm to download  */
	int cout = 0;
	bool buselock = false;
	u64 start_time = get_cur_time();
	unsigned int action = 0;

	if (g_wcount >= MAX_THREAD) {
		pr_info("hwad:wait DLST_NOT wcnt=%d\n", g_wcount);
		return DLST_NOT;
	}
	spin_lock_bh(&g_count_lock);
	g_wcount++;
	spin_unlock_bh(&g_count_lock);
	pr_info("hwad:wait dlid=%d wcnt=%d\n", node->dlid, g_wcount);
	while (get_cur_time() - start_time < MAX_WAIT) {
		if (DLST_WAIT != node->action) {
			buselock = true;
			break;
		}
		cout++;
		mdelay(WAIT_DELAY);
	}
	spin_lock_bh(&g_count_lock);
	g_wcount--;
	spin_unlock_bh(&g_count_lock);
	if (buselock) {
		/* notify action */
		pr_info("hwad:wait event dlid=%d action=%d wcnt=%d time=%d\n",
			node->dlid, node->action, g_wcount,
			(unsigned int)(get_cur_time() - start_time));
	} else {
		pr_info("hwad:wait timeout dlid=%d cnt=%d wcnt=%d time=%d\n",
			node->dlid, cout, g_wcount,
			(unsigned int)(get_cur_time() - start_time));
		node->action = DLST_ALLOW;
	}
	action = node->action;
	free_node(node);
	return action;
}

void init_appdl(void)
{
	int i;

	g_dlid = ID_START;
	g_waitcount = 0;
	g_wcount = 0;
	for (i = 0; i < MAX_HASH; i++) {
		INIT_LIST_HEAD(&g_appdlhead[i]);
		spin_lock_init(&g_appdllock[i]);
	}
	INIT_LIST_HEAD(&g_dl_wait);
	spin_lock_init(&g_wait_lock);
	spin_lock_init(&g_count_lock);
	init_timer(&g_timer);
	g_timer.data = 0;
	g_timer.function = dl_timer;
	g_timer.expires = jiffies + 180 * HZ;
	add_timer(&g_timer);
}

void uninit_appdl(void)
{
}


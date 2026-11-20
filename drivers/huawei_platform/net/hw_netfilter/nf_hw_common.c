/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: This file used by the netfilter hook for app download monitor
 *              and ad filter.
 * Author: chenzhongxian@huawei.com
 * Create: 2016-05-28
 */

#include <linux/ctype.h>
#include <linux/jhash.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "nf_hw_common.h"

const char *strfind(const char *s1, const char *s2)
{
	register const unsigned char *s = (unsigned char *)s1;
	register const unsigned char *p = (unsigned char *)s2;

	do {
		if (!*p)
			return (const char *)s1;
		if (*p == *s || tolower(*p) == tolower(*s)) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s)
				return NULL;
			s = ++s1;
		}
	} while (1);
}

const char *strfindpos(const char *s1, const char *s2, int len)
{
	register const unsigned char *s = (unsigned char *)s1;
	register const unsigned char *p = (unsigned char *)s2;

	do {
		if (!*p)
			return (const char *)s1;
		if (*p == *s || tolower(*p) == tolower(*s)) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s || len == 0)
				return NULL;
			s = ++s1;
			len--;
		}
	} while (1);
}

const char *strfinds2(const char *s1, const char *s2, int s2l)
{
	register const unsigned char *s = (unsigned char *)s1;
	register const unsigned char *p = (unsigned char *)s2;

	do {
		if (p == (const unsigned char *)s2 + s2l || !*p)
			return (const char *)s1;
		if (*p == *s || tolower(*p) == tolower(*s)) {
			++p;
			++s;
		} else {
			p = (const unsigned char *)s2;
			if (!*s)
				return NULL;
			s1++;
			s = (const unsigned char *)s1;
		}
	} while (1);
}

const char *substring(const char *src, const char *f, const char *s, int *l)
{
	/* "k=<xxx>"  f="k=<"  s=">" ret="xxx" */
	const char *pf = NULL;
	const char *ps = NULL;
	const char *ret = NULL;
	int datalen;
	int flen;

	if ((src == NULL) || (f == NULL) || (s == NULL) || (l == NULL))
		return NULL;

	datalen = *l;
	flen = strlen(f);

	*l = 0;
	if (flen == 0)
		return NULL;
	if (strlen(s) == 0)
		return NULL;
	pf = strfindpos(src, f, datalen);
	if (pf == NULL)
		return NULL;
	ps = strfindpos(pf + flen, s, datalen - (pf + flen - src));
	if (ps == NULL)
		return NULL;
	ret = pf + strlen(f);
	*l = ps - ret;
	return ret;
}

void byte_to_hex(const char *pin, int ilen, char *pout, int olen)
{
	char hex[] = "0123456789ABCDEF";
	int i;
	unsigned char *input = (unsigned char *)pin;
	unsigned char *output = (unsigned char *)pout;

	if (ilen <= 0 || olen <= 1)
		return;
	for (i = 0; i < ilen; i++) {
		if (i * DOUBLE_TIME + 1 > olen - 1)
			break;
		output[i * DOUBLE_TIME] = hex[input[i] / HEX_LEN];
		output[i * DOUBLE_TIME + 1] = hex[input[i] % HEX_LEN];
	}
	if (i * DOUBLE_TIME <= olen)
		output[i * DOUBLE_TIME] = 0;
	else
		output[(i - 1) * DOUBLE_TIME + 1] = 0;
}

unsigned char hex_string_to_value(unsigned char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return (ch - 'A') + HEX_VALUE_OFFSET;
	else if (ch >= 'a' && ch <= 'f')
		return (ch - 'a') + HEX_VALUE_OFFSET;
	return 0;
}

void hex_to_byte(const char *pin, int inlen, char *pout, int olen)
{
	int i;
	int j;
	unsigned char *input = (unsigned char *)pin;
	unsigned char *output = (unsigned char *)pout;
	int n = inlen / DOUBLE_TIME;

	output[0] = 0;
	if (olen < inlen / DOUBLE_TIME || inlen <= 1)
		return;

	for (i = 0; i < n; i++) {
		output[i] = 0;
		for (j = 0; j < DOUBLE_TIME; j++) {
			output[i] +=
				hex_string_to_value(input[i * DOUBLE_TIME + j]);
			if (j == 0)
				output[i] *= HEX_LEN;
			}
		}
}

u64 get_cur_time(void)
{
	u64 ret = (u64)jiffies;

	ret = ret * US_MS / HZ;
	return ret;
}

/* remove the blank char from the right,fill with \0 */
void right_trim(char *p)
{
	int len = strlen(p);
	char *pos = p + len - 1;

	while (pos >= p) {
		if (*pos == ' ' || *pos == '\t' || *pos == '\f' ||
			*pos == '\v' || *pos == '\0' ||
			*pos == POS_INVALID_VALUE ||
			*pos == '\r' || *pos == '\n')
			*pos = 0;
		else
			break;
		pos--;
	}
}

/* remove the blank char from the left */
const char *left_trim(const char *p)
{
	const char *pos = p;

	while (*pos != 0) {
		if (*pos == ' ' || *pos == '\t' || *pos == '\f' ||
			*pos == '\v' || *pos == '\0' ||
			*pos == POS_INVALID_VALUE ||
			*pos == '\r' || *pos == '\n')
			pos++;
		else
			break;
	}
	return pos;
}

char *get_url_path(const char *data, int datalen)
{
	int uplen = datalen;
	char *temurl = NULL;
	const char *temurl1 = NULL;
	char *temp = NULL;
	const char *urlpath = substring(data, " ", " HTTP", &uplen);

	if (uplen == 0) {
		pr_info("\nhwad:upath ul=%d hl=%d\n", uplen, datalen);
		return NULL;
	}
	temurl = kmalloc(uplen + 1, GFP_ATOMIC);
	if (temurl == NULL)
		return NULL;
	memcpy(temurl, urlpath, uplen);
	temurl[uplen] = 0;
	right_trim(temurl);
	temurl1 = left_trim(temurl);

	temp = kmalloc(uplen + ULR_LEN_OFFSET_64, GFP_ATOMIC);
	if (temp == NULL) {
		kfree(temurl);
		return NULL;
	}
	snprintf(temp, uplen + ULR_LEN_OFFSET_64, "%s", temurl1);
	uplen = strlen(temurl1);
	temp[uplen] = 0;
	kfree(temurl);
	return temp;
}

static char * get_url_form_data_post_proc(const char *temurl, char *temhost,
	int uplen, int hlen)
{
	const char *temurl1 = NULL;
	const char *temhost1 = NULL;
	char *temp = NULL;

	temurl1 = left_trim(temurl);
	right_trim(temhost);
	temhost1 = left_trim(temhost);
	temp = kmalloc(uplen + hlen + ULR_LEN_OFFSET_64, GFP_ATOMIC);
	if (temp == NULL) {
		kfree(temhost);
		kfree(temurl);
		return NULL;
	}
	if (snprintf(temp, uplen + hlen + ULR_LEN_OFFSET_64, "http://%s%s",
		temhost1, temurl1) == -1)
		pr_info("snprintf return error\n");
	kfree(temhost);
	kfree(temurl);
	return temp;
}

 /* return : http://xxx.com/yyy */
char *get_url_form_data(const char *data, int datalen)
{
	int uplen = datalen;
	int hlen = datalen;
	char *temurl = NULL;
	char *temhost = NULL;
	char *temp = NULL;
	const char *urlpath = substring(data, " ", " HTTP", &uplen);
	const char *host = substring(data, "Host:", "\n", &hlen);

	if (uplen == 0 || hlen == 0) {
		pr_info("\nhwad:upath ul=%d hl=%d\n", uplen, hlen);
		return NULL;
	}
	if (strfindpos(urlpath, "http://", HTTP_HEAD_LEN)) {
		const char *urltem = left_trim(urlpath);
		int len = uplen - (urltem - urlpath);

		if (len > 0) {
			temurl = kmalloc(len + 1, GFP_ATOMIC);
			if (temurl == NULL)
				return NULL;
			memset(temurl, 0, len + 1);
			memcpy(temurl, urltem, len);
			return temurl;
		}
	}
	temurl = kmalloc(uplen + 1, GFP_ATOMIC);
	if (temurl == NULL)
		return NULL;
	temhost = kmalloc(hlen + 1, GFP_ATOMIC);
	if (temhost == NULL) {
		kfree(temurl);
		return NULL;
	}
	memcpy(temurl, urlpath, uplen);
	temurl[uplen] = 0;
	memcpy(temhost, host, hlen);
	temhost[hlen] = 0;
	right_trim(temurl);
	temp = get_url_form_data_post_proc(temurl, temhost, uplen, hlen);
	return temp;
}

unsigned int get_hash_id(int uid)
{
	unsigned int  hid = jhash_3words(uid, 0, 0, HASH_INITVAL);

	hid %= MAX_HASH;
	return hid;
}




#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>/* add for log */
#include <linux/ctype.h>/* add for tolower */
#include <linux/slab.h>
#include <linux/jhash.h>/* add for jhash */
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
		if (p  == (const unsigned char *) s2 + s2l || !*p)
			return (const char *)s1;
		if (*p == *s || tolower(*p) == tolower(*s)) {
			++p;
			++s;
		} else {
			p = (const unsigned char *)s2;
			if (!*s)
				return NULL;
			s = (const unsigned char *) ++s1;
		}
	} while (1);
}

const char *substring(const char *src, const char *f, const char *s, int *l)
{
	/* "k=<xxx>"  f="k=<"  s=">" ret="xxx"*/
	const char *pf = NULL;
	const char *ps = NULL;
	const char *ret = NULL;
	int datalen = *l;
	int flen = strlen(f);

	*l = 0;
	if (0 == flen)
		return NULL;
	if (0 == strlen(s))
		return NULL;
	pf = strfindpos(src, f, datalen);
	if (NULL == pf)
		return NULL;
	ps = strfindpos(pf + flen, s, datalen - (pf + flen - src));
	if (NULL == ps)
		return NULL;
	ret = pf + strlen(f);
	*l = ps - ret;
	return ret;
}

void byte_to_hex(const void *pin, int ilen, void *pout, int olen)
{
	char hex[] = "0123456789ABCDEF";
	int i = 0;
	unsigned char *input = (unsigned char *)pin;
	unsigned char *output = (unsigned char *)pout;

	if (ilen <= 0 || olen <= 1)
		return;
	for (i = 0; i < ilen; i++) {
		if (i * 2 + 1 > olen-1)
			break;
		output[i * 2] = hex[input[i] / 16];
		output[i * 2 + 1] = hex[input[i] % 16];
	}
	if (i * 2 <= olen)
		output[i * 2] = 0;
	else
		output[(i - 1) * 2 + 1] = 0;

}

unsigned char hex_string_to_value(unsigned char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return (ch - 'A') + 10;
	else if (ch >= 'a' && ch <= 'f')
		return (ch - 'a') + 10;
	return 0;
}

void hex_to_byte(const void *pin, int inlen, void *pout, int olen)
{
	int i = 0;
	int j = 0;
	unsigned char *input = (unsigned char *)pin;
	unsigned char *output = (unsigned char *)pout;
	int n = inlen / 2;

	output[0] = 0;
	if (olen < inlen / 2 || inlen <= 1)
		return;

	for (i = 0; i < n; i++) {
		output[i] = 0;
		for (j = 0; j < 2; j++) {
			output[i] += hex_string_to_value(input[i * 2 + j]);
			if (j == 0)
				output[i] *= 16;
			}
		}
}

u64 get_cur_time(void)
{
	u64 ret = (u64)jiffies;

	ret = ret * 1000 / HZ;
	return ret;
}

/*remove the blank char from the right,fill with \0*/
void right_trim(char *p)
{
	int len = strlen(p);
	char *pos = p + len - 1;

	while (pos >= p) {
		if (*pos == ' ' || *pos == '\t' || *pos == '\f' ||
				*pos == '\v' || *pos == '\0' || *pos == 160 ||
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
				*pos == '\v' || *pos == '\0' || *pos == 160 ||
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
	if (NULL == temurl)
		return NULL;
	memcpy(temurl, urlpath, uplen);
	temurl[uplen] = 0;
	right_trim(temurl);
	temurl1 = left_trim(temurl);

	temp = kmalloc(uplen + 64, GFP_ATOMIC);
	if (NULL == temp) {
		kfree(temurl);
		return NULL;
	}
	snprintf(temp, uplen + 64, "%s", temurl1);
	uplen = strlen(temurl1);
	temp[uplen] = 0;
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
	const char *temurl1 = NULL;
	const char *temhost1 = NULL;
	char *temp = NULL;
	const char *urlpath = substring(data, " ", " HTTP", &uplen);
	const char *host = substring(data, "Host:", "\n", &hlen);

	if (uplen == 0 || hlen == 0) {
		pr_info("\nhwad:upath ul=%d hl=%d\n", uplen, hlen);
		return NULL;
	}
	if (strfindpos(urlpath, "http://", 10)) {
		const char *urltem = left_trim(urlpath);
		int len = uplen - (urltem - urlpath);

		if (len > 0) {
			temurl = kmalloc(len + 1, GFP_ATOMIC);
			if (NULL == temurl)
				return NULL;
			memset(temurl, 0, len + 1);
			memcpy(temurl, urltem, len);
			return temurl;
		}
	}
	temurl = kmalloc(uplen + 1, GFP_ATOMIC);
	if (NULL == temurl)
		return NULL;
	temhost = kmalloc(hlen + 1, GFP_ATOMIC);
	if (NULL == temhost) {
		kfree(temurl);
		return NULL;
	}
	memcpy(temurl, urlpath, uplen);
	temurl[uplen] = 0;
	memcpy(temhost, host, hlen);
	temhost[hlen] = 0;
	right_trim(temurl);
	temurl1 = left_trim(temurl);

	right_trim(temhost);
	temhost1 = left_trim(temhost);
	temp = kmalloc(uplen + hlen + 64, GFP_ATOMIC);
	if (NULL == temp) {
		kfree(temhost);
		kfree(temurl);
		return NULL;
	}
	snprintf(temp, uplen + hlen + 64, "http://%s%s", temhost1, temurl1);
	kfree(temhost);
	kfree(temurl);
	return temp;
}

unsigned int get_hash_id(int uid)
{
	unsigned int  hid = jhash_3words(uid, 0, 0, 1005);

	hid %= MAX_HASH;
	return hid;
}


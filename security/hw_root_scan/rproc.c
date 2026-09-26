/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2021. All rights reserved.
 * Description: the rproc.c for root processes list checking
 * Author: Yongzheng Wu <Wu.Yongzheng@huawei.com>
 *         likun <quentin.lee@huawei.com>
 *         likan <likan82@huawei.com>
 * Create: 2016-06-18
 */

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h> /* for find_lock_task_mm */
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>
#include <linux/version.h>
#include "./include/hw_rscan_utils.h"

#define MAX_PROC_NUM 128 /* 128 root processes should be enough for anybody */

static const char *TAG = "rproc";

/*
 * return number of tokens
 * Note, paths will be modified, just like strtok
 */
static int paths_explode(char *paths, char **const tokens)
{
	int ntoken = 0;
	int i = 0;

	tokens[ntoken++] = paths;
	while ((paths + i) && *(paths + i)) {
		if (paths[i] == ':') {
			paths[i] = '\0';
			if (ntoken >= MAX_PROC_NUM) {
				RSLogError(TAG, "paths_explode overflow");
				break;
			}

			if ((paths + i + 1) && *(paths + i + 1)) {
				tokens[ntoken++] = paths + i + 1;
			} else {
				RSLogError(TAG, "white list formate abnormal");
				break;
			}
		}
		i++;
	}

	return ntoken;
}

static void paths_implode(char *paths,  char **const tokens, int ntoken)
{
	int ti;

	if (ntoken == 0) {
		paths[0] = '\0';
		return;
	}

	for (ti = 0; ti < ntoken; ti++)
		paths += sprintf(paths, ti == 0 ? "%s" : ":%s", tokens[ti]);
}

static int tokens_cmp(const void *p1, const void *p2)
{
	char * const *i1 = p1;
	char * const *i2 = p2;

	return strcmp(*i1, *i2);
}

static char *g_trustlist_proc[MAX_PROC_NUM] = { NULL };
static int g_trustlist_count;
static char *g_trustlist_proc_buf;

static int move_tokens(char **const tokens, uint32_t cur,
			int next, int token_count)
{
	int ret;
	ret = memmove_s(tokens + cur,
			(ulong)((token_count - next) * sizeof(char *)),
			tokens + next,
			(ulong)((token_count - next) * sizeof(char *)));
	if (ret != EOK)
		RSLogError(TAG, "memmove_s fail\n");

	return ret;
}

static bool is_dup_token(char **const tokens, uint32_t index)
{
	/* we should delete empty token */
	if (tokens[index][0]  == '\0')
		return true;

	/* the current token is same with the last, so it's dup token */
	if ((index > 0) && (strcmp(tokens[index - 1], tokens[index]) == 0))
		return true;

	return false;
}

/*
 * "::sbin/c:sbin/a:sbin/b::sbin/a"  ->   [sbin/a][sbin/b][sbin/c]
 */
static bool format_rprocs(char **const tokens, int *ntoken, char *trustlist)
{
	uint32_t cur;
	int token_count;
	int ret;

	if ((tokens == NULL) || (trustlist == NULL) || (ntoken == NULL))
		return false;

	token_count = paths_explode(trustlist, tokens);

	sort(tokens, (ulong)token_count, sizeof(char *), tokens_cmp, NULL);

	/* delete dup */
	for (cur = 0; cur < token_count; cur++) {
		if (is_dup_token(tokens, cur)) {
			int next = cur + 1;

			if (next < token_count) {
				ret = move_tokens(tokens, cur, next,
							token_count);
				if (ret != EOK)
					return false;
			}
			cur--;
			token_count--;
		}
	}
	(*ntoken) = token_count;

	return true;
}

bool init_rprocs_trustlist(const char *trustlist)
{
	int ret;
	int trustlist_proc_buf_len;

	if (trustlist == NULL)
		return false;

	if (strlen(trustlist) >= PAGE_SIZE) {
		RSLogError(TAG, "trustlist over the PAGE_SIZE, %u",
				(unsigned int)strlen(trustlist));
		return false;
	}

	if (g_trustlist_proc_buf == NULL) {
		/* never freed, the maximum size of trustlist is PAGE_SIZE */
		g_trustlist_proc_buf = (char *)(uintptr_t)__get_free_page(GFP_KERNEL);
		trustlist_proc_buf_len = PAGE_SIZE;

		if (g_trustlist_proc_buf != NULL) {
			ret = strncpy_s(g_trustlist_proc_buf,
					trustlist_proc_buf_len,
					trustlist, PAGE_SIZE);
			if (ret != EOK) {
				RSLogError(TAG, "strncpy_s fail\n");
				return false;
			}
		} else {
			return false;
		}
	}
	return format_rprocs(g_trustlist_proc, &g_trustlist_count,
			g_trustlist_proc_buf);
}

bool find_proc_in_init_list(const char *proc_name)
{
	/* use dichotomization */
	int begin = 0;
	int end = g_trustlist_count - 1;

	if (proc_name == NULL)
		return false;

	while (begin <= end) {
		int pos = (begin + end) / 2;
		int ret = strcmp(proc_name, g_trustlist_proc[pos]);
		if (ret == 0) {
			return true;
		} else if (ret < 0) {
			/* proc_name < current */
			end = pos - 1;
		} else {
			/* proc_name > current */
			begin = pos + 1;
		}
	}
	return false;
}

/*
 * "::sbin/c:sbin/a:sbin/b::sbin/a"  ->   "sbin/a:sbin/b:sbin/c"
 */
void _rprocs_strip_trustlist(char **const exec_path, char **final_tokens,
					char *rprocs, ssize_t rprocs_len)
{
	int ntoken = 0;
	int final_ntoken = 0;
	int i;
	int ret;
	char *tmp = NULL;

	if ((exec_path == NULL) || (final_tokens == NULL) || (rprocs == NULL) ||
			(rprocs_len > MAX_RPROC_SIZE) || (rprocs_len <= 0)) {
		RSLogError(TAG, "strip white list failed, input value invalid");
		return;
	}

	tmp = vmalloc((ulong)rprocs_len);
	if (tmp == NULL) {
		RSLogError(TAG, "rprocs_strip_trustlist failed, out of memory");
		return;
	}

	ret = memcpy_s(tmp, rprocs_len, rprocs, rprocs_len);
	if (ret != EOK) {
		RSLogError(TAG, "memcpy_s fail\n");
		vfree(tmp);
		return;
	}

	if (!format_rprocs(exec_path, &ntoken, tmp))
		RSLogError(TAG, "strip white list failed, format_rprocs error");

	/* strip trustlist */
	for (i = 0; i < ntoken; i++) {
		if (!find_proc_in_init_list(exec_path[i])) {
			final_tokens[final_ntoken++] = exec_path[i];
			RSLogError(TAG, "the root proc is abnormal: %s", exec_path[i]);
		}
	}

	/* reconstruct */
	paths_implode(rprocs, final_tokens, final_ntoken);
	vfree(tmp);
}

/*
 * remove tokens that are in rprocs_trustlist, also sort and remove duplicates
 * Example:
 * input: "a:d:a:c:d:b"
 * rprocs_trustlist: "a:c"
 * output: "b:d"
 */
void rprocs_strip_trustlist(char *rprocs, ssize_t rprocs_len)
{
	char **tokens = NULL;
	char **final_tokens = NULL;

	tokens = kmalloc((ulong)MAX_PROC_NUM * sizeof(char *), GFP_KERNEL);
	if (tokens == NULL) {
		RSLogError(TAG, "no enough memory for allocation");
		return;
	}

	final_tokens = kmalloc((ulong)MAX_PROC_NUM * sizeof(char *),
			GFP_KERNEL);
	if (final_tokens == NULL) {
		RSLogError(TAG, "no enough memory for allocation");
		kfree(tokens);
		return;
	}

	_rprocs_strip_trustlist(tokens, final_tokens, rprocs, rprocs_len);

	kfree(tokens);
	kfree(final_tokens);
}

/* For pre-4.1 kernel, calling get_mm_exe_file causes kernel crash, because
 * we are holding task_lock and get_mm_exe_file calls down_read() which calls
 * might_sleep(). We reimplement get_mm_exe_file and use down_read_trylock()
 * instead of down_read().
 * Since kernel 4.1, get_mm_exe_file() start to use RCU instead of semaphore.
 *  We are safe to call it.
 * The exact commit is 90f31d0ea88880f780574f3d0bb1a227c4c66ca3.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
#define my_get_mm_exe_file(mm) get_mm_exe_file(mm)
#else
static struct file *my_get_mm_exe_file(struct mm_struct *mm)
{
	struct file *exe_file = NULL;

	if (!down_read_trylock(&mm->mmap_sem))
		return NULL;
	exe_file = mm->exe_file;
	if (exe_file != NULL)
		get_file(exe_file);
	up_read(&mm->mmap_sem);
	return exe_file;
}
#endif

/*
 * get the path of the process' execute file.
 * refer to fs/proc/base.c proc_exe_link() and do_proc_readlink()
 * return length of the path string.
 * return -1 if outlen is not enough
 * return 0 on other failure
 */
static int get_task_exe(struct mm_struct *mm, pid_t pid, char *out,
					int outlen, char *dbuf, int dbuflen)
{
	struct file *exe_file = NULL;
	char *pathname = NULL;
	char *blank_spaces = NULL;
	size_t len;
	int ret;

	if ((mm == NULL) || (out == NULL) || (dbuf == NULL)) {
		RSLogError(TAG, "input parameter invalid");
		return -EINVAL;
	}
	if (&mm->mmap_sem == NULL)
		return -EINVAL;
	exe_file = my_get_mm_exe_file(mm);
	if (exe_file == NULL) {
		RSLogError(TAG, "get mm exe file %d failed", pid);
		return 0;
	}
	pathname = d_path(&exe_file->f_path, dbuf, dbuflen);
	fput(exe_file);
	if (pathname == NULL || IS_ERR(pathname)) {
		RSLogError(TAG, "get task exe d_path %d failed", pid);
		return 0;
	}

	blank_spaces = strchr(pathname, ' ');
	/* handle "/path/file (deleted)" */
	if (blank_spaces != NULL)
		blank_spaces[0] = '\0';

	len = strlen(pathname);
	if (len >= SIZE_MAX) {
		RSLogError(TAG, "value of len larger than SIZE_MAX");
		return 0;
	}

	if (outlen < (int)len) {
		RSLogWarning(TAG, "get task exe path too long \"%s\" > %d",
			pathname, outlen);
		return -1;
	}

	ret = memcpy_s(out, len, pathname, len);
	if (ret != EOK) {
		RSLogError(TAG, "memcpy_s fail\n");
		return -1;
	}

	return len;
}

int get_root_procs(char *out, size_t outlen)
{
	char *tmp = NULL;
	struct task_struct *task = NULL;
	size_t pos = 0;

	if (out == NULL || outlen <= 0) {
		RSLogError(TAG, "input parameter invalid");
		return -EINVAL;
	}

	tmp = (char *)(uintptr_t)__get_free_page(GFP_KERNEL);
	if (tmp == NULL) {
		RSLogError(TAG, "get free page failed");
		return -EINVAL;
	}

	read_lock(&tasklist_lock);
	for_each_process(task) { /* lint -save -e550 */
		int len;
		struct task_struct *t = NULL;

		/*
		 * last char must be '\0',
		 * so the invalid space for the process name must -1
		 */
		if (pos >= outlen - 1)
			break;

		if ((task->flags & PF_KTHREAD) || (task->mm == NULL) ||
				(from_kuid_munged(&init_user_ns,
				task->cred->euid) != 0)) {
			continue;
		}

		/* follow mm/oom_kill.c:dump_tasks */
		t = find_lock_task_mm(task);
		if ((t == NULL) || (t->mm == NULL))
			continue;

		len = get_task_exe(t->mm, t->pid, out + pos, outlen - pos - 1,
				tmp, PAGE_SIZE);
		task_unlock(t);

		if (len < 0)
			break;

		if (len == 0)
			continue;

		pos += len;

		/* add split char */
		out[pos++] = ':';
	}
	read_unlock(&tasklist_lock);
	free_page((uintptr_t)tmp);

	/* replace last split char to '\0' */
	if (pos > 0) {
		pos--;
		out[pos] = '\0';
	}

	return pos;
}


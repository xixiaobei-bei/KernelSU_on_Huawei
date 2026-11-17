/* SPDX-License-Identifier: GPL-2.0 */
/*
 * message_verify.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: wangminmin4@huawei.com lousong@huawei.com
 * Create: 2020-07-08
 *
 */

#include "message_verify.h"

#include <linux/errno.h>
#include <linux/limits.h>
#include <linux/statfs.h>

#include "connection.h"
#include "hmdfs.h"
#include "hmdfs_server.h"

size_t message_length[C_FLAG_SIZE][F_SIZE][HMDFS_MESSAGE_MIN_MAX];
bool need_response[F_SIZE];

void hmdfs_message_verify_init(void)
{
	int flag, cmd;

	for (cmd = 0; cmd < F_SIZE; cmd++)
		need_response[cmd] = true;
	need_response[F_RELEASE] = false;
	need_response[F_CONNECT_REKEY] = false;
	need_response[F_DROP_PUSH] = false;

	for (flag = 0; flag < C_FLAG_SIZE; flag++) {
		for (cmd = 0; cmd < F_SIZE; cmd++) {
			message_length[flag][cmd][HMDFS_MESSAGE_MIN_INDEX] = 1;
			message_length[flag][cmd][HMDFS_MESSAGE_MAX_INDEX] = 0;
			message_length[flag][cmd][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
				MESSAGE_LEN_JUDGE_RANGE;
		}
	}

	message_length[C_REQUEST][F_OPEN][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct open_request);
	message_length[C_REQUEST][F_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct open_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_OPEN][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_OPEN][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct open_response);
	message_length[C_RESPONSE][F_OPEN][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_ATOMIC_OPEN][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct atomic_open_request);
	message_length[C_REQUEST][F_ATOMIC_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct atomic_open_request) + PATH_MAX + NAME_MAX + 1;
	message_length[C_REQUEST][F_ATOMIC_OPEN][HMDFS_MESSAGE_LEN_JUDGE_INDEX]
		= MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_ATOMIC_OPEN][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_ATOMIC_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct atomic_open_response);
	message_length[C_RESPONSE][F_ATOMIC_OPEN][HMDFS_MESSAGE_LEN_JUDGE_INDEX]
		= MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_RELEASE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct release_request);
	message_length[C_REQUEST][F_RELEASE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct release_request);
	message_length[C_REQUEST][F_RELEASE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_FSYNC][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct fsync_request);
	message_length[C_REQUEST][F_FSYNC][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct fsync_request);
	message_length[C_REQUEST][F_FSYNC][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_FSYNC][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_FSYNC][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_FSYNC][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_READPAGE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct readpage_request);
	message_length[C_REQUEST][F_READPAGE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct readpage_request);
	message_length[C_REQUEST][F_READPAGE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_READPAGE][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_READPAGE][HMDFS_MESSAGE_MAX_INDEX] =
		HMDFS_PAGE_SIZE;
	message_length[C_RESPONSE][F_READPAGE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_READPAGES][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct readpages_request);
	message_length[C_REQUEST][F_READPAGES][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct readpages_request);
	message_length[C_REQUEST][F_READPAGES][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_READPAGES][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_READPAGES][HMDFS_MESSAGE_MAX_INDEX] =
		HMDFS_READPAGES_NR_MAX * HMDFS_PAGE_SIZE;
	message_length[C_RESPONSE][F_READPAGES][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_READPAGES_OPEN][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct readpages_open_request);
	message_length[C_REQUEST][F_READPAGES_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct readpages_open_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_READPAGES_OPEN][
		HMDFS_MESSAGE_LEN_JUDGE_INDEX] = MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_READPAGES_OPEN][HMDFS_MESSAGE_MIN_INDEX] =
		0;
	message_length[C_RESPONSE][F_READPAGES_OPEN][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct readpages_open_response) +
		HMDFS_READPAGES_NR_MAX * HMDFS_PAGE_SIZE;
	message_length[C_RESPONSE][F_READPAGES_OPEN][
		HMDFS_MESSAGE_LEN_JUDGE_INDEX] = MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_WRITEPAGE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct writepage_request) + HMDFS_PAGE_SIZE;
	message_length[C_REQUEST][F_WRITEPAGE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct writepage_request) + HMDFS_PAGE_SIZE;
	message_length[C_REQUEST][F_WRITEPAGE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_WRITEPAGE][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_WRITEPAGE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct writepage_response);
	message_length[C_RESPONSE][F_WRITEPAGE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_ITERATE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct readdir_request);
	message_length[C_REQUEST][F_ITERATE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct readdir_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_ITERATE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_ITERATE][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_ITERATE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(__le64) + HMDFS_MAX_MESSAGE_LEN;
	message_length[C_RESPONSE][F_ITERATE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_MKDIR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct mkdir_request);
	message_length[C_REQUEST][F_MKDIR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct mkdir_request) + PATH_MAX + NAME_MAX + 2;
	message_length[C_REQUEST][F_MKDIR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_MKDIR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct hmdfs_inodeinfo_response);
	message_length[C_RESPONSE][F_MKDIR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct hmdfs_inodeinfo_response);
	message_length[C_RESPONSE][F_MKDIR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_CREATE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct create_request);
	message_length[C_REQUEST][F_CREATE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct create_request) + PATH_MAX + NAME_MAX + 2;
	message_length[C_REQUEST][F_CREATE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_CREATE][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct hmdfs_inodeinfo_response);
	message_length[C_RESPONSE][F_CREATE][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct hmdfs_inodeinfo_response);
	message_length[C_RESPONSE][F_CREATE][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_RMDIR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct rmdir_request);
	message_length[C_REQUEST][F_RMDIR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct rmdir_request) + PATH_MAX + NAME_MAX + 2;
	message_length[C_REQUEST][F_RMDIR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_RMDIR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_RMDIR][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_RMDIR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_UNLINK][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct unlink_request);
	message_length[C_REQUEST][F_UNLINK][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct unlink_request) + PATH_MAX + NAME_MAX + 2;
	message_length[C_REQUEST][F_UNLINK][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_UNLINK][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_UNLINK][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_UNLINK][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_RENAME][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct rename_request);
	message_length[C_REQUEST][F_RENAME][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct rename_request) + 4 + 4 * PATH_MAX;
	message_length[C_REQUEST][F_RENAME][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_RENAME][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_RENAME][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_RENAME][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_SETATTR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct setattr_request);
	message_length[C_REQUEST][F_SETATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct setattr_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_SETATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_SETATTR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_SETATTR][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_SETATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_GETATTR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct getattr_request);
	message_length[C_REQUEST][F_GETATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct getattr_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_GETATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_GETATTR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_GETATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct getattr_response);
	message_length[C_RESPONSE][F_GETATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_CONNECT_ECHO][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct connection_echo_param);
	message_length[C_REQUEST][F_CONNECT_ECHO][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct connection_echo_param);
	message_length[C_REQUEST][F_CONNECT_ECHO]
		      [HMDFS_MESSAGE_LEN_JUDGE_INDEX] = MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_CONNECT_ECHO][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct connection_echo_param);
	message_length[C_RESPONSE][F_CONNECT_ECHO][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct connection_echo_param);
	message_length[C_RESPONSE][F_CONNECT_ECHO]
		      [HMDFS_MESSAGE_LEN_JUDGE_INDEX] = MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_STATFS][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct statfs_request);
	message_length[C_REQUEST][F_STATFS][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct statfs_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_STATFS][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_STATFS][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_STATFS][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct statfs_response);
	message_length[C_RESPONSE][F_STATFS][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_SYNCFS][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct syncfs_request);
	message_length[C_REQUEST][F_SYNCFS][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct syncfs_request);
	message_length[C_REQUEST][F_SYNCFS][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;
	message_length[C_RESPONSE][F_SYNCFS][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_SYNCFS][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_SYNCFS][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_GETXATTR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct getxattr_request);
	message_length[C_REQUEST][F_GETXATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct getxattr_request) + PATH_MAX + XATTR_NAME_MAX + 2;
	message_length[C_REQUEST][F_GETXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_GETXATTR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_GETXATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct getxattr_response) + HMDFS_XATTR_SIZE_MAX;
	message_length[C_RESPONSE][F_GETXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_SETXATTR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct setxattr_request);
	message_length[C_REQUEST][F_SETXATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct setxattr_request) + PATH_MAX + XATTR_NAME_MAX +
		HMDFS_XATTR_SIZE_MAX + 2;
	message_length[C_REQUEST][F_SETXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_SETXATTR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_SETXATTR][HMDFS_MESSAGE_MAX_INDEX] = 0;
	message_length[C_RESPONSE][F_SETXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_LISTXATTR][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct listxattr_request);
	message_length[C_REQUEST][F_LISTXATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct listxattr_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_LISTXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
	message_length[C_RESPONSE][F_LISTXATTR][HMDFS_MESSAGE_MIN_INDEX] = 0;
	message_length[C_RESPONSE][F_LISTXATTR][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct listxattr_response) + HMDFS_LISTXATTR_SIZE_MAX;
	message_length[C_RESPONSE][F_LISTXATTR][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;

	message_length[C_REQUEST][F_CONNECT_REKEY][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct connection_rekey_request);
	message_length[C_REQUEST][F_CONNECT_REKEY][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct connection_rekey_request);
	message_length[C_REQUEST][F_CONNECT_REKEY]
		      [HMDFS_MESSAGE_LEN_JUDGE_INDEX] = MESSAGE_LEN_JUDGE_BIN;

	message_length[C_REQUEST][F_DROP_PUSH][HMDFS_MESSAGE_MIN_INDEX] =
		sizeof(struct drop_push_request);
	message_length[C_REQUEST][F_DROP_PUSH][HMDFS_MESSAGE_MAX_INDEX] =
		sizeof(struct drop_push_request) + PATH_MAX + 1;
	message_length[C_REQUEST][F_DROP_PUSH][HMDFS_MESSAGE_LEN_JUDGE_INDEX] =
		MESSAGE_LEN_JUDGE_RANGE;
}

static void find_first_no_slash(const char **name, int *len)
{
	const char *s = *name;
	int l = *len;

	while (*s == '/' && l > 0) {
		s++;
		l--;
	}

	*name = s;
	*len = l;
}

static void find_first_slash(const char **name, int *len)
{
	const char *s = *name;
	int l = *len;

	while (*s != '/' && l > 0) {
		s++;
		l--;
	}

	*name = s;
	*len = l;
}

static bool path_contain_dotdot(const char *name, int len)
{
	while (true) {
		find_first_no_slash(&name, &len);

		if (len == 0)
			return false;

		if (len >= 2 && name[0] == '.' && name[1] == '.' &&
		    (len == 2 || name[2] == '/'))
			return true;

		find_first_slash(&name, &len);
	}
}

static int hmdfs_open_message_verify(int flag, size_t len, void *data)
{
	struct open_request *req = NULL;
	size_t tmp_len = 0;
	int path_len;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	path_len = le32_to_cpu(req->path_len);
	tmp_len = strnlen(req->buf, PATH_MAX);
	if (tmp_len == PATH_MAX ||
	    tmp_len != len - sizeof(struct open_request) - 1 ||
	    path_len != tmp_len) {
		hmdfs_err("verify fail");
		return -EINVAL;
	}

	/*
	 * We only alow server to open file in hmdfs, thus we need to
	 * make sure path don't contain "..".
	 */
	if (path_contain_dotdot(req->buf, path_len)) {
		hmdfs_err("verify fail, path contain dotdot");
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_atomic_open_verify(int flag, size_t len, void *data)
{
	struct atomic_open_request *req = NULL;
	size_t total_len;
	size_t path_len;
	size_t max_path_size;
	size_t file_len;
	size_t max_file_size;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	total_len = len - sizeof(*req);
	max_path_size = min_t(size_t, PATH_MAX, total_len);
	path_len = strnlen(req->buf, max_path_size);
	/* file name need 2 byte at least */
	if (path_len == max_path_size || path_len + 3 > total_len) {
		hmdfs_err("verify fail, len %lu, path_len %lu", len, path_len);
		return -EINVAL;
	}

	max_file_size = min_t(size_t, NAME_MAX + 1, total_len - path_len - 1);
	file_len = strnlen(req->buf + path_len + 1, max_file_size);

	if (file_len == max_file_size ||
	    total_len != path_len + 1 + file_len + 1 ||
	    le32_to_cpu(req->path_len) != path_len ||
	    le32_to_cpu(req->file_len) != file_len) {
		hmdfs_err("verify fail total len %zu path_len %zu, decalared path len %u, file_len %zu, decalared file_len %u",
			  total_len, path_len, le32_to_cpu(req->path_len),
			  file_len, le32_to_cpu(req->file_len) != file_len);
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_iterate_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct readdir_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_len = strnlen(tmp_char, PATH_MAX);
		} else {
			return err;
		}

		if (le32_to_cpu(tmp_request->path_len) != tmp_len ||
		    len - sizeof(struct readdir_request) - 1 != tmp_len) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_mkdir_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct mkdir_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_path_len = 0;
	size_t tmp_name_len = 0;
	size_t tmp_char_path_len = 0;
	size_t tmp_char_name_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_path_len = le32_to_cpu(tmp_request->path_len);
			tmp_name_len = le32_to_cpu(tmp_request->name_len);
			tmp_char_path_len = strnlen(tmp_char, PATH_MAX);
			tmp_char_name_len = strnlen(
				tmp_char + tmp_char_path_len + 1, NAME_MAX);
		} else {
			return err;
		}

		if (tmp_path_len != tmp_char_path_len ||
		    tmp_name_len != tmp_char_name_len ||
		    len - sizeof(struct mkdir_request) !=
			    tmp_path_len + 1 + tmp_name_len + 1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}
	return err;
}

static int hmdfs_create_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct create_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_path_len = 0;
	size_t tmp_name_len = 0;
	size_t tmp_char_path_len = 0;
	size_t tmp_char_name_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_path_len = le32_to_cpu(tmp_request->path_len);
			tmp_name_len = le32_to_cpu(tmp_request->name_len);
			tmp_char_path_len = strnlen(tmp_char, PATH_MAX);
			tmp_char_name_len = strnlen(
				tmp_char + tmp_char_path_len + 1, NAME_MAX);
		} else {
			return err;
		}

		if (tmp_path_len != tmp_char_path_len ||
		    tmp_name_len != tmp_char_name_len ||
		    len - sizeof(struct create_request) !=
			    tmp_path_len + 1 + tmp_name_len + 1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}
	return err;
}

static int hmdfs_rmdir_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct rmdir_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_path_len = 0;
	size_t tmp_name_len = 0;
	size_t tmp_char_path_len = 0;
	size_t tmp_char_name_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_path_len = le32_to_cpu(tmp_request->path_len);
			tmp_name_len = le32_to_cpu(tmp_request->name_len);
			tmp_char_path_len = strnlen(tmp_char, PATH_MAX);
			tmp_char_name_len = strnlen(
				tmp_char + tmp_char_path_len + 1, NAME_MAX);
		} else {
			return err;
		}

		if (tmp_path_len != tmp_char_path_len ||
		    tmp_name_len != tmp_char_name_len ||
		    len - sizeof(struct rmdir_request) !=
			    tmp_path_len + 1 + tmp_name_len + 1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_unlink_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct unlink_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_path_len = 0;
	size_t tmp_name_len = 0;
	size_t tmp_char_path_len = 0;
	size_t tmp_char_name_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_path_len = le32_to_cpu(tmp_request->path_len);
			tmp_name_len = le32_to_cpu(tmp_request->name_len);
			tmp_char_path_len = strnlen(tmp_char, PATH_MAX);
			tmp_char_name_len = strnlen(
				tmp_char + tmp_char_path_len + 1, NAME_MAX);
		} else {
			return err;
		}

		if (tmp_path_len != tmp_char_path_len ||
		    tmp_name_len != tmp_char_name_len ||
		    len - sizeof(struct unlink_request) !=
			    tmp_path_len + 1 + tmp_name_len + 1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_rename_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct rename_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_old_path_len = 0;
	size_t tmp_new_path_len = 0;
	size_t tmp_old_name_len = 0;
	size_t tmp_new_name_len = 0;
	size_t tmp_char_old_path_len = 0;
	size_t tmp_char_new_path_len = 0;
	size_t tmp_char_old_name_len = 0;
	size_t tmp_char_new_name_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;

			tmp_old_path_len =
				le32_to_cpu(tmp_request->old_path_len);
			tmp_new_path_len =
				le32_to_cpu(tmp_request->new_path_len);
			tmp_old_name_len =
				le32_to_cpu(tmp_request->old_name_len);
			tmp_new_name_len =
				le32_to_cpu(tmp_request->new_name_len);

			tmp_char_old_path_len = strnlen(tmp_char, PATH_MAX);
			tmp_char_new_path_len = strnlen(
				tmp_char + tmp_char_old_path_len + 1, PATH_MAX);

			tmp_char_old_name_len =
				strnlen(tmp_char + tmp_char_old_path_len + 1 +
						tmp_char_new_path_len + 1,
					PATH_MAX);
			tmp_char_new_name_len =
				strnlen(tmp_char + tmp_char_old_path_len + 1 +
						tmp_char_new_path_len + 1 +
						tmp_char_old_name_len + 1,
					PATH_MAX);
		} else {
			return err;
		}

		if (tmp_new_name_len != tmp_char_new_name_len ||
		    tmp_old_name_len != tmp_char_old_name_len ||
		    tmp_new_path_len != tmp_char_new_path_len ||
		    tmp_old_path_len != tmp_char_old_path_len ||
		    len - sizeof(struct rename_request) !=
			    tmp_new_name_len + 1 + tmp_old_name_len + 1 +
				    tmp_new_path_len + 1 + tmp_old_path_len +
				    1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_setattr_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct setattr_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->buf;
			tmp_len = strnlen(tmp_char, PATH_MAX);
		} else {
			return err;
		}

		if (tmp_len != len - sizeof(struct setattr_request) - 1 ||
		    le32_to_cpu(tmp_request->path_len) != tmp_len) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_getattr_verify(int flag, size_t len, void *data)
{
	struct getattr_request *req = NULL;
	size_t tmp_len;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	tmp_len = strnlen(req->buf, PATH_MAX);
	if (tmp_len != len - sizeof(struct getattr_request) - 1 ||
	    le32_to_cpu(req->path_len) != tmp_len) {
		hmdfs_err("verify fail");
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_getxattr_verify(int flag, size_t len, void *data)
{
	struct getxattr_request *req = NULL;
	struct getxattr_response *resp = NULL;
	size_t path_len = 0;
	size_t name_len = 0;
	size_t size = 0;

	if (!data)
		return 0;

	if (flag == C_REQUEST) {
		req = data;
		path_len = le32_to_cpu(req->path_len);
		name_len = le32_to_cpu(req->name_len);
		size = le32_to_cpu(req->size);
		if (path_len >= PATH_MAX ||
		    path_len != strnlen(req->buf, PATH_MAX) ||
		    name_len !=
			    strnlen(req->buf + path_len + 1, XATTR_NAME_MAX) ||
		    size > HMDFS_XATTR_SIZE_MAX)
			return -EINVAL;
	} else {
		resp = data;
		size = le32_to_cpu(resp->size);
		if (len != sizeof(struct getxattr_response) &&
		    len < sizeof(struct getxattr_response) + size)
			return -EINVAL;
	}

	return 0;
}

static int hmdfs_setxattr_verify(int flag, size_t len, void *data)
{
	struct setxattr_request *req = NULL;
	size_t path_len = 0;
	size_t name_len = 0;
	size_t size = 0;

	/* No need to verify response */
	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	path_len = le32_to_cpu(req->path_len);
	name_len = le32_to_cpu(req->name_len);
	size = le32_to_cpu(req->size);
	if (path_len >= PATH_MAX || path_len != strnlen(req->buf, PATH_MAX) ||
	    name_len != strnlen(req->buf + path_len + 1, XATTR_NAME_MAX) ||
	    len != path_len + name_len + size + 2 +
			    sizeof(struct setxattr_request) ||
	    size > HMDFS_XATTR_SIZE_MAX)
		return -EINVAL;

	return 0;
}

static int hmdfs_listxattr_verify(int flag, size_t len, void *data)
{
	struct listxattr_request *req = NULL;
	struct listxattr_response *resp = NULL;
	size_t path_len = 0;
	size_t size = 0;

	if (!data)
		return 0;

	if (flag == C_REQUEST) {
		req = data;
		path_len = le32_to_cpu(req->path_len);
		size = le32_to_cpu(req->size);
		if (path_len >= PATH_MAX ||
		    path_len != strnlen(req->buf, PATH_MAX) ||
		    size > HMDFS_LISTXATTR_SIZE_MAX)
			return -EINVAL;
	} else {
		resp = data;
		size = le32_to_cpu(resp->size);
		if (len != sizeof(struct listxattr_response) &&
		    len < sizeof(struct listxattr_response) + size)
			return -EINVAL;
	}

	return 0;
}

static int hmdfs_writepage_verify(int flag, size_t len, void *data)
{
	struct writepage_request *req = NULL;
	__u32 count;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	count = le32_to_cpu(req->count);
	if (count == 0 || count > HMDFS_PAGE_SIZE ||
	    HMDFS_PAGE_SIZE != len - sizeof(struct writepage_request)) {
		hmdfs_err("verify fail, count is %d", count);
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_statfs_verify(int flag, size_t len, void *data)
{
	int err = 0;
	struct statfs_request *tmp_request = NULL;
	char *tmp_char = NULL;
	size_t tmp_len = 0;

	if (flag == C_REQUEST) {
		if (data) {
			tmp_request = data;
			tmp_char = tmp_request->path;
			tmp_len = strnlen(tmp_char, PATH_MAX);
		} else {
			return err;
		}

		if (le32_to_cpu(tmp_request->path_len) != tmp_len ||
		    tmp_len != len - sizeof(struct statfs_request) - 1) {
			err = -EINVAL;
			hmdfs_err("verify fail");
			return err;
		}
	}

	return err;
}

static int hmdfs_readpages_verify(int flag, size_t len, void *data)
{
	struct readpages_request *req = NULL;
	unsigned int size;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	size = le32_to_cpu(req->size);
	if (size > HMDFS_READPAGES_NR_MAX * HMDFS_PAGE_SIZE) {
		hmdfs_err("verify fail, invalid req->size %u", size);
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_readpages_open_verify(int flag, size_t len, void *data)
{
	struct readpages_open_request *req = NULL;
	unsigned int size;
	size_t tmp_len;

	if (flag != C_REQUEST || !data)
		return 0;

	req = data;
	size = le32_to_cpu(req->size);
	tmp_len = strnlen(req->buf, PATH_MAX);
	if (tmp_len + 1 != len - sizeof(*req) ||
	    le32_to_cpu(req->path_len) != tmp_len ||
	    size > HMDFS_READPAGES_NR_MAX * HMDFS_PAGE_SIZE) {
		hmdfs_err("verify fail, req->size %u", size);
		return -EINVAL;
	}

	return 0;
}

typedef int (*hmdfs_message_verify_func)(int, size_t, void *);

static const hmdfs_message_verify_func message_verify[F_SIZE] = {
	[F_OPEN] = hmdfs_open_message_verify,
	[F_WRITEPAGE] = hmdfs_writepage_verify,
	[F_ITERATE] = hmdfs_iterate_verify,
	[F_MKDIR] = hmdfs_mkdir_verify,
	[F_CREATE] = hmdfs_create_verify,
	[F_RMDIR] = hmdfs_rmdir_verify,
	[F_UNLINK] = hmdfs_unlink_verify,
	[F_RENAME] = hmdfs_rename_verify,
	[F_SETATTR] = hmdfs_setattr_verify,
	[F_STATFS] = hmdfs_statfs_verify,
	[F_GETATTR] = hmdfs_getattr_verify,
	[F_GETXATTR] = hmdfs_getxattr_verify,
	[F_SETXATTR] = hmdfs_setxattr_verify,
	[F_LISTXATTR] = hmdfs_listxattr_verify,
	[F_READPAGES] = hmdfs_readpages_verify,
	[F_READPAGES_OPEN] = hmdfs_readpages_open_verify,
	[F_ATOMIC_OPEN] = hmdfs_atomic_open_verify,
};

static void handle_bad_message(struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *head, int *err)
{
	/*
	 * Bad message won't be awared by upper layer, so ETIME is
	 * always given to upper layer. It is prefer to pass ENOTSUPP
	 * to upper layer when bad message (eg. caused by wrong len)
	 * received.
	 */
	if (head->operations.cmd_flag == C_RESPONSE) {
		/*
		 * Change msg ret code. To let upper layer handle
		 * ENOTSUPP, hmdfs_message_verify() should return
		 * 0, so err code is modified either.
		 */
		head->ret_code = cpu_to_le32(-ENOTSUPP);
		*err = 0;
	} else {
		if (head->operations.command >= F_SIZE)
			return;
		/*
		 * Some request messages do not need to be responded.
		 * Even if a response is returned, the response msg
		 * is automatically ignored in hmdfs_response_recv().
		 * Therefore, it is normal to directly return a response.
		 */
		if (need_response[head->operations.command])
			hmdfs_send_err_response(con, head, -ENOTSUPP);
	}
}

int hmdfs_message_verify(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			 void *data)
{
	int err = 0;
	int flag, cmd, len_type;
	size_t len, min, max;

	if (!head)
		return -EINVAL;

	flag = head->operations.cmd_flag;
	if (flag != C_REQUEST && flag != C_RESPONSE)
		return -EINVAL;

	cmd = head->operations.command;
	if (cmd >= F_SIZE || cmd < F_OPEN || cmd == F_RESERVED_0 ||
	    (cmd >= F_RESERVED_1 && cmd <= F_RESERVED_4)) {
		err = -EINVAL;
		goto handle_bad_msg;
	}

	if (head->version == DFS_2_0) {
		len = le32_to_cpu(head->data_len) -
		      sizeof(struct hmdfs_head_cmd);
		min = message_length[flag][cmd][HMDFS_MESSAGE_MIN_INDEX];
		if (head->operations.command == F_ITERATE && flag == C_RESPONSE)
			max = sizeof(struct slice_descriptor) + PAGE_SIZE;
		else
			max = message_length[flag][cmd][HMDFS_MESSAGE_MAX_INDEX];
		len_type =
			message_length[flag][cmd][HMDFS_MESSAGE_LEN_JUDGE_INDEX];

		if (len_type == MESSAGE_LEN_JUDGE_RANGE) {
			if (len < min || len > max) {
				hmdfs_err(
					"cmd %d -> %d message verify fail, len = %lu",
					cmd, flag, len);
				err = -EINVAL;
				goto handle_bad_msg;
			}
		} else {
			if (len != min && len != max) {
				hmdfs_err(
					"cmd %d -> %d message verify fail, len = %lu",
					cmd, flag, len);
				err = -EINVAL;
				goto handle_bad_msg;
			}
		}

		if (message_verify[cmd])
			err = message_verify[cmd](flag, len, data);

		if (err)
			goto handle_bad_msg;

		return err;
	}

handle_bad_msg:
	if (err) {
		handle_bad_message(con, head, &err);
		return err;
	}

	if (head->version == DFS_1_0)
		return err; // now, DFS_1_0 version do not verify

	return -EINVAL;
}

#include "buffered_log_sender.h"

#include <linux/net.h>
#include <linux/un.h>
#include <linux/vmalloc.h>

#include "dubai_utils.h"

#define DUBAID_SOCKET_NAME			"DUBAI_BUFFERED_LOG"
#define BUFFERED_LOG_TYPE			"dubai_driver"
#define MAX_DATA_TYPE_LENGTH		32
// limit log under 50MB
#define MAX_LOG_LENGTH				52428800
#define dubai_is_valid_length(len)	(((len) >= 0) && ((len) <= MAX_LOG_LENGTH))

static DEFINE_MUTEX(sock_lock);
static struct socket *dubaid_sock = NULL;
static int do_send_buffered_log(const struct buffered_log_entry *entry);
static int send_buffered_log_init(const struct buffered_log_entry *entry);
int (*send_buffered_log)(const struct buffered_log_entry *entry) = send_buffered_log_init;

static int write_to_daemon(const struct buffered_log_entry *entry)
{
	size_t payload_size = 0;
	struct msghdr msg;
	struct kvec vec[3];

	if (dubaid_sock == NULL || entry == NULL) {
		dubai_err("Invalid parameter");
		return -EINVAL;
	}
	if (unlikely(!dubai_is_valid_length(entry->length))) {
		dubai_err("Invalid log entry length: %d, magic: %d", entry->length, entry->magic);
		return -EINVAL;
	}

	vec[0].iov_base = (void *) &(entry->length);
	vec[0].iov_len = sizeof(entry->length);
	payload_size += vec[0].iov_len;
	vec[1].iov_base = (void *) &(entry->magic);
	vec[1].iov_len = sizeof(entry->magic);
	payload_size += vec[1].iov_len;
	vec[2].iov_base = (void *) entry->data;
	vec[2].iov_len = (size_t) entry->length;
	payload_size += vec[2].iov_len;
	memset(&msg, 0, sizeof(msg));

	return kernel_sendmsg(dubaid_sock, &msg, vec, 3, payload_size);
}

// Send data type to dubaid socket server to authenticate this socket
static int authenticate_data_type(int magic)
{
	char buf[MAX_DATA_TYPE_LENGTH] = {0};
	struct buffered_log_entry *entry = NULL;

	entry = (struct buffered_log_entry *) buf;
	entry->length = strlen(BUFFERED_LOG_TYPE);
	entry->magic = magic;
	memcpy(entry->data, BUFFERED_LOG_TYPE, entry->length);

	return write_to_daemon(entry);
}

static int make_sockaddr_un(const char *name, struct sockaddr_un *un, int *alen)
{
	size_t name_len;

	memset(un, 0, sizeof(*un));
	name_len = strlen(name);
	if ((name_len + 1) > sizeof(un->sun_path)) {
		dubai_err("Invalid name length");
		return -EINVAL;
	}
	// The path in this case is not supposed to be '\0'-terminated
	un->sun_path[0] = 0;
	memcpy(un->sun_path + 1, name, name_len);
	un->sun_family = AF_LOCAL;
	*alen = name_len + offsetof(struct sockaddr_un, sun_path) + 1;

	return 0;
}

/*
 * Try to connect dubaid local socket server and authenticate data type
 * Connect success if both operations were done
 */
static int connect_dubaid_server(int magic)
{
	int ret, alen = 0;
	struct sockaddr_un un;

	if (dubaid_sock != NULL)
		return 0;

	ret = sock_create(AF_LOCAL, SOCK_STREAM, 0, &dubaid_sock);
	if (ret < 0 || dubaid_sock == NULL) {
		dubai_err("Failed to create socket");
		goto error;
	}
	ret = make_sockaddr_un(DUBAID_SOCKET_NAME, &un, &alen);
	if (ret < 0) {
		dubai_err("Failed to make socket address");
		goto error;
	}
	ret = kernel_connect(dubaid_sock, (struct sockaddr *)&un, alen, O_CLOEXEC);
	if (ret < 0) {
		dubai_err("Failed to connect socket");
		goto error;
	}
	ret = authenticate_data_type(magic);
	if (ret < 0) {
		dubai_err("Failed to authenticate data type");
		goto error;
	}
	dubai_info("Succeed to connect dubai local socket server");

	return 0;

error:
	if (dubaid_sock != NULL) {
		sock_release(dubaid_sock);
		dubaid_sock = NULL;
	}

	return -ECONNREFUSED;
}

// Initilize socket to connect dubaid local socket server
static int send_buffered_log_init(const struct buffered_log_entry *entry)
{
	int ret;

	if (entry == NULL) {
		dubai_err("Invalid parameter");
		return -EINVAL;
	}

	mutex_lock(&sock_lock);
	if (send_buffered_log == send_buffered_log_init) {
		// initialize dubaid socket for the first time
		ret = connect_dubaid_server(entry->magic);
		if (ret < 0) {
			// Retry to connect to dubaid local socket server
			ret = connect_dubaid_server(entry->magic);
			if (ret < 0) {
				mutex_unlock(&sock_lock);
				return -ECONNREFUSED;
			}
		}
		send_buffered_log = do_send_buffered_log;
	}
	mutex_unlock(&sock_lock);

	return do_send_buffered_log(entry);
}

static int do_send_buffered_log(const struct buffered_log_entry *entry)
{
	int ret;

	if (entry == NULL) {
		dubai_err("Invalid parameter");
		return -EINVAL;
	}

	mutex_lock(&sock_lock);
	ret = write_to_daemon(entry);
	if (ret < 0) {
		if (dubaid_sock != NULL) {
			sock_release(dubaid_sock);
			dubaid_sock = NULL;
		} else {
			dubai_err("Unexpectedly! dubaid_sock is null!");
		}
		dubai_err("Failed to write to daemon: %d", ret);
		send_buffered_log = send_buffered_log_init;
	}
	mutex_unlock(&sock_lock);

	return ret;
}

// Warnning: Using vmalloc to allocate memory, so this fuction might sleep
struct buffered_log_entry *create_buffered_log_entry(long long size, int magic)
{
	unsigned long entry_size;
	struct buffered_log_entry *entry = NULL;

	might_sleep();

	if (!dubai_is_valid_length(size)) {
		dubai_err("Invalid log size: %lld, magic: %d", size, magic);
		return NULL;
	}

	entry_size = size + sizeof(struct buffered_log_entry);
	entry = vmalloc(entry_size);
	if (entry == NULL) {
		dubai_err("Failed to allocate memory");
		return NULL;
	}
	memset(entry, 0, entry_size);
	entry->length = size;
	entry->magic = magic;

	return entry;
}

void free_buffered_log_entry(struct buffered_log_entry *entry)
{
	if (entry)
		vfree(entry);
}

void buffered_log_release(void)
{
	mutex_lock(&sock_lock);
	if (dubaid_sock != NULL)
		sock_release(dubaid_sock);
	mutex_unlock(&sock_lock);
}

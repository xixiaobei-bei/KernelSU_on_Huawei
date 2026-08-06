
#ifndef _TC_NS_CLIENT_DRIVER_SUB_H_
#define _TC_NS_CLIENT_DRIVER_SUB_H_
#define INVALID_TYPE	0xff
#define HIDL_SIDE	0x01
#define NON_HIDL_SIDE	0x02
#define CHECK_PATH_HASH_FAIL  0xff01
#define CHECK_SECLABEL_FAIL   0xff02
#define CHECK_CODE_HASH_FAIL  0xff03
#define ENTER_BYPASS_CHANNEL  0xff04
#define BUF_MAX_SIZE 1024
#define MAX_PATH_SIZE 512
#define SHA256_DIGEST_LENTH 32

int check_mm_struct(struct mm_struct *mm);
char *get_process_path(struct task_struct *task, char *tpath);
int calc_process_path_hash(unsigned char *data, unsigned long len, char *digest);
int pack_ca_cert(int type, char *ca_cert, char *path, struct task_struct *ca_task, const struct cred *cred);
int check_process_selinux_security(struct task_struct *ca_task, char *context);
tc_ns_service *tc_find_service_in_dev(tc_ns_dev_file *dev, unsigned char *uuid, bool ref);
tc_ns_service *tc_find_service_from_all(unsigned char *uuid);
int add_service_to_dev(tc_ns_dev_file *dev, tc_ns_service *service);
void del_service_from_dev(tc_ns_dev_file *dev, tc_ns_service *service);
tc_ns_session *tc_find_session_withowner(struct list_head *session_list,
				  unsigned int session_id, tc_ns_dev_file *dev_file);
void dump_services_status(char *param);
errno_t init_context(tc_ns_client_context *context, unsigned char *uuid);
#ifdef SECURITY_AUTH_ENHANCE
int generate_random_data(uint8_t *data, uint32_t size);
bool is_valid_encryption_head(const struct encryption_head *head, const uint8_t *data, uint32_t len);
int generate_challenge_word(uint8_t *challenge_word, uint32_t size);
int set_encryption_head(struct encryption_head *head, const uint8_t *data, uint32_t len);
tc_ns_session *tc_find_session2(unsigned int dev_file_id, unsigned int context_id, unsigned char *uuid);
void clean_session_secure_information(tc_ns_session *session);
int get_session_secure_params(tc_ns_dev_file *dev_file, tc_ns_client_context *context, tc_ns_session *session);
#endif
int close_session(tc_ns_dev_file *dev, tc_ns_session *session, unsigned char *uuid, unsigned int session_id);
void kill_session(tc_ns_dev_file *dev, unsigned char *uuid,
			unsigned int session_id);
int tc_ns_service_init(unsigned char *uuid, tc_ns_service **new_service);
uint32_t tc_ns_get_uid(void);
int tee_init_crypto(char *hash_type);
int get_pack_name_len(tc_ns_dev_file *dev_file, uint8_t *cert_buffer);
int get_public_key_len(tc_ns_dev_file *dev_file, uint8_t *cert_buffer);
bool is_valid_ta_size(char *file_buffer, unsigned int file_size);
int tc_ns_need_load_image(unsigned int file_id, unsigned char *uuid);
int load_ta_image(tc_ns_dev_file *dev_file, tc_ns_client_context *context);
void release_free_session(tc_ns_dev_file *dev_file, tc_ns_client_context *context, tc_ns_session *session);
void close_session_in_service_list(tc_ns_dev_file *dev, tc_ns_service *service, uint32_t i);
void close_unclosed_session(tc_ns_dev_file *dev, uint32_t i);
int security_context_str_to_sid(const char *scontext, u32 *out_sid, gfp_t gfp);
void del_dev_node(tc_ns_dev_file *dev);
int tc_ns_tui_event(tc_ns_dev_file *dev_file, void *argp);
int ns_client_close_teecd_not_agent(tc_ns_dev_file *dev);



#endif

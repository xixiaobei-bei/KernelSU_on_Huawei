/*noc err bus*/
enum noc_error_bus {
	 NOC_ERRBUS_SYS_CONFIG,
	 NOC_ERRBUS_VCODEC,
	 NOC_ERRBUS_VIVO,
	 NOC_ERRBUS_NPU,
	 NOC_ERRBUS_FCM,
	 NOC_ERRBUS_BOTTOM,
};

struct noc_err_para_s {
	u32 masterid;
	u32 targetflow;
	enum noc_error_bus bus;
};

struct noc_mid_modid_trans_s {
	struct list_head s_list;
	struct noc_err_para_s err_info_para;
	u32 modid;
	void* reserve_p;
};
void noc_modid_register(struct noc_err_para_s noc_err_info,u32 modid);


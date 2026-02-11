#ifndef __HISI_SVM_H
#define __HISI_SVM_H
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mmu_notifier.h>

enum smmu_evt_type {
	PG_FAULT,
	HW_SERROR
};

struct smmu_evt_msg {
	unsigned long fault_ss;
	unsigned long fault_reason;
	unsigned long fault_addr;
	unsigned long fault_resv;
};

struct hisi_svm {
	char                *name;
	struct device       *dev;
	struct mm_struct    *mm;
	struct iommu_domain *dom;
	unsigned long        l2addr;
	struct dentry       *debug_root;
	pid_t                pid;
};

struct hisi_aicpu_irq_info {
	u64 pgfault_asid_addr;
	u64 pgfault_va_addr;
	int (*callback)(void *);/*reserved*/
	u64 cookie;/*reserved*/
};

int hisi_smmu_poweron(int smmuid);
int hisi_smmu_poweroff(int smmuid);
void hisi_svm_unbind_task(struct hisi_svm *svm);
struct hisi_svm *hisi_svm_bind_task(struct device *dev, struct task_struct *task);
void *hisi_svm_get_l2buf_pte(struct hisi_svm *svm, unsigned long addr);
void hisi_svm_show_pte(struct hisi_svm *svm, unsigned long addr, size_t size);
int hisi_svm_get_ssid(struct hisi_svm *svm, u16 *ssid,  u64 *ttbr, u64 *tcr);
int hisi_svm_flush_cache(struct mm_struct *mm,
	unsigned long addr, size_t size);
int hisi_aicpu_irq_offset_register(struct hisi_aicpu_irq_info info);
int hisi_svm_flag_set(struct task_struct *task, u32 flag);

int hisi_smmu_evt_register_notify(struct notifier_block *n);
int hisi_smmu_evt_unregister_notify(struct notifier_block *n);

#endif

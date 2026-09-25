#include <linux/export.h>
#include <linux/hisi/hisi_powerkey_event.h>
#include <linux/notifier.h>

static ATOMIC_NOTIFIER_HEAD(hisi_powerkey_notifier_list);

int hisi_powerkey_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&hisi_powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(hisi_powerkey_register_notifier);

int hisi_powerkey_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(
		&hisi_powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(hisi_powerkey_unregister_notifier);

int hisi_call_powerkey_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&hisi_powerkey_notifier_list,
		val, v);
}
EXPORT_SYMBOL_GPL(hisi_call_powerkey_notifiers);

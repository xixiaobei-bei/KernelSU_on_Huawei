#include <linux/err.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include "policy/allowlist.h"
#include "klog.h" // IWYU pragma: keep
#include "runtime/ksud_boot.h"
#include "runtime/ksud.h"
#include "manager/manager_observer.h"
#include "manager/throne_tracker.h"

bool ksu_module_mounted __read_mostly = false;
bool ksu_boot_completed __read_mostly = false;

extern void stop_input_hook();
extern void ksu_avc_spoof_late_init();

void on_post_fs_data(void)
{
        static bool done = false;
        if (done) {
                pr_info("on_post_fs_data already done\n");
                return;
        }
        done = true;
        pr_info("on_post_fs_data!\n");
        ksu_load_allow_list();
        ksu_observer_init();
        // sanity check, this may influence the performance
        stop_input_hook();
}

/*
 * ext4_unregister_sysfs exists in this kernel but is not exported via
 * EXPORT_SYMBOL, so a direct extern declaration causes an "undefined
 * reference" link error when KernelSU is built as a module.
 *
 * Fix: resolve the symbol at runtime with kallsyms_lookup_name().
 * If the symbol is not found (e.g. CONFIG_KALLSYMS=n or different
 * kernel), nuke_ext4_sysfs() returns -ENOENT gracefully.
 */
typedef void (*ext4_unregister_sysfs_fn)(struct super_block *sb);

int nuke_ext4_sysfs(const char *mnt)
{
        struct path path;
        struct super_block *sb;
        const char *name;
        ext4_unregister_sysfs_fn fn;
        int err;

        err = kern_path(mnt, 0, &path);
        if (err) {
                pr_err("nuke path err: %d\n", err);
                return err;
        }

        sb = path.dentry->d_inode->i_sb;
        name = sb->s_type->name;

        if (strcmp(name, "ext4") != 0) {
                pr_info("nuke but filesystem isn't ext4\n");
                path_put(&path);
                return -EINVAL;
        }

        /* Dynamically look up the unexported symbol */
        fn = (ext4_unregister_sysfs_fn)kallsyms_lookup_name("ext4_unregister_sysfs");
        if (!fn) {
                pr_err("nuke_ext4_sysfs: symbol not found, skipping\n");
                path_put(&path);
                return -ENOENT;
        }

        fn(sb);
        path_put(&path);
        return 0;
}

void on_module_mounted(void)
{
        pr_info("on_module_mounted!\n");
        ksu_module_mounted = true;
}

void on_boot_completed(void)
{
    ksu_boot_completed = true;
    pr_info("on_boot_completed!\n");
    track_throne(true);
    ksu_avc_spoof_late_init();
}

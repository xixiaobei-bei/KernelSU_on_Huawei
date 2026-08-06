/*
 * powerkey_event.c - powerkey notify
 *
 * Copyright (C) 2013 Hisilicon Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/export.h>
#include <linux/hisi/powerkey_event.h>
#include <linux/notifier.h>

static ATOMIC_NOTIFIER_HEAD(powerkey_notifier_list);

int powerkey_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(powerkey_register_notifier);

int powerkey_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(
		&powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(powerkey_unregister_notifier);

int call_powerkey_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&powerkey_notifier_list,
		val, v);
}
EXPORT_SYMBOL_GPL(call_powerkey_notifiers);

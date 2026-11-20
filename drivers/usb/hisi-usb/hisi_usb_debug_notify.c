/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hiusb debug framework notify chain
 * Create: 2020-08-13
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/hisi/usb/chip_usb_debug_framework.h>

static ATOMIC_NOTIFIER_HEAD(usb_atomic_error_notifier_list);

/**
 * usb_blockerr_register_notify - register a atomic error eventnotifier callback
 * whenever a usb driver error event happens
 * @nb: pointer to the notifier block for the callback events.
 *
 */
int usb_atomicerr_register_notify(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&usb_atomic_error_notifier_list, nb);
}

/**
 * usb_unregister_notify - unregister a atomic error eventnotifier callback
 * @nb: pointer to the notifier block for the callback events.
 *
 * usb_register_notify() must have been previously called for this function
 * to work properly.
 */
int usb_atomicerr_unregister_notify(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&usb_atomic_error_notifier_list, nb);
}

int usb_notify_atomicerr_occur(unsigned int event_type)
{
	return atomic_notifier_call_chain(&usb_atomic_error_notifier_list, event_type, NULL);
}

static BLOCKING_NOTIFIER_HEAD(usb_block_error_notifier_list);

/**
 * usb_blockerr_register_notify - register a block error eventnotifier callback
 * whenever a usb driver error event happens
 * @nb: pointer to the notifier block for the callback events.
 *
 */
int usb_blockerr_register_notify(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&usb_block_error_notifier_list, nb);
}

/**
 * usb_unregister_notify - unregister a block error eventnotifier callback
 * @nb: pointer to the notifier block for the callback events.
 *
 * usb_register_notify() must have been previously called for this function
 * to work properly.
 */
int usb_blockerr_unregister_notify(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&usb_block_error_notifier_list, nb);
}

int usb_notify_blockerr_occur(unsigned int event_type)
{
	return blocking_notifier_call_chain(&usb_block_error_notifier_list, event_type, NULL);
}

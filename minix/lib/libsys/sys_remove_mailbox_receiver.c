/*
 * sys_remove_mailbox_receiver.c - Implements remove mailbox receiver system call.
 *
 *  Created on: Nov 15, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include <unistd.h>
#include "syslib.h"

int sys_remove_mailbox_receiver(int mboxd, uid_t receiver_uid) {
	message m;
	m.m_mailbox_update_acl.caller_uid = getuid();
	m.m_mailbox_update_acl.mboxd = mboxd;
	m.m_mailbox_update_acl.uid = receiver_uid;
	return _kernel_call(SYS_REMOVE_MAILBOX_RECEIVER, &m);
}

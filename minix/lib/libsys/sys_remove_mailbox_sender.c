/*
 * sys_remove_mailbox_sender.c - Implements remove mailbox sender system call.
 *
 *  Created on: Nov 15, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include <unistd.h>
#include "syslib.h"

int sys_remove_mailbox_sender(int mboxd, uid_t sender_uid) {
	message m;
	m.m_mailbox_update_acl.caller_uid = getuid();
	m.m_mailbox_update_acl.mboxd = mboxd;
	m.m_mailbox_update_acl.uid = sender_uid;
	return _kernel_call(SYS_REMOVE_MAILBOX_SENDER, &m);
}

/*
 * sys_add_owner.c - Implements add owner system call.
 *
 *  Created on: Nov 15, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include <unistd.h>
#include "syslib.h"

int sys_add_owner(uid_t owner_uid) {
	message m;
	m.m_mailbox_update_acl.caller_uid = getuid();
	m.m_mailbox_update_acl.uid = owner_uid;
	return _kernel_call(SYS_ADD_OWNER, &m);
}

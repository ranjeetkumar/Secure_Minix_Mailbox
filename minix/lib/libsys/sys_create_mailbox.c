/*
 * sys_create_mailbox.c - Implements create mailbox system call.
 *
 *  Created on: Oct 13, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include <unistd.h>
#include "syslib.h"

int sys_create_mailbox(int mailbox_type, int *mboxd) {
	message m;
	m.m_mailbox_create.caller_uid = getuid();
	m.m_mailbox_create.mailbox_type = mailbox_type;
	int status = _kernel_call(SYS_CREATE_MAILBOX, &m);
	if(status == OK) {
		*mboxd = m.m_mailbox_create.mboxd;
	}

	return status;
}

/*
 * do_remove_owner.c - Implements remove owner kernel call.
 *
 *  Created on: Nov 15, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include "kernel/system.h"
#include "kernel/post_office.h"
#include <minix/endpoint.h>
#include <minix/sysutil.h>

int do_remove_owner(struct proc *caller_prt, message *m_ptr) {
	return remove_owner(m_ptr->m_mailbox_update_acl.caller_uid, m_ptr->m_mailbox_update_acl.uid);
}

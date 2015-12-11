/*
 * post_office.c - Implements various functions for mailbox.
 *
 *  Created on: Oct 13, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 *
 */

#include <sys/errno.h>
#include <minix/sysutil.h>
#include <string.h>
#include "post_office.h"

post_office po; // This will allocate memory for the post office, mailboxes, and messages

// Internal functions used by post office
int garbage_collector(int mboxd);
int message_index_minorder(int mboxd, int dest_pid);
int reordering(int order_number, int mboxd);
int min(int num1, int num2);
int is_legal_owner(uid_t caller_uid);
int is_legal_mailbox_owner(int mboxd, uid_t caller_uid);
int is_legal_mailbox_sender(int mboxd, uid_t caller_uid);
int is_legal_mailbox_receiver(int mboxd, uid_t caller_uid);

/**
 * Initializes post office.
 */
int init_post_office() {
	// Initialize number of used mail box slots in post office to 0
	po.num_mailboxes = 0;
	// Initialize number of used messages and availability flag in each mailbox of the post office
	for (int i = 0; i < MAX_MAILBOXES; i++) {
		po.mbox[i].num_messages = 0;
		po.mbox_availability[i] = TRUE; // set availability of mailbox to TRUE
	}
	// Set the owner's list to be EMPTY
	for(int i = 0; i < MAX_OWNERS; i++) {
		po.owners_acl[i] = EMPTY;
	}
	printf("Post office initialized\n");
	return OK;
}

/**
 * Adds a user as owner in post office.
 */
int add_owner(uid_t caller_uid, uid_t owner_uid) {
	// Validate if caller is authorized to add owner
	if(caller_uid != SUPERUSER_UID) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	// Find if owner already exists
	int i;
	for(i = 0; i < MAX_OWNERS; i++) {
		if(po.owners_acl[i] == owner_uid) {
			return POST_OFFICE_OWNER_EXISTS;
		}
	}

	// Find an empty slot to add owner
	for(i = 0; i < MAX_OWNERS; i++) {
		if(po.owners_acl[i] == owner_uid) {
			return POST_OFFICE_OWNER_EXISTS;
		}

		if(po.owners_acl[i] == EMPTY) {
			break;
		}
	}

	if(i == MAX_OWNERS) {
		return POST_OFFICE_MAX_OWNERS;
	}

	po.owners_acl[i] = owner_uid;
	return OK;
}

/**
 * Remove a user as owner in post office.
 */
int remove_owner(uid_t caller_uid, uid_t owner_uid) {
	// Validate if caller is authorized to add owner
	if(caller_uid != SUPERUSER_UID) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	// Find if the given owner exists in owners list
	int i;
	for(i = 0; i < MAX_OWNERS; i++) {
		if(po.owners_acl[i] == owner_uid) {
			// Remove the owner found
			po.owners_acl[i] = EMPTY;
			break;
		}
	}

	if(i == MAX_OWNERS) {
		return POST_OFFICE_OWNER_NOT_FOUND;
	}

	return OK;
}

/**
 * Creates/allocates a new mailbox and stores/returns a corresponding mailbox descriptor in mboxd.
 */
int create_mailbox(uid_t caller_uid, int mailbox_type, int *mboxd) {
	// Validate if caller is a legal owner
	if(!is_legal_owner(caller_uid)) {
		*mboxd = -1;
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	// Check is a new mail box can be created i.e. post office is full
	if(po.num_mailboxes == MAX_MAILBOXES){
		*mboxd = -1;
		return POST_OFFICE_FULL;
	}

	for(int i = 0; i < MAX_MAILBOXES; i++) {
		// See if mbox at index is available; return it as new mbox descriptor
		if (po.mbox_availability[i] == TRUE) {
			*mboxd = i; // return index as new mbox descriptor
			po.mbox[i].owner_uid = caller_uid; // Set owner of mailbox to user ID
			po.mbox[i].mailbox_type = mailbox_type; // Set mailbox type

			// Reset ACL for senders
			int j;
			for (j = 0; j < MAX_MAILBOX_SENDERS; j++) {
				po.mbox[i].senders_acl[j] = EMPTY;
			}
			// Reset ACL for receivers
			for (j = 0; j < MAX_MAILBOX_RECEIVERS; j++) {
				po.mbox[i].receivers_acl[j] = EMPTY;
			}

			// If secure mailbox, add self as sender and receiver
			if(mailbox_type == MAILBOX_TYPE_SECURE) {
				po.mbox[i].senders_acl[0] = caller_uid;
				po.mbox[i].senders_count = 1;

				po.mbox[i].receivers_acl[0] = caller_uid;
				po.mbox[i].receivers_count = 1;
			}

			// Initialize message slots in new mailbox
			for(j = 0; j < MAX_MAILBOX_MESSAGES; j++) {
				po.mbox[i].message_empty[j] = TRUE;
				po.mbox[i].msg[j].order_number = START;
				for (int k = 0; k < MAX_MAIL_DESTS; k++) {
					po.mbox[i].msg[j].dest[k] = EMPTY;
				}
			}
			po.mbox_availability[i] = FALSE; // make mailbox slot unavailable
			po.num_mailboxes++; // Increment number of mailboxes in the post office
			break;
		}
	}
	return OK;
}

/**
 * Adds a user as sender of given mailbox.
 */
int add_mailbox_sender(uid_t caller_uid, int mboxd, uid_t sender_uid) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is a legal mailbox owner
	if(!is_legal_mailbox_owner(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	if(po.mbox[mboxd].senders_count == MAX_MAILBOX_SENDERS) {
		return MAILBOX_SENDERS_ACL_FULL;
	}

	// Find if sender already exists
	for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
		if(po.mbox[mboxd].senders_acl[i] == sender_uid) {
			return MAILBOX_SENDER_EXISTS;
		}
	}

	for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
		if(po.mbox[mboxd].senders_acl[i] == EMPTY) {
			po.mbox[mboxd].senders_count++;
			po.mbox[mboxd].senders_acl[i] = sender_uid;
			break;
		}
	}

	return OK;
}

/**
 * Removes a user as sender of given mailbox.
 */
int remove_mailbox_sender(uid_t caller_uid, int mboxd, uid_t sender_uid) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is a legal mailbox owner
	if(!is_legal_mailbox_owner(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	if(po.mbox[mboxd].senders_count == 0) {
		return MAILBOX_SENDER_NOT_FOUND;
	}

	int i;
	for(i = 0; i < MAX_MAILBOX_SENDERS; i++) {
		if(po.mbox[mboxd].senders_acl[i] == sender_uid) {
			po.mbox[mboxd].senders_acl[i] = EMPTY;
			po.mbox[mboxd].senders_count--;
			break;
		}
	}

	if(i == MAX_MAILBOX_SENDERS) {
		return MAILBOX_SENDER_NOT_FOUND;
	}

	return OK;
}

/**
 * Adds a user as receiver of given mailbox.
 */
int add_mailbox_receiver(uid_t caller_uid, int mboxd, uid_t receiver_uid) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is a legal mailbox owner
	if(!is_legal_mailbox_owner(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	if(po.mbox[mboxd].receivers_count == MAX_MAILBOX_RECEIVERS) {
		return MAILBOX_RECEIVERS_ACL_FULL;
	}

	// Find if receiver already exists
	for(int i = 0; i < MAX_MAILBOX_RECEIVERS; i++) {
		if(po.mbox[mboxd].receivers_acl[i] == receiver_uid) {
			return MAILBOX_RECEIVER_EXISTS;
		}
	}

	for(int i = 0; i < MAX_MAILBOX_RECEIVERS; i++) {
		if(po.mbox[mboxd].receivers_acl[i] == EMPTY) {
			po.mbox[mboxd].receivers_count++;
			po.mbox[mboxd].receivers_acl[i] = receiver_uid;
			break;
		}
	}

	return OK;
}

/**
 * Removes a user as receiver of given mailbox.
 */
int remove_mailbox_receiver(uid_t caller_uid, int mboxd, uid_t receiver_uid) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is a legal mailbox owner
	if(!is_legal_mailbox_owner(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	if(po.mbox[mboxd].receivers_count == 0) {
		return MAILBOX_RECEIVER_NOT_FOUND;
	}

	int i;
	for(i = 0; i < MAX_MAILBOX_RECEIVERS; i++) {
		if(po.mbox[mboxd].receivers_acl[i] == receiver_uid) {
			po.mbox[mboxd].receivers_acl[i] = EMPTY;
			po.mbox[mboxd].receivers_count--;
			break;
		}
	}

	if(i == MAX_MAILBOX_RECEIVERS) {
		return MAILBOX_RECEIVER_NOT_FOUND;
	}

	return OK;
}

/**
 * Deposits a message msg into mailbox having given descriptor mboxd, which is sent by process
 * src to processes dest.
 */
int deposit_mail(uid_t caller_uid, int mboxd, endpoint_t src, endpoint_t dest[], char *msg) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is legal mailbox sender
	if(!is_legal_mailbox_sender(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	int num_messages = po.mbox[mboxd].num_messages;
	if (num_messages >= MAX_MAILBOX_MESSAGES) {
		return MAILBOX_FULL;
	} else {
		int vacant_position;
		for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
			if (po.mbox[mboxd].message_empty[i] == TRUE) {
				vacant_position = i;
				break;
			}
		}
		int max_order_number = START;
		for(int i= 0; i < MAX_MAILBOX_MESSAGES; i++) {
			if(po.mbox[mboxd].msg[i].order_number > max_order_number)
			 max_order_number = po.mbox[mboxd].msg[i].order_number;
		}
		max_order_number = max_order_number + 1;
		memcpy(po.mbox[mboxd].msg[vacant_position].msg, msg, MAX_MAIL_PAYLOAD_SIZE);
		po.mbox[mboxd].msg[vacant_position].src = src;
		po.mbox[mboxd].message_empty[vacant_position] = FALSE;
		po.mbox[mboxd].msg[vacant_position].order_number = max_order_number;
		for(int i = 0; i < MAX_MAIL_DESTS;i++) {
			po.mbox[mboxd].msg[vacant_position].dest[i] = dest[i];
		}
		num_messages = num_messages + 1;
		po.mbox[mboxd].num_messages = num_messages;
		return OK;
	}
}

/**
 * Retrieves a message msg from mailbox having given descriptor mboxd, which is sent to the given
 * process dest.
 */
int retrieve_mail(uid_t caller_uid, int mboxd, endpoint_t dest, char *msg) {
	// Check if mailbox is valid
	if(mboxd < 0 || mboxd >= MAX_MAILBOXES || po.mbox_availability[mboxd] == TRUE) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is legal mailbox receiver
	if(!is_legal_mailbox_receiver(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	if(po.mbox[mboxd].num_messages == 0) {
		return MAILBOX_EMPTY;
	}

	int message_index = message_index_minorder(mboxd, dest);

	if (message_index == INVALID) {
		return MAILBOX_NO_MESSAGE;
	} else {
		for(int j = 0; j <MAX_MAIL_DESTS; j++) {
			if(po.mbox[mboxd].msg[message_index].dest[j] == dest) {
				memcpy(msg, po.mbox[mboxd].msg[message_index].msg, MAX_MAIL_PAYLOAD_SIZE);
				po.mbox[mboxd].msg[message_index].dest[j] = EMPTY;
				garbage_collector(mboxd);
			}
		}
	}
	return OK;
}

/**
 * Removes the mailbox with given descriptor.
 */
int remove_mailbox(uid_t caller_uid, int mboxd) {
	// Index of mailbox mentioned does not exist in the post office
	if (mboxd < 0 || mboxd >= MAX_MAILBOXES) {
		return MAILBOX_NOT_FOUND;
	}

	// Validate if caller is a legal owner
	if(!is_legal_mailbox_owner(mboxd, caller_uid)) {
		return POST_OFFICE_USER_NOT_AUTHORIZED;
	}

	po.mbox[mboxd].num_messages = START; // Reset number of messages in that mailbox to be zero; Other fields will be overwritten by subsequent mailboxes that use this postoffice slot
	po.mbox_availability[mboxd] = TRUE; // Set availability to TRUE
	po.num_mailboxes--; // Decrease number of mailboxes in the post office

	po.mbox[mboxd].mailbox_type = INVALID; // Set the mailbox type to invalid
	po.mbox[mboxd].owner_uid = INVALID; // Set owner ID to invalid

	// Reset ACL for senders
	for (int j = 0; j < MAX_MAILBOX_SENDERS; j++) {
		po.mbox[mboxd].senders_acl[j] = EMPTY;
	}
	po.mbox[mboxd].senders_count = START;

	// Reset ACL for receivers
	for (int j = 0; j < MAX_MAILBOX_RECEIVERS; j++) {
		po.mbox[mboxd].receivers_acl[j] = EMPTY;
	}
	po.mbox[mboxd].receivers_count = START;

	return OK;
}

/**
 * Destroy post office.
 */
int destroy_post_office() {
	// Initialize number of used mail box slots in post office to invalid
	po.num_mailboxes = INVALID;
	// Initialize number of message slots in each mailbox of the post office to invalid
	for (int i = 0; i < MAX_MAILBOXES; i++) {
		po.mbox[i].num_messages = INVALID;
		po.mbox_availability[i] = INVALID; // set availability of mailbox to invalid
		po.mbox[i].mailbox_type = INVALID; // Set the mailbox type to invalid
		po.mbox[i].owner_uid = INVALID; // Set owner ID to invalid

		// Reset ACL for sender
		for (int j = 0; j < MAX_MAILBOX_SENDERS; j++) {
			po.mbox[i].senders_acl[j] = EMPTY;
		}
		po.mbox[i].senders_count = START;

		// Reset ACL for receivers
		for (int j = 0; j < MAX_MAILBOX_RECEIVERS; j++) {
			po.mbox[i].receivers_acl[j] = EMPTY;
		}
		po.mbox[i].receivers_count = START;
	}

	printf("Post office destroyed\n");
	return OK;
}

// Implementation of internal functions used by post office
int message_index_minorder(int mboxd, int dest) {
	int order_number;
	int min_order_number = 100;
	int message_index = INVALID;
	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		order_number = po.mbox[mboxd].msg[i].order_number;
		if(po.mbox[mboxd].message_empty[i] == FALSE) {
			for(int j = 0; j<MAX_MAIL_DESTS; j++) {
				if(po.mbox[mboxd].msg[i].dest[j] == dest) {
					if(min_order_number > min(min_order_number, order_number)) {
						message_index = i;
						min_order_number = min(min_order_number, order_number);
					}
				}
			}
		}
	}
	return message_index;
}

int min(int num1, int num2) {
	int min;
	if(num1 >= num2)
		min = num2;
	else
		min = num1;
	return min;
}

int garbage_collector(int mboxd) {
	int all_empty = TRUE;
	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		if(po.mbox[mboxd].message_empty[i] == FALSE) {
			for(int j = 0; j<MAX_MAIL_DESTS; j++) {
				if (po.mbox[mboxd].msg[i].dest[j] != EMPTY) {
					all_empty = 0;
				}
			}
			if(all_empty == TRUE) {
				po.mbox[mboxd].message_empty[i] = TRUE;
				po.mbox[mboxd].num_messages -= 1;
				reordering(po.mbox[mboxd].msg[i].order_number, mboxd);
			}
		}
	}
	return OK;
}

int reordering(int order_number, int mboxd) {
	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		if(po.mbox[mboxd].message_empty[i] == FALSE) {
			if(po.mbox[mboxd].msg[i].order_number > order_number) {
				po.mbox[mboxd].msg[i].order_number -= 1;
			}
		}
	}
	return OK;
}

/**
 * Checks if the specified UID is a valid owner
 */
int is_legal_owner(uid_t caller_uid) {
	// Super user is an owner of all mailboxes
	if(caller_uid == SUPERUSER_UID) {
		return TRUE;
	}

	int flag = FALSE; // Flags if a user with the corresponding uid is found in the owners list
	for (int i = 0; i < MAX_OWNERS; i++) {
		if (po.owners_acl[i] == caller_uid) {
			flag = TRUE;
		}
	}

	return flag;
}

int is_legal_mailbox_owner(int mboxd, uid_t caller_uid) {
	// Super user is an owner of all mailboxes
	return (caller_uid == SUPERUSER_UID || po.mbox[mboxd].owner_uid == caller_uid);
}

int is_legal_mailbox_sender(int mboxd, uid_t caller_uid) {
	if(caller_uid == SUPERUSER_UID) {
		return TRUE;
	}

	if(po.mbox[mboxd].mailbox_type == MAILBOX_TYPE_PUBLIC) {
		// Blacklist for public mailbox
		int flag = TRUE;
		for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
			if(po.mbox[mboxd].senders_acl[i] == caller_uid) {
				flag = FALSE;
				break;
			}
		}
		return flag;
	} else {
		// Whitelist for secure mailbox
		int flag = FALSE;
		for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
			if(po.mbox[mboxd].senders_acl[i] == caller_uid) {
				flag = TRUE;
				break;
			}
		}
		return flag;
	}
}

int is_legal_mailbox_receiver(int mboxd, uid_t caller_uid) {
	if(caller_uid == SUPERUSER_UID) {
		return TRUE;
	}

	if(po.mbox[mboxd].mailbox_type == MAILBOX_TYPE_PUBLIC) {
		// Blacklist for public mailbox
		int flag = TRUE;
		for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
			if(po.mbox[mboxd].receivers_acl[i] == caller_uid) {
				flag = FALSE;
				break;
			}
		}
		return flag;
	} else {
		// Whitelist for secure mailbox
		int flag = FALSE;
		for(int i = 0; i < MAX_MAILBOX_SENDERS; i++) {
			if(po.mbox[mboxd].receivers_acl[i] == caller_uid) {
				flag = TRUE;
				break;
			}
		}
		return flag;
	}
}

/*
 * post_office.h - Declares various functions for mailbox.
 *
 *  Created on: Oct 13, 2015
 *      Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 */

#ifndef CS551_PROJECT3_STANDALONE_POST_OFFICE_H_
#define CS551_PROJECT3_STANDALONE_POST_OFFICE_H_

// #include <minix/type.h>
#include <sys/types.h>

#define MAX_MAILBOXES 16
#define MAX_MAILBOX_MESSAGES 16
#define MAX_MAIL_DESTS 4
#define MAX_MAIL_PAYLOAD_SIZE 32

// Status of mailbox and message slots
#define OK 0
#define FALSE 0
#define TRUE 1
#define INVALID -1
#define START 0
#define EMPTY -1

// Max owners, mailbox senders and receivers
#define MAX_OWNERS 32
#define MAX_MAILBOX_SENDERS 16
#define MAX_MAILBOX_RECEIVERS 16

// Mailbox types - public and secure
#define MAILBOX_TYPE_PUBLIC 0
#define MAILBOX_TYPE_SECURE 1

#define SUPERUSER_UID 0

#define endpoint_t int
#define _SIGN -1*
/*
 * Structure for message stored in mailbox
 */
typedef struct mail_message {
	int order_number;
	endpoint_t src; // Source process id
	endpoint_t dest[MAX_MAIL_DESTS]; // Destination process ids
	char msg[32]; // Message data defined in ipc.h. This is the message sent by user process.
} mail_message;

/*
 * Structure for mail-box
 */
typedef struct mailbox {
	int mailbox_type; // Mailbox type
	uid_t owner_uid; // Owner uid
	int senders_count;
	uid_t senders_acl[MAX_MAILBOX_SENDERS]; // Sender users ACL
	int receivers_count;
	uid_t receivers_acl[MAX_MAILBOX_RECEIVERS]; // Receiver users ACL
	int message_empty[MAX_MAILBOX_MESSAGES];
	int num_messages; // Number of messages queued in mailbox currently
	mail_message msg[MAX_MAILBOX_MESSAGES]; // Messages
	void *data; // Extra data for mailbox
} mailbox;

/*
 * Structure for post-office
 */
typedef struct post_office {
	uid_t owners_acl[MAX_OWNERS]; // Owner users ACL
	int num_mailboxes; // Number of mailboxes in post office currently
	mailbox mbox[MAX_MAILBOXES]; // Mailboxes
	int mbox_availability[MAX_MAILBOXES]; // Array used to indicate whether mbox at corresponding index in the mbox array is free/in use
	void *data; // Extra data for post office
} post_office;

/**
 * Initializes post office. This can be invoked from kmain() in /usr/src/minix/kernel/main.c
 */
int init_post_office();

/**
 * Adds a user as owner in post office.
 */
int add_owner(uid_t caller_uid, uid_t owner_uid);

/**
 * Remove a user as owner in post office.
 */
int remove_owner(uid_t caller_uid, uid_t owner_uid);

/**
 * Creates a new mailbox and stores a corresponding mailbox descriptor in mboxd.
 */
int create_mailbox(uid_t caller_uid, int mailbox_type, int *mboxd);

/**
 * Adds a user as sender of given mailbox.
 */
int add_mailbox_sender(uid_t caller_uid, int mboxd, uid_t sender_uid);

/**
 * Removes a user as sender of given mailbox.
 */
int remove_mailbox_sender(uid_t caller_uid, int mboxd, uid_t sender_uid);

/**
 * Adds a user as receiver of given mailbox.
 */
int add_mailbox_receiver(uid_t caller_uid, int mboxd, uid_t receiver_uid);

/**
 * Removes a user as receiver of given mailbox.
 */
int remove_mailbox_receiver(uid_t caller_uid, int mboxd, uid_t receiver_uid);

/**
 * Deposits a message msg into mailbox having given descriptor mboxd, which is sent by process
 * src_pid to processes dest_pid.
 */
int deposit_mail(uid_t caller_uid, int mboxd, endpoint_t src, endpoint_t dest[], char *msg);

/**
 * Retrieves a message msg from mailbox having given descriptor mboxd, which is sent to the given
 * process dest_pid.
 */
int retrieve_mail(uid_t caller_uid, int mboxd, endpoint_t dest, char *msg);

/**
 * Removes the mailbox with given descriptor.
 */
int remove_mailbox(uid_t caller_uid, int mboxd);

/**
 * Destroy post office. Need to find where to invoke this from.
 */
int destroy_post_office();

#endif /* CS551_PROJECT3_STANDALONE_POST_OFFICE_H_ */

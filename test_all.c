/*
 * test_all.c - Test all system calls
 *
 *  Created on: Nov 20, 2015
 *	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
 */
//#include <minix/syslib.h>
#include <stdio.h>
//#include <sys/errno.h>
#include "errno.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "post_office.h"

#define NUM_WRITERS 2
#define MAX_MAIL_DESTS 4
#define NUM_ITER 20

// Test functions
void testOwner();
void testMailbox();
void testReader();
void testWriter();
void testReaderWriter();

// Util method
void assertStatusCode();
void assertMessage(char * msg, char *expected, char *actual);

int main() {
	init_post_office();
	testOwner();
	testMailbox();
	testReader();
	testWriter();
	testReaderWriter();
	destroy_post_office();
}

void testOwner() {
	// Add owner - super user
	uid_t caller_uid = SUPERUSER_UID;
	int result;
	for(int i = 0; i < MAX_OWNERS; i++) {
		result = add_owner(caller_uid, i + 1);
		assertStatusCode("Add owner", OK, result);
	}

	// Add max owner - super user
	result = add_owner(caller_uid, MAX_OWNERS + 1);
	assertStatusCode("Add owner", POST_OFFICE_MAX_OWNERS, result);

	// Remove invalid owner - super user
	result = remove_owner(caller_uid, MAX_OWNERS + 1);
	assertStatusCode("Remove owner", POST_OFFICE_OWNER_NOT_FOUND, result);

	// Remove valid owner - super user
	for(int i = 0; i < MAX_OWNERS; i++) {
		result = remove_owner(caller_uid, i + 1);
		assertStatusCode("Remove owner", OK, result);
	}

	// Add owner - unauthorized caller
	caller_uid = 1;
	result = add_owner(caller_uid, 1);
	assertStatusCode("Add owner", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove owner - unauthorized caller
	result = remove_owner(caller_uid, 1);
	assertStatusCode("Remove owner", POST_OFFICE_USER_NOT_AUTHORIZED, result);
}

void testMailbox() {
	int mboxd[20];
	int result;

	// Add owners
	result = add_owner(SUPERUSER_UID, 1);
	assertStatusCode("Add owner", OK, result);

	result = add_owner(SUPERUSER_UID, 2);
	assertStatusCode("Add owner", OK, result);

	// Create mailbox - valid owner
	for(int i = 0; i < 16; i++) {
		int mailbox_type = (i % 2 == 0) ? MAILBOX_TYPE_PUBLIC : MAILBOX_TYPE_SECURE;
		result = create_mailbox(1, mailbox_type, &mboxd[i]);
		assertStatusCode("Create mailbox", OK, result);
	}

	// Create mailbox - post office is full - valid owner
	result = create_mailbox(1, MAILBOX_TYPE_PUBLIC, &mboxd[17]);
	assertStatusCode("Create mailbox", POST_OFFICE_FULL, result);

	// Remove invalid mailbox - valid owner
	result = remove_mailbox(1, 17);
	assertStatusCode("Remove mailbox", MAILBOX_NOT_FOUND, result);

	// Remove mailbox - invalid mailbox owner
	result = remove_mailbox(2, mboxd[0]);
	assertStatusCode("Remove mailbox", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove valid mailboxes - valid owner
	for(int i = 15; i >= 0; i--) {
		result = remove_mailbox(1, mboxd[i]);
		assertStatusCode("Remove mailbox", OK, result);
	}

	// Create mailbox - invalid owner
	result = create_mailbox(3, MAILBOX_TYPE_PUBLIC, &mboxd[0]);
	assertStatusCode("Create mailbox", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Create mailbox - super user
	result = create_mailbox(SUPERUSER_UID, MAILBOX_TYPE_PUBLIC, &mboxd[0]);
	assertStatusCode("Create mailbox", OK, result);

	// Remove mailbox - invalid owner
	result = remove_mailbox(3, mboxd[0]);
	assertStatusCode("Remove mailbox", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox - invalid mailbox owner
	result = remove_mailbox(1, mboxd[0]);
	assertStatusCode("Remove mailbox", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox - super user
	result = remove_mailbox(SUPERUSER_UID, mboxd[0]);
	assertStatusCode("Remove mailbox", OK, result);

	// Remove owner
	result = remove_owner(SUPERUSER_UID, 1);
	assertStatusCode("Remove owner", OK, result);
}

void testReader() {
	int mboxd;
	int result;
	char msg[32];

	// Add owner
	result = add_owner(SUPERUSER_UID, 1);
	assertStatusCode("Add owner", OK, result);

	// ****************************
	// Create secure mailbox
	result = create_mailbox(1, MAILBOX_TYPE_SECURE, &mboxd);
	assertStatusCode("Create secure mailbox", OK, result);

	// Read from empty mailbox - super user
	result = retrieve_mail(SUPERUSER_UID, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Read from empty mailbox - mailbox owner
	result = retrieve_mail(1, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Read from empty mailbox - invalid mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Add mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Add mailbox receiver", OK, result);

	// Add duplicate mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Add mailbox receiver", MAILBOX_RECEIVER_EXISTS, result);

	// Read from empty mailbox - valid mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Remove mailbox receiver - invalid mailbox owner
	result = remove_mailbox_receiver(3, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox receiver - mailbox owner
	result = remove_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Read from empty mailbox - invalid mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Add mailbox receiver - super user
	result = add_mailbox_receiver(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Add mailbox receiver", OK, result);

	// Read from empty mailbox - invalid mailbox reader
	result = retrieve_mail(3, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Read from empty mailbox - valid mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Remove mailbox receiver - invalid mailbox owner
	result = remove_mailbox_receiver(3, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox receiver - super user
	result = remove_mailbox_receiver(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Read from invalid mailbox - valid mailbox owner
	result = retrieve_mail(1, 1, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_NOT_FOUND, result);

	// Remove mailbox - valid mailbox owner
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove secure mailbox", OK, result);

	// ****************************
	// Create public mailbox
	result = create_mailbox(1, MAILBOX_TYPE_PUBLIC, &mboxd);
	assertStatusCode("Create public mailbox", OK, result);

	// Read from empty mailbox - super user
	result = retrieve_mail(SUPERUSER_UID, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Read from empty mailbox - mailbox owner
	result = retrieve_mail(1, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Read from empty mailbox - public mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Add (deny) mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Add (deny) mailbox receiver", OK, result);

	// Add (deny) duplicate mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Add (deny) mailbox receiver", MAILBOX_RECEIVER_EXISTS, result);

	// Read from empty mailbox - denied mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox receiver - invalid mailbox owner
	result = remove_mailbox_receiver(3, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove (allow) mailbox receiver - mailbox owner
	result = remove_mailbox_receiver(1, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Read from empty mailbox - public mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Add mailbox receiver - super user
	result = add_mailbox_receiver(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Add mailbox receiver", OK, result);

	// Read from empty mailbox - public mailbox reader
	result = retrieve_mail(3, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_EMPTY, result);

	// Read from empty mailbox - invalid mailbox reader
	result = retrieve_mail(2, mboxd, 10, msg);
	assertStatusCode("Retrieve mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox receiver - invalid mailbox owner
	result = remove_mailbox_receiver(3, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox receiver - super user
	result = remove_mailbox_receiver(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Read from invalid mailbox - valid mailbox owner
	result = retrieve_mail(1, 1, 10, msg);
	assertStatusCode("Retrieve mail", MAILBOX_NOT_FOUND, result);

	// Remove mailbox - valid mailbox owner
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove public mailbox", OK, result);

	// Remove owner
	result = remove_owner(SUPERUSER_UID, 1);
	assertStatusCode("Remove owner", OK, result);
}

void testWriter() {
	int mboxd;
	int result;
	char msg[32] = "test";
	int dest[] = {11, -1, -1, -1};

	// Add owner
	result = add_owner(SUPERUSER_UID, 1);
	assertStatusCode("Add owner", OK, result);

	// ****************************
	// Create secure mailbox
	result = create_mailbox(1, MAILBOX_TYPE_SECURE, &mboxd);
	assertStatusCode("Create secure mailbox", OK, result);

	// Write to mailbox - super user
	result = deposit_mail(SUPERUSER_UID, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Write to mailbox - mailbox owner
	result = deposit_mail(1, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Write to mailbox - invalid mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Add mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add mailbox sender", OK, result);

	// Add duplicate mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add mailbox sender", MAILBOX_SENDER_EXISTS, result);

	// Write to mailbox - valid mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Remove mailbox sender - invalid mailbox owner
	result = remove_mailbox_sender(3, mboxd, 2);
	assertStatusCode("Remove mailbox sender", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox sender - mailbox owner
	result = remove_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Write to mailbox - invalid mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Add mailbox sender - super user
	result = add_mailbox_sender(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Add mailbox receiver", OK, result);

	// Write to mailbox - invalid mailbox sender
	result = deposit_mail(3, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Write to mailbox - valid mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Remove mailbox sender - invalid mailbox owner
	result = remove_mailbox_sender(3, mboxd, 2);
	assertStatusCode("Remove mailbox sender", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox sender - super user
	result = remove_mailbox_sender(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Write to invalid mailbox - valid mailbox owner
	result = deposit_mail(1, 1, 10, dest, msg);
	assertStatusCode("Deposit mail", MAILBOX_NOT_FOUND, result);

	// Remove mailbox - valid mailbox owner
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove secure mailbox", OK, result);

	// ****************************
	// Create public mailbox
	result = create_mailbox(1, MAILBOX_TYPE_PUBLIC, &mboxd);
	assertStatusCode("Create public mailbox", OK, result);

	// Read from empty mailbox - super user
	result = deposit_mail(SUPERUSER_UID, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Read from empty mailbox - mailbox owner
	result = deposit_mail(1, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Read from empty mailbox - public mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Add (deny) mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add (deny) mailbox sender", OK, result);

	// Add (deny) duplicate mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add (deny) mailbox sender", MAILBOX_SENDER_EXISTS, result);

	// Read from empty mailbox - denied mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox sender - invalid mailbox owner
	result = remove_mailbox_sender(3, mboxd, 2);
	assertStatusCode("Remove mailbox sender", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove (allow) mailbox sender - mailbox owner
	result = remove_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Write to mailbox - public mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Add mailbox receiver - super user
	result = add_mailbox_sender(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Add mailbox sender", OK, result);

	// Write to mailbox - public mailbox sender
	result = deposit_mail(3, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", OK, result);

	// Write to mailbox - invalid mailbox sender
	result = deposit_mail(2, mboxd, 10, dest, msg);
	assertStatusCode("Deposit mail", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox sender - invalid mailbox owner
	result = remove_mailbox_sender(3, mboxd, 2);
	assertStatusCode("Remove mailbox sender", POST_OFFICE_USER_NOT_AUTHORIZED, result);

	// Remove mailbox sender - super user
	result = remove_mailbox_sender(SUPERUSER_UID, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Read from invalid mailbox - valid mailbox owner
	result = deposit_mail(1, 1, 10, dest, msg);
	assertStatusCode("Deposit mail", MAILBOX_NOT_FOUND, result);

	// Remove mailbox - valid mailbox owner
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove public mailbox", OK, result);

	// Remove owner
	result = remove_owner(SUPERUSER_UID, 1);
	assertStatusCode("Remove owner", OK, result);
}

void testReaderWriter() {
	int mboxd;
	int result;
	char msg[32] = "test";
	char msg1[32];
	char msg2[32];
	int dest[] = {11, 12, -1, -1};

	// Add owner
	result = add_owner(SUPERUSER_UID, 1);
	assertStatusCode("Add owner", OK, result);

	// ****************************
	// Create secure mailbox
	result = create_mailbox(1, MAILBOX_TYPE_SECURE, &mboxd);
	assertStatusCode("Create secure mailbox", OK, result);

	// Add mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add mailbox sender", OK, result);

	// Add mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 3);
	assertStatusCode("Add mailbox receiver", OK, result);

	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		// Write to mailbox - valid mailbox sender
		sprintf(msg, "%d", i);
		result = deposit_mail(2, mboxd, 10, dest, msg);
		assertStatusCode("Deposit mail", OK, result);
	}

	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		sprintf(msg2, "%d", i);
		// Read from mailbox - valid mailbox receiver
		result = retrieve_mail(3, mboxd, 11, msg1);
		assertStatusCode("Retrieve mail", OK, result);
		assertMessage("Received message", msg2, msg1);

		// Read from mailbox - valid mailbox receiver
		result = retrieve_mail(3, mboxd, 12, msg1);
		assertStatusCode("Retrieve mail", OK, result);
		assertMessage("Received message", msg2, msg1);
	}

	// Remove mailbox sender - mailbox owner
	result = remove_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Remove mailbox sender - mailbox owner
	result = remove_mailbox_receiver(1, mboxd, 3);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Remove mailbox
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove mailbox", OK, result);

	// ****************************
	// Create public mailbox
	result = create_mailbox(1, MAILBOX_TYPE_PUBLIC, &mboxd);
	assertStatusCode("Create secure mailbox", OK, result);

	// Add (deny) mailbox sender - mailbox owner
	result = add_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Add mailbox sender", OK, result);

	// Add (deny) mailbox receiver - mailbox owner
	result = add_mailbox_receiver(1, mboxd, 3);
	assertStatusCode("Add mailbox receiver", OK, result);

	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		// Write to mailbox - valid mailbox sender
		sprintf(msg, "%d", i);
		result = deposit_mail(4, mboxd, 10, dest, msg);
		assertStatusCode("Deposit mail", OK, result);
	}

	for(int i = 0; i < MAX_MAILBOX_MESSAGES; i++) {
		sprintf(msg2, "%d", i);
		// Read from mailbox - valid mailbox receiver
		result = retrieve_mail(5, mboxd, 11, msg1);
		assertStatusCode("Retrieve mail", OK, result);
		assertMessage("Received message", msg2, msg1);

		// Read from mailbox - valid mailbox receiver
		result = retrieve_mail(5, mboxd, 12, msg1);
		assertStatusCode("Retrieve mail", OK, result);
		assertMessage("Received message", msg2, msg1);
	}

	// Remove mailbox sender - mailbox owner
	result = remove_mailbox_sender(1, mboxd, 2);
	assertStatusCode("Remove mailbox sender", OK, result);

	// Remove mailbox sender - mailbox owner
	result = remove_mailbox_receiver(1, mboxd, 3);
	assertStatusCode("Remove mailbox receiver", OK, result);

	// Remove mailbox
	result = remove_mailbox(1, mboxd);
	assertStatusCode("Remove mailbox", OK, result);

	// Remove owner
	result = remove_owner(SUPERUSER_UID, 1);
	assertStatusCode("Remove owner", OK, result);
}

void assertStatusCode(char *msg, int expected, int actual) {
	if(expected == actual) {
		printf("%s: expected (%d), actual (%d). [%s]\n", msg, expected, actual, "Success");
	} else {
		printf("%s: expected (%d), actual (%d). [%s]\n", msg, expected, actual, "Fail");
		exit(0);
	}
}

void assertMessage(char * msg, char *expected, char *actual) {
	if(strcmp(expected, actual) == 0) {
		printf("%s: expected (%s), actual (%s). [%s]\n", msg, expected, actual, "Success");
	} else {
		printf("%s: expected (%s), actual (%s). [%s]\n", msg, expected, actual, "Fail");
		exit(0);
	}
}

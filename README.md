
o Test program description and execution:
=========================================
- Present in this bundle in one main test program (test_all.c) which helps us execute several tests for the system calls that we have implemented. Following is a brief description of each module present in test_all.c
		a. function testOwner()
			Tests the addition and removal of an owner to the post office. An owner is a user who can create or remove a mailbox. An owner can also add authorized senders who can deposit mail to the mailbox and authorized receivers 
			who can read mail from the mailbox if messages are sent to them. An owner can remove the privileges of a sender/receiver whenever required. An owner can only be added by the super user (ID: 0). 
			This function tests for normal working of the sys_add_owner() and sys_remove_owner() system calls and the following exception scenarios:
			
			* Adding more than 32 owners (MAX_OWNERS) to the post office: POST_OFFICE_MAX_OWNERS error code returned to user process
			* Removing an owner whose ID does not exist in the access control list of owners to the post office: POST_OFFICE_OWNER_NOT_FOUND error code is returned to the user process
			* Removal of an owner by a user who is not a super user: POST_OFFICE_USER_NOT_AUTHORIZED error code returned to the user process
			* Addition of an owner by a user who is not a super user: POST_OFFICE_USER_NOT_AUTHORIZED error code returned to the user process
			
		b. function testMailbox()
			Tests the creation of mailbox through the sys_create_mailbox() system call and the removal of mailbox through the sys_remove_mailbox() system calls. We have implemented the notion of a "post office"
			where a post office can hold a maximum of 16 mailboxes. In other words, only 16 mailboxes can operate at any given time. A mailbox can be of two type: Public or Secure.  A “secure mailbox” can be used
			only by designated users, and each user is given the capability to send to or receive from the mailbox or both. A “public mailbox” can be used by any user, except those who are specifically denied send
			or receive access. Only a legal owner added through the sys_add_owner() system call or the super user is able to create/remove a mailbox. 
			This function tests for normal working of the sys_create_mailbox() and sys_remove_mailbox() system calls and the following exception scenarios:
			
			* Creating more than 16 (defined by MAX_MAILBOXES) mailboxes in the post office: POST_OFFICE_FULL error code returned to user process
			* Deleting a mailbox whose ID is not known: MAILBOX_NOT_FOUND error code returned to user process
			* Removing a mailbox by a user who is not a legal owner in the post office access control list: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			* Creating a mailbox by a user who is not a legal owner in the post office access control list: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			
		c. function testReader()
			Tests the retrieve mail IPC system call implemented as sys_retrieve_mail() system call which is used to retrieve mail from a mail box. 
			This function also tests for sys_add_mailbox_receiver() and sys_remove_mailbox_receiver which adds a legal receiver to a mailbox specified by an mbox ID or removes a user as a legal 
			receiver to a mailbox respectively. A legal receiver can be added to a mailbox only by the owner of that mailbox or the super user.
			Normal working of these three system calls along with the following exception scenarios are tested:
			
			* Retrieving mail from a mailbox whose ID does not exist: MAILBOX_NOT_FOUND error code returned to user process
			* Retrieving mail from a mailbox that has no messages: MAILBOX_EMPTY error code returned
			* Retrieving mail from a mailbox that is not empty but has no message present for the concerned reader/user process: MAILBOX_NO_MESSAGE error code returned
			* Adding more than 16 (defined by MAX_MAILBOX_RECEIVERS) legal senders to a mailbox: MAILBOX_RECEIVERS_ACL_FULL error code is returned
			* Adding a receiver by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code returned
			* Removing a receiver who does not exist in the access control list of receivers of the mailbox: MAILBOX_RECEIVER_NOT_FOUND error code is returned
			* Adding a receiver who already exists in the access control list of receivers of the mailbox: MAILBOX_RECEIVER_EXISTS error code is returned to the user process
			* Adding a legal receiver by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			* Removing a legal receiver by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			
		d. function testWriter()
			Tests the deposit mail IPC system call implemented as sys_deposit_mail() system call which is used to send mail to a particular mail box. 
			This function also tests for sys_add_mailbox_sender() and sys_remove_mailbox_sender which adds a legal sender to a mailbox specified by an mbox ID or removes a user as a legal 
			sender to a mailbox respectively. A legal sender can be added to a mailbox only by the owner of that mailbox or the super user.
			Normal working of these three system calls along with the following exception scenarios are tested:
			
			* Sending mail to a mailbox whose ID does not exist: MAILBOX_NOT_FOUND error code returned to user process
			* Sending mail to a mailbox that has reached its maximum capacity of 16 messages(defined by MAX_MAILBOX_MESSAGES): MAILBOX_FULL error code returned
			* Adding more than 16 (defined by MAX_MAILBOX_SENDERS) legal senders to a mailbox: MAILBOX_SENDERS_ACL_FULL error code is returned
			* Adding a sender by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code returned
			* Removing a sender who does not exist in the access control list of receivers of the mailbox: MAILBOX_SENDER_NOT_FOUND error code is returned
			* Adding a sender who already exists in the access control list of receivers of the mailbox: MAILBOX_SENDER_EXISTS error code is returned to the user process
			* Adding a legal sender by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			* Removing a legal sender by a user who is not the owner of the mailbox or the super user: POST_OFFICE_USER_NOT_AUTHORIZED error code is returned to the user process
			
		e. function testReaderWriter()
			Tests both the sender and receiver codes. Messages sent to a particular receiver from the same sender are received in the same order
			as they are delivered.
			
		f. function assertStatusCode()
			This is a helper function that tests whether the expected value of status code matches the actual status code returned from the particular function. It is used in all of the above test modules to check
			SUCCESS/FAIL status of the various IPC system call implementations
			
		g. function assertMessage()
			This is a helper function that tests whether the expected value of a message delivered by the sender matches the actual message retrieved by the receiver for which the message was intended.
			It is used in testReaderWriter() module to ascertain the SUCCESS/FAIL status of the IPC system calls - sys_retrieve_mail() and sys_deposit_mail()
			
- Executing test programs:
##  change to directory containing test cases ##
% cd /root/cs551_project3/
##  build test case ##
% make
##  execute tests ##
./test_all.sh			




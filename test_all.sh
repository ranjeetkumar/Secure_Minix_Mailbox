##########################################################
# test_all.sh - Test all system calls
#
#  Created on: Nov 23, 2015
#	  Author: Aman Mishra, Ranjeet Kumar, Shruti Kasetty
##########################################################

MAX_OWNERS=32
MAX_MAILBOXES=16
OWNER_UID_BASE=1001

MAILBOX_TYPE_PUBLIC=0
MAILBOX_TYPE_SECURE=1

OK=0

POST_OFFICE_FULL=97
MAILBOX_NOT_FOUND=98
MAILBOX_FULL=99
MAILBOX_NO_MESSAGE=100
MAILBOX_EMPTY=101

POST_OFFICE_OWNER_EXISTS=102
POST_OFFICE_MAX_OWNERS=103
POST_OFFICE_OWNER_NOT_FOUND=104
POST_OFFICE_USER_NOT_AUTHORIZED=105
MAILBOX_SENDER_EXISTS=106
MAILBOX_RECEIVER_EXISTS=107
MAILBOX_SENDERS_ACL_FULL=108
MAILBOX_RECEIVERS_ACL_FULL=109
MAILBOX_SENDER_NOT_FOUND=110
MAILBOX_RECEIVER_NOT_FOUND=111

# Test functions
testOwner() {
	echo "testOwner"

	# Add owner - super user
	i=0
	while [ $i -lt $MAX_OWNERS ]
	do
		owner_uid=`echo $(( $OWNER_UID_BASE + i ))`
		./add_owner_test $owner_uid
		assertEquals "Add owner" $OK $?
		i=`echo $(( $i + 1 ))`
	done

	# Add max owner - super user
	owner_uid=`echo $(( $OWNER_UID_BASE + $MAX_OWNERS + 1))`
	./add_owner_test $owner_uid
	assertEquals "Add owner" $POST_OFFICE_MAX_OWNERS $?

	# Remove invalid owner - super user
	./remove_owner_test $owner_uid
	assertEquals "Remove owner" $POST_OFFICE_OWNER_NOT_FOUND $?

	# Remove valid owner - super user
	i=0
	while [ $i -lt $MAX_OWNERS ]
	do
		owner_uid=`echo $(( $OWNER_UID_BASE + i ))`
		./remove_owner_test $owner_uid
		assertEquals "Remove owner" $OK $?
		i=`echo $(( $i + 1 ))`
	done

	# Add owner - unauthorized caller
	su user1 -c "./add_owner_test 1002"
	assertEquals "Add owner" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove owner - unauthorized caller
	su user1 -c "./remove_owner_test 1002"
	assertEquals "Remove owner" $POST_OFFICE_USER_NOT_AUTHORIZED $?
}

testMailbox() {
	echo "testMailbox"

	# Add owners
	./add_owner_test 1001
	assertEquals "Add owner" $OK $?

	./add_owner_test 1002
	assertEquals "Add owner" $OK $?

	# Create mailbox - valid owner
	i=0
	while [ $i -lt $MAX_MAILBOXES ]
	do
		mailbox_type=`echo $(( $i % 2))`
		mboxd=`su user1 -c "./create_mailbox_test $mailbox_type"`
		assertEquals "Create mailbox" $OK $?
		assertEquals "Mailbox id" $i $mboxd
		i=`echo $(( $i + 1 ))`
	done

	# Create mailbox - post office is full - valid owner
	su user1 -c "./create_mailbox_test $mailbox_type"
	assertEquals "Create mailbox" $POST_OFFICE_FULL $?

	# Remove invalid mailbox - valid owner
	su user1 -c "./remove_mailbox_test 17"
	assertEquals "Remove mailbox" $MAILBOX_NOT_FOUND $?

	# Remove mailbox - invalid mailbox owner
	su user2 -c "./remove_mailbox_test 0"
	assertEquals "Remove mailbox" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove valid mailboxes - valid owner
	i=`echo $(( $MAX_MAILBOXES - 1 ))`
	while [ $i -ge 0 ]
	do
		su user1 -c "./remove_mailbox_test $i"
		assertEquals "Remove mailbox" $OK $?
		i=`echo $(( $i - 1 ))`
	done

	# Create mailbox - invalid owner
	su user3 -c "./create_mailbox_test $MAILBOX_TYPE_PUBLIC"
	assertEquals "Create mailbox" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Create mailbox - super user
	mboxd=`./create_mailbox_test $MAILBOX_TYPE_PUBLIC`
	assertEquals "Create mailbox" $OK $?
	assertEquals "Mailbox id" 0 $mboxd

	# Remove mailbox - invalid owner
	su user3 -c "./remove_mailbox_test 0"
	assertEquals "Remove mailbox" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox - invalid mailbox owner
	su user1 -c "./remove_mailbox_test 0"
	assertEquals "Remove mailbox" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox - super user
	./remove_mailbox_test 0
	assertEquals "Remove mailbox" $OK $?

	# Remove owners
	./remove_owner_test 1001
	assertEquals "Remove owner" $OK $?

	./remove_owner_test 1002
	assertEquals "Remove owner" $OK $?
}

testReader() {
	echo "testReader"

	# Add owner
	./add_owner_test 1001
	assertEquals "Add owner" $OK $?

	# ****************************
	# Create secure mailbox
	mboxd=`su user1 -c "./create_mailbox_test $MAILBOX_TYPE_SECURE"`
	assertEquals "Create secure mailbox" $OK $?
	assertEquals "Mailbox id" 0 $?

	# Read from empty mailbox - super user
	./retrieve_mail_test $mboxd
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Read from empty mailbox - mailbox owner
	su user1 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Read from empty mailbox - invalid mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Add mailbox receiver - mailbox owner
	su user1 -c "./add_mailbox_receiver_test $mboxd 1002"
	assertEquals "Add mailbox receiver" $OK $?

	# Add duplicate mailbox receiver - mailbox owner
	su user1 -c "./add_mailbox_receiver_test $mboxd 1002"
	assertEquals "Add mailbox receiver" $MAILBOX_RECEIVER_EXISTS $?

	# Read from empty mailbox - valid mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Remove mailbox receiver - invalid mailbox owner
	su user3 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox receiver - mailbox owner
	su user1 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $OK $?

	# Read from empty mailbox - invalid mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Add mailbox receiver - super user
	./add_mailbox_receiver_test $mboxd 1002
	assertEquals "Add mailbox receiver" $OK $?

	# Read from empty mailbox - invalid mailbox reader
	su user3 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Read from empty mailbox - valid mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Remove mailbox receiver - invalid mailbox owner
	su user3 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox receiver - super user
	./remove_mailbox_receiver_test $mboxd 1002
	assertEquals "Remove mailbox receiver" $OK $?

	# Read from invalid mailbox - valid mailbox owner
	su user1 -c "./retrieve_mail_test 1"
	assertEquals "Retrieve mail" $MAILBOX_NOT_FOUND $?

	# Remove mailbox - valid mailbox owner
	su user1 -c "./remove_mailbox_test $mboxd"
	assertEquals "Remove secure mailbox" $OK $?

	# ****************************
	# Create public mailbox
	mboxd=`su user1 -c "./create_mailbox_test $MAILBOX_TYPE_PUBLIC"`
	assertEquals "Create public mailbox" $OK $?
	assertEquals "Mailbox id" 0 $mboxd

	# Read from empty mailbox - super user
	./retrieve_mail_test $mboxd
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Read from empty mailbox - mailbox owner
	su user1 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Read from empty mailbox - public mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Add (deny) mailbox receiver - mailbox owner
	su user1 -c "./add_mailbox_receiver_test $mboxd 1002"
	assertEquals "Add (deny) mailbox receiver" $OK $?

	# Add (deny) duplicate mailbox receiver - mailbox owner
	su user1 -c "./add_mailbox_receiver_test $mboxd 1002"
	assertEquals "Add (deny) mailbox receiver" $MAILBOX_RECEIVER_EXISTS $?

	# Read from empty mailbox - denied mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox receiver - invalid mailbox owner
	su user3 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove (allow) mailbox receiver - mailbox owner
	su user1 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $OK $?

	# Read from empty mailbox - public mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Add mailbox receiver - super user
	./add_mailbox_receiver_test $mboxd 1002
	assertEquals "Add mailbox receiver" $OK $?

	# Read from empty mailbox - public mailbox reader
	su user3 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $MAILBOX_EMPTY $?

	# Read from empty mailbox - invalid mailbox reader
	su user2 -c "./retrieve_mail_test $mboxd"
	assertEquals "Retrieve mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox receiver - invalid mailbox owner
	su user3 -c "./remove_mailbox_receiver_test $mboxd 1002"
	assertEquals "Remove mailbox receiver" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox receiver - super user
	./remove_mailbox_receiver_test $mboxd 1002
	assertEquals "Remove mailbox receiver" $OK $?

	# Read from invalid mailbox - valid mailbox owner
	su user1 -c "./retrieve_mail_test 1"
	assertEquals "Retrieve mail" $MAILBOX_NOT_FOUND $?

	# Remove mailbox - valid mailbox owner
	su user1 -c "./remove_mailbox_test $mboxd"
	assertEquals "Remove public mailbox" $OK $?

	# Remove owner
	./remove_owner_test 1001
	assertEquals "Remove owner" $OK $?
}

testWriter() {
	echo "testWriter"

	msg="test"
	dest="11 -1 -1 -1"

	# Add owner
	./add_owner_test 1001
	assertEquals "Add owner" $OK $?

	# ****************************
	# Create secure mailbox
	mboxd=`su user1 -c "./create_mailbox_test $MAILBOX_TYPE_SECURE"`
	assertEquals "Create secure mailbox" $OK $?
	assertEquals "Mailbox id" 0 $mboxd

	# Write to mailbox - super user
	./deposit_mail_test $mboxd $dest $msg
	assertEquals "Deposit mail" $OK $?

	# Write to mailbox - mailbox owner
	su user1 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Write to mailbox - invalid mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Add mailbox sender - mailbox owner
	su user1 -c "./add_mailbox_sender_test $mboxd 1002"
	assertEquals "Add mailbox sender" $OK $?

	# Add duplicate mailbox sender - mailbox owner
	su user1 -c "./add_mailbox_sender_test $mboxd 1002"
	assertEquals "Add mailbox sender" $MAILBOX_SENDER_EXISTS $?

	# Write to mailbox - valid mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Remove mailbox sender - invalid mailbox owner
	su user3 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox sender - mailbox owner
	su user1 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $OK $?

	# Write to mailbox - invalid mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Add mailbox sender - super user
	./add_mailbox_sender_test $mboxd 1002
	assertEquals "Add mailbox receiver" $OK $?

	# Write to mailbox - invalid mailbox sender
	su user3 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Write to mailbox - valid mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Remove mailbox sender - invalid mailbox owner
	su user3 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox sender - super user
	./remove_mailbox_sender_test $mboxd 1002
	assertEquals "Remove mailbox sender" $OK $?

	# Write to invalid mailbox - valid mailbox owner
	su user1 -c "./deposit_mail_test 1 $dest $msg"
	assertEquals "Deposit mail" $MAILBOX_NOT_FOUND $?

	# Remove mailbox - valid mailbox owner
	su user1 -c "./remove_mailbox_test $mboxd"
	assertEquals "Remove secure mailbox" $OK $?

	# ****************************
	# Create public mailbox
	mboxd=`su user1 -c "./create_mailbox_test $MAILBOX_TYPE_PUBLIC"`
	assertEquals "Create secure mailbox" $OK $?
	assertEquals "Mailbox id" 0 $mboxd

	# Read from empty mailbox - super user
	./deposit_mail_test $mboxd $dest $msg
	assertEquals "Deposit mail" $OK $?

	# Read from empty mailbox - mailbox owner
	su user1 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Read from empty mailbox - public mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Add (deny) mailbox sender - mailbox owner
	su user1 -c "./add_mailbox_sender_test $mboxd 1002"
	assertEquals "Add (deny) mailbox sender" $OK $?

	# Add (deny) duplicate mailbox sender - mailbox owner
	su user1 -c "./add_mailbox_sender_test $mboxd 1002"
	assertEquals "Add (deny) mailbox sender" $MAILBOX_SENDER_EXISTS $?

	# Read from empty mailbox - denied mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox sender - invalid mailbox owner
	su user3 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove (allow) mailbox sender - mailbox owner
	su user1 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $OK $?

	# Write to mailbox - public mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Add mailbox receiver - super user
	./add_mailbox_sender_test $mboxd 1002
	assertEquals "Add mailbox sender" $OK $?

	# Write to mailbox - public mailbox sender
	su user3 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $OK $?

	# Write to mailbox - invalid mailbox sender
	su user2 -c "./deposit_mail_test $mboxd $dest $msg"
	assertEquals "Deposit mail" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox sender - invalid mailbox owner
	su user3 -c "./remove_mailbox_sender_test $mboxd 1002"
	assertEquals "Remove mailbox sender" $POST_OFFICE_USER_NOT_AUTHORIZED $?

	# Remove mailbox sender - super user
	./remove_mailbox_sender_test $mboxd 1002
	assertEquals "Remove mailbox sender" $OK $?

	# Read from invalid mailbox - valid mailbox owner
	su user1 -c "./deposit_mail_test 1 $dest $msg"
	assertEquals "Deposit mail" $MAILBOX_NOT_FOUND $?

	# Remove mailbox - valid mailbox owner
	su user1 -c "./remove_mailbox_test $mboxd"
	assertEquals "Remove public mailbox" $OK $?

	# Remove owner
	./remove_owner_test 1001
	assertEquals "Remove owner" $OK $?
}

# Util methods
assertEquals() {
	msg1=$1
	expected=$2
	actual=$3

	if [ $expected -eq $actual ]
	then
		echo "$msg1: expected ($expected), actual ($actual). [Success]"
	else
		echo "$msg1: expected ($expected), actual ($actual). [Fail]"
		exit
	fi
}

echo "Starting test"
testOwner
testMailbox
testReader
testWriter
echo "Test complete"

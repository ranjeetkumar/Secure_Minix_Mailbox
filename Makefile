all: test_all add_mailbox_receiver_test add_mailbox_sender_test add_owner_test create_mailbox_test deposit_mail_test remove_mailbox_receiver_test remove_mailbox_sender_test remove_mailbox_test remove_owner_test retrieve_mail_test

clean:
	rm test_all add_mailbox_receiver_test add_mailbox_sender_test add_owner_test create_mailbox_test deposit_mail_test remove_mailbox_receiver_test remove_mailbox_sender_test remove_mailbox_test remove_owner_test retrieve_mail_test *.o

post_office.o: post_office.c post_office.h
	clang -c -g -std=gnu99 post_office.c

test_all.o: test_all.c post_office.h
	clang -c -g -std=gnu99 test_all.c

test_all: post_office.o test_all.o
	clang -o test_all post_office.o test_all.o

add_mailbox_receiver_test.o: add_mailbox_receiver_test.c
	clang -c -std=gnu99 add_mailbox_receiver_test.c

add_mailbox_sender_test.o: add_mailbox_sender_test.c
	clang -c -std=gnu99 add_mailbox_sender_test.c

add_owner_test.o: add_owner_test.c
	clang -c -std=gnu99 add_owner_test.c

create_mailbox_test.o: create_mailbox_test.c
	clang -c -std=gnu99 create_mailbox_test.c

deposit_mail_test.o: deposit_mail_test.c
	clang -c -std=gnu99 deposit_mail_test.c

remove_mailbox_receiver_test.o: remove_mailbox_receiver_test.c
	clang -c -std=gnu99 remove_mailbox_receiver_test.c

remove_mailbox_sender_test.o: remove_mailbox_sender_test.c
	clang -c -std=gnu99 remove_mailbox_sender_test.c

remove_mailbox_test.o: remove_mailbox_test.c
	clang -c -std=gnu99 remove_mailbox_test.c

remove_owner_test.o: remove_owner_test.c
	clang -c -std=gnu99 remove_owner_test.c

retrieve_mail_test.o: retrieve_mail_test.c
	clang -c -std=gnu99 retrieve_mail_test.c

add_mailbox_receiver_test: add_mailbox_receiver_test.o 
	clang -o add_mailbox_receiver_test add_mailbox_receiver_test.o -lsys -ltimers

add_mailbox_sender_test: add_mailbox_sender_test.o 
	clang -o add_mailbox_sender_test add_mailbox_sender_test.o -lsys -ltimers

add_owner_test: add_owner_test.o 
	clang -o add_owner_test add_owner_test.o -lsys -ltimers

create_mailbox_test: create_mailbox_test.o 
	clang -o create_mailbox_test create_mailbox_test.o -lsys -ltimers

deposit_mail_test: deposit_mail_test.o 
	clang -o deposit_mail_test deposit_mail_test.o -lsys -ltimers

remove_mailbox_receiver_test: remove_mailbox_receiver_test.o 
	clang -o remove_mailbox_receiver_test remove_mailbox_receiver_test.o -lsys -ltimers

remove_mailbox_sender_test: remove_mailbox_sender_test.o 
	clang -o remove_mailbox_sender_test remove_mailbox_sender_test.o -lsys -ltimers

remove_mailbox_test: remove_mailbox_test.o 
	clang -o remove_mailbox_test remove_mailbox_test.o -lsys -ltimers

remove_owner_test: remove_owner_test.o 
	clang -o remove_owner_test remove_owner_test.o -lsys -ltimers

retrieve_mail_test: retrieve_mail_test.o 
	clang -o retrieve_mail_test retrieve_mail_test.o -lsys -ltimers
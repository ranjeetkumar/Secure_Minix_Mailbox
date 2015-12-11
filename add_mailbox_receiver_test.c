#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define OK 0

/**
 * add_mailbox_receiver_test <mboxd> <receiver_uid>
 */
int main(int argc, char *argv[]) {
	int mboxd = atoi(argv[1]);
	uid_t receiver_uid = atoi(argv[2]);
	int result = sys_add_mailbox_receiver(mboxd, receiver_uid);
	return result;
}

#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/**
 * create_mailbox_test <mailbox_type>
 */
int main(int argc, char *argv[]) {
	int mailbox_type = atoi(argv[1]);
	int mboxd;
	int result = sys_create_mailbox(mailbox_type, &mboxd);
	printf("%d", mboxd);
	return result;
}

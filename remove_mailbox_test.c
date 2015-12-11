#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/**
 * remove_mailbox_test <mboxd>
 */
int main(int argc, char *argv[]) {
	int mboxd = atoi(argv[1]);
	int result = sys_remove_mailbox(mboxd);
	return result;
}

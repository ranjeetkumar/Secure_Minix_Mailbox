#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_MAIL_DESTS 4

/**
 * deposit_mail_test <mboxd> <pid1> <pid2> <pid3> <pid4> <msg>
 */
int main(int argc, char *argv[]) {
	int mboxd = atoi(argv[1]);
	pid_t dest[MAX_MAIL_DESTS];
	for(int i = 0; i < MAX_MAIL_DESTS; i++) {
		dest[i] = atoi(argv[2 + i]);
	}
	int result = sys_deposit_mail(mboxd, dest, argv[2 + MAX_MAIL_DESTS]);
	return result;
}

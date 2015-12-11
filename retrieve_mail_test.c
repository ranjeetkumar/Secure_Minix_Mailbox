#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_MAIL_PAYLOAD_SIZE 32

/**
 * retrieve_mail_test <mboxd>
 */
int main(int argc, char *argv[]) {
	int mboxd = atoi(argv[1]);
	char msg[MAX_MAIL_PAYLOAD_SIZE];
	int result = sys_retrieve_mail(mboxd, msg);
	printf("%s", msg);
	return result;
}

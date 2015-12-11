#include <minix/syslib.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define OK 0

/**
 * add_owner_test <owner_uid>
 */
int main(int argc, char *argv[]) {
	uid_t owner_uid = atoi(argv[1]);
	int result = sys_add_owner(owner_uid);
	return result;
}

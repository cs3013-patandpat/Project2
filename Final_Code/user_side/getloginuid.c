
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int getloginuid(pid_t target){
	pid_t* ptr_to_pid;
	uid_t* ptr_to_uid;
	long results;
	uid_t process_usr_value;

	ptr_to_pid = (pid_t*)malloc(sizeof(pid_t));
	ptr_to_uid = (pid_t*)malloc(sizeof(uid_t));
	*ptr_to_pid = target;

	results = syscall(357,ptr_to_pid,ptr_to_uid);
	if(results < 0){
		printf("pid # %d could not be found.\n",target);//failure
		return -2; //-1 is legitimate value of some users
	}
	process_usr_value = *ptr_to_uid;
	printf("The loginuid of process %d is %d.\n",target,process_usr_value);//success
	return process_usr_value;
}

int main(int argc, char *argv){
	if(argc !=2){
		printf("Executed command [getloginuid] improperly. Correct usage:\n");
		printf("./getloginuid <pid>\n");
	}
	
	const char *pid = argv[1];
	getloginuid(atoi(pid));
	
	return 0;
}

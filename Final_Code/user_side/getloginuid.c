
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

	syscall(357,ptr_to_pid,ptr_to_uid);
	results = *ptr_to_uid;
	if(results < -1){//root can sometimes be -1 here.
		printf("pid # %d could not be found.\n",target);//failure
		return -1;//error. data not found
	}
	process_usr_value = *ptr_to_uid;
	printf("The loginuid of process %d is %d.\n",target,process_usr_value);//success
	return process_usr_value;
}

int main(int argc, char **argv){
	if(argc !=2){
		printf("Executed command [getloginuid] improperly. Correct usage:\n");
		printf("./getloginuid <pid>\n");
	}
	
	int pid = atoi(argv[1]);
	getloginuid(pid);
	
	return 0;
}

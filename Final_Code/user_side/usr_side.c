//Patrick's of the Polley and Lebold

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int shift2user(pid_t target, uid_t user){
	uid_t current_uid;
	pid_t* ptr_to_pid;
	uid_t* ptr_to_uid;
	long results;

	ptr_to_pid = (pid_t*)malloc(sizeof(pid_t));
	ptr_to_uid = (pid_t*)malloc(sizeof(uid_t));
	*ptr_to_pid = target;
	current_uid = getuid();

	if(current_uid != 0){
		printf("non-root is attempting to change user.\n Changing user of pid to 1001.\n");//if the user is not root, they can't specify what to change the uid to.
		*ptr_to_uid = 1001;
		results = syscall(356,ptr_to_pid,ptr_to_uid);//custom syscall #2 is numbered as 356
	}else{
		printf("root is calling. better do what it says.\n");	
		*ptr_to_uid = user;
		results = syscall(356,ptr_to_pid,ptr_to_uid);
	}
	
	if(results == 0) printf("Success. Process # %d has had its user changed to %d.\n",target,user);
	if(results == -2) printf("Failure. User # %d attempted to change the process uid of a different user.\n",user);
	else printf("process id # %d could not be found. Ending process.\n",target);
	return 0;
}

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
		return -1;
	}
	process_usr_value = *ptr_to_uid;
	printf("The loginuid of process %d is %d.\n",target,process_usr_value);//success
}

int main(void){

	return 0;
}


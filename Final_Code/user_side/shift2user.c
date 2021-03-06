//Patrick Lebold and Patrick Polley 

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
    printf("You do not have specified permissions. Changing uid of requested pid of pid to 1001.\n");//if the user is not root, they can't specify what to change the uid to.
    *ptr_to_uid = 1001;
  }else{
    *ptr_to_uid = user;
  }
  syscall(356,ptr_to_pid,ptr_to_uid);
  results = *ptr_to_pid;
	
  if(results == 0) 
    printf("Success. Process # %d has had its user changed to %d.\n",target,*ptr_to_uid);
  else if(results == -2) 
    printf("Failure. User # %d attempted to change the process uid of a different user.\n",*ptr_to_uid);
  else 
    printf("process id # %d could not be found. Ending process.\n",target);
  return 0;
}

int main(int argc, char **argv){
  if(argc !=3){
    printf("Executed command [shift2user] improperly. Correct usage:\n");
    printf("./shift2user <pid> <uid>");
  }
	
  int pid = atoi(argv[1]);
  int uid = atoi(argv[2]);
	
  shift2user(pid,uid);
	
  return 0;
}


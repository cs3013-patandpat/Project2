#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/current.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, unsigned short *target_uid);
asmlinkage long (*ref_sys_cs3013_syscall3)(unsigned short *target_pid, unsigned short *actual_uid);

asmlinkage long new_sys_cs3013_syscall1(void) {
  printk(KERN_INFO "\"’Hello world?!’ More like ’Goodbye, world!’ EXTERMINATE!\" -- Dalek");
  return 0;
}

//dfs for a root user
asmlinkage long helper_function2(struct task_struct *task, int t_pid, unsigned short t_uid){
	struct list_head* list;
	struct task_struct* ptr_to_task;
	int this_pid;
	int status;

	list_for_each(list,&task->children){
		ptr_to_task = list_entry(list,struct task_struct, sibling);
		this_pid = ptr_to_task -> pid;
		if(this_pid == t_pid){
			//usr_info = (struct cred*) &(ptr_to_task -> cred);//need to cast this
			//(usr_info -> uid).val = t_uid;
			printk("found the bastard");
			ptr_to_task -> loginuid.val = t_uid;
			return 0;
		}
		status = helper_function2(ptr_to_task,t_pid,t_uid);
		if(status == 0) return 0;
	}
	return -1;//something went wrong. couldn't find it
}

//dfs for a non-root user
asmlinkage long helper_function2_alt(struct task_struct *task, int t_pid, unsigned short t_uid, unsigned int callee){
	struct list_head* list;
	struct task_struct* ptr_to_task;
	int this_pid;
	int status;
	printk(KERN_INFO "The process to be found is %d.\n",t_pid);

	list_for_each(list,&task->children){
		ptr_to_task = list_entry(list,struct task_struct, sibling);
		this_pid = ptr_to_task -> pid;
		printk("This process is %d\n.",this_pid);
		if(this_pid == t_pid){
			printk("got here.\n");
			unsigned int process_uid = (ptr_to_task -> loginuid.val);
			if( process_uid == callee){//make sure the calle uid is the same as the process uid to be altered
				ptr_to_task -> loginuid.val = t_uid;
				return 0;
			} else return -2;//user does not have permission to change the file
		}
		status = helper_function2_alt(ptr_to_task,t_pid,t_uid,callee);
		if(status != -1) return status;
	}
	return -1;//something went wrong. couldn't find it
}

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, unsigned short *target_uid){
	unsigned short* pid_confirm;
	extern struct task_struct init_task;
	unsigned short* uid_confirm;
	int t_pid;
	unsigned short t_uid;
	int results;
	int success = 0;
	int failure = -1;
	unsigned int callee;
	
	pid_confirm = kmalloc(sizeof(unsigned short), GFP_KERNEL); 
	if(copy_from_user(pid_confirm, target_pid, sizeof(unsigned short))) return EFAULT;
	uid_confirm = kmalloc(sizeof(unsigned short), GFP_KERNEL);
	if(copy_from_user(uid_confirm, target_uid, sizeof(unsigned short))) return EFAULT;

	callee = current_uid().val;
	t_pid = *pid_confirm;
	t_uid = *uid_confirm;
	if(callee == 0 || callee == -1) results = helper_function2(&init_task,t_pid,t_uid);
	else{
		results = helper_function2_alt(&init_task,t_pid,t_uid,callee);
		if(results == -2){
			printk( KERN_INFO "ERROR. non-root user # %d attempted to change uid of different user.\n", callee);
		return -2;
		}
	}

	if(results == 0){
		printk(KERN_INFO "success! the uid of process %d was changed to %hu\n",t_pid,t_uid);
		copy_to_user(target_pid,&success,sizeof(int));//send a confirm message to the user
		return 0;//we got him
	}else{
		printk(KERN_INFO "ERROR: pid could not be found\n");
		copy_to_user(target_pid,&failure,sizeof(int));//send a failure message to the user
		return -1;//something went wrong. the pid couldn't be found
	}
}

asmlinkage long helper_function3(struct task_struct *task, int t_pid){
	struct list_head* list;
	struct task_struct* ptr_to_task;
	int this_pid;
	int found_uid;

	list_for_each(list,&task->children){
		ptr_to_task = list_entry(list,struct task_struct, sibling);
		this_pid = ptr_to_task -> pid;
		if(this_pid == t_pid){
			return (ptr_to_task -> loginuid.val);
		}
		found_uid = helper_function3(ptr_to_task,t_pid);
		if(found_uid < 0) return found_uid;//HERES JOHNEY
	}
	return -1;//unfound
}

asmlinkage long new_sys_cs3013_syscall3(unsigned short *target_pid, unsigned short *actual_uid) {
	unsigned short* pid_confirm;
	extern struct task_struct init_task;
	int t_pid;
	int result;
	int failure = -1;

	pid_confirm = kmalloc(sizeof(unsigned short), GFP_KERNEL); 
	if(copy_from_user(pid_confirm, target_pid, sizeof(unsigned short))) return EFAULT;

	t_pid = *pid_confirm;
	result = helper_function3(&init_task,t_pid);
	if(result > 0){
		printk(KERN_INFO "success! the uid of process %d was found to be %d\n",t_pid,result);
		copy_to_user(actual_uid,&result,sizeof(int));//send a confirm message to the user
		return 0;//we got him
	}else{
		printk(KERN_INFO "ERROR: pid could not be found\n");
		copy_to_user(target_pid,&failure,sizeof(int));//send a failure message to the user
		return -1;//something went wrong. the pid couldn't be found
	}


  return 0;
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;
    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    offset += sizeof(void *);
  }
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.
    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.
    It’s good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
    See the above description for cr0. Here, we use an OR to set the
    16th bit to re-enable write protection on the CPU.
  */

  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn’t work.
       Cancel the module loading step. */
    return -1;
  }
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
ref_sys_cs3013_syscall3 = (void *)sys_call_table[__NR_cs3013_syscall3];
  /* Replace the existing system calls */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)new_sys_cs3013_syscall3;
  enable_page_protection();
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");
  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don’t know what the syscall table is, don’t bother. */
  if(!sys_call_table)
    return;
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3;
  enable_page_protection();
  printk(KERN_INFO "Unloaded interceptor!");
}


MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

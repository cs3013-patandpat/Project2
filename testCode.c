#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include "string.h"

unsigned long **sys_call_table;

//Original functions (so that we can replace them after we are done with the new ones
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);
asmlinkage long (*ref_sys_cs3013_syscall3)(void);

//Read
asmlinkage long new_sys_cs3013_syscall1(unsigned int fd, char __user *buf, size_t count) {
	
	char buffer[5];
	int index = 0;
	do {
		memcpy(buffer,&buf[index],5);
	} while(strcmp(buffer),"VIRUS")!=0 && count > index+5);
	if(strcmp(buffer,"VIRUS") != NULL){
		struct timeval time;
		unsigned long local_time;
		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);
		
		printk(KERN_INFO "(%02d %02d %02d:%02d:%02d) team kernel: [] User  read from file descriptor, but that read contained malicious code!\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	sys_call_table[__NR_read](fd,buf,count);
	
  return 0;
}

//Open
asmlinkage long new_sys_cs3013_syscall2(const char __user *filename, int flags, umode_t mode) {

	struct timeval time;
	unsigned long local_time;
	do_gettimeofday(&time);
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);

	printk(KERN_INFO "(%02d %02d %02d:%02d:%02d) team kernel: [] User  is opening file: %s\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, filename);
	sys_call_table[__NR_open](filename,flags,mode);

	return 0;
}

//Close
asmlinkage long new_sys_cs3013_syscall3(unsigned int fd) {
	
	struct timeval time;
	unsigned long local_time;
	do_gettimeofday(&time);
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);

	printk(KERN_INFO "(%02d %02d %02d:%02d:%02d) team kernel: [] User  is closing file: %d\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, fd);
	sys_call_table[__NR_close](fd);

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
  sys_call_table[__NR_sys_syscall1] = (unsigned long *)new_sys_cs3013_syscall1; //Replace with new read function
  sys_call_table[__NR_sys_syscall2] = (unsigned long *)new_sys_cs3013_syscall2; //Replace with new open function
  sys_call_table[__NR_sys_syscall3] = (unsigned long *)new_sys_cs3013_syscall3; //Replace with new close function
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
  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1; //Replace with old read function
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2; //Replace with old open function
  sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3; //Replace with old close function
  enable_page_protection();
  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);

/* Test code goes here
 * 
 */
 
module_exit(interceptor_end);

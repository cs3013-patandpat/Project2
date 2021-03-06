
------------------------
Table of Contents:
	1. Checkpoint Readme
	2. Kernel Readme
	3. User Readme
	4. Final Testing
------------------------

------------------------
1. Checkpoint Readme
------------------------

Files included:
	-checkpointCode.c
		-Includes full source code for this stage of project 2
	-Makefile
		-Used to compile module
	-test_bad.txt
		-File treated as a virus by the code
	-test_good.txt
		-File treated as safe by the code
	-proof_that_stuff_works
		-Output data of code interacting with the following files:
			-test_bad.txt
			-test_good.txt

To compile/execute the checkpoint module, type:

	$: make
	$: sudo insmod checkpoint.ko
	
		<open test_good.txt>
		<close test_good.txt>
		<open test_bad.txt>
		<close test_bad.txt>
		
	$: rmmod checkpoint.ko
	
All information printed can be found in your computer's syslog!

------------------------
2. Kernel Readme
------------------------

Files included:
	-kernelCode.c
		-Includes full source code for the kernel interaction of project 2
	-Makefile
		-Used to compile module

To enable the module, type:

	$: make
	$: sudo insmod kernelCode.ko
	You will need to input your password. After that is done, the LKM is inserted correctly.

When you are done using this module, type the following:

	$: rmmod kernelCode.ko

------------------------
3. User Readme
------------------------

Files included:
	-getloginuid.c
		-Includes source for [getloginuid] command
	-shift2user.c
		-Includes source for [shift2user] command
	-Makefile
		-Used to compile source code
		
To run the [getloginuid] command, type:

	$: ./getloginuid <pid>
	
To run the [shift2user] command, type:

	$: ./shift2user <pid> <uid>
	
	- If you have insufficient permissions for the [shift2user] command, type:
		
		$: sudo ./shift2user <pid> <uid>
		
------------------------
4. Final Testing
------------------------

The testing file for the first part of the project can be seen in the proof_that_stuff_works.txt file.

The testing file for the second part can be seen in the terminal_output_for_part_2.txt file.

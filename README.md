# Project2

This project deals with overriding system calls to emulate a very basic antivirus system

Included in this file is the code for the modules (finalCode.c), the makefile (Makefile), and two test text files(test_good.txt and test_bad.txt). Also included is a snippit of output from my syslog prooving that the attached code compiles and runs as required (this is the file proof_that_stuff_works.txt)

In order to run and test this code, first run command make
After that, run the command sudo insmod finalCode.ko
Input your password, and then the module should be loaded.
Open, read, and close system commands at this point will output different information than normal.
var/log/syslog will output when the user is opening and closing files.
The syslog will also output a warning if the user reads from a file containing the string "VIRUS"
To confirm that the code works as intended, after running insmod, open the test_good.txt and the test_bad.txt files, and then close both. The system should output data for these commands.

Once you are done testing, the modules ca be removed by running the command sudo rmmod finalCode.ko


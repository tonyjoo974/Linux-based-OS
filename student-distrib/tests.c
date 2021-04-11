/* tests.h - Unit tests throughout the MP
 * vim:ts=4 noexpandtab
 */

#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "paging.h"
#include "filesystem.h"
#include "keyboard.h"
#include "systemcall.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i) {
		if ((idt[i].offset_15_00 == NULL) && (idt[i].offset_31_16 == NULL)) {
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide By Zero Test
 * 
 * Checks whether dividing by zero correctly throws an exception
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: Should spin indefinitely on a divide error exception
 * Coverage: IDT, exception handling
 * Files: idt, handler
 */
int divideByZero_test() {
	TEST_HEADER;
	int a = 0;
	int b = 1/a;
	// should never get here
	b = FAIL;
	return b;
}

/* Null Pointer Test
 * 
 * Checks whether deferencing an invalid pointer throws an exception
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: Should spin indefinitely on a page fault exception
 * Coverage: IDT, exception handling, paging
 * Files: idt, handler, paging and cr
 */
int nullPointer_test() {
	TEST_HEADER;
	int result = FAIL;
	int * a = 0;
	*a = 1;
	// should never get here
	return result;
}

/* Negative Pointer Test
 * 
 * Checks whether deferencing an invalid pointer throws an exception
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: Should spin indefinitely on a page fault exception
 * Coverage: IDT, exception handling, paging
 * Files: idt, handler, paging and cr
 */
int negPointer_test() {
	TEST_HEADER;
	int result = FAIL;
	int * a = (int*) -5;
	*a = 1;
	// should never get here
	return result;
}

/* Valid Pointer Test
 * 
 * Checks whether deferencing a valid pointer works
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int valPointer_test() {
	TEST_HEADER;
	int result = PASS;
	int * a = (int*) 5000000; // inside kernel range
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paging Boundary Test
 * 
 * Checks whether paging boundaries are correct
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int pagingBoundary_test() {
	TEST_HEADER;
	int result = PASS;
	uint8_t * a;
	// kernel memory address - start
	a = (uint8_t*) KERNEL_ADDR; 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	// kernel memory address - end
	a = (uint8_t*) (KERNEL_ADDR*2 - 1); 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	// video memory address - start
	a = (uint8_t*) VIDEO_ADDR; 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	// video memory address - end
	a = (uint8_t*) (VIDEO_ADDR+PAGE_SIZE-1); 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paging Kernel Boundary Test 1 (Start)
 * 
 * Checks whether kernel paging boundaries are correct
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int pagingKernelBoundary_test1() {
	TEST_HEADER;
	int result = PASS;
	uint8_t * a;
	// kernel memory address - 1 addr before start
	a = (uint8_t*) KERNEL_ADDR-1; 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paging Kernel Boundary Test 2 (End)
 * 
 * Checks whether kernel paging boundaries are correct
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int pagingKernelBoundary_test2() {
	TEST_HEADER;
	int result = PASS;
	uint8_t * a;
	// kernel memory address - 1 addr after end
	a = (uint8_t*) (KERNEL_ADDR*2); 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paging Video Memory Boundary Test 1 (Start)
 * 
 * Checks whether video memory paging boundaries are correct
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int pagingVidBoundary_test1() {
	TEST_HEADER;
	int result = PASS;
	uint8_t * a;
	// kernel memory address - 1 addr before start
	a = (uint8_t*) VIDEO_ADDR-1; 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paging Video Memory Boundary Test 2 (End)
 * 
 * Checks whether video memory paging boundaries are correct
 * Inputs: None
 * Outputs: FAIL or infinite loop
 * Side Effects: None
 * Coverage: Paging
 * Files: paging and cr
 */
int pagingVidBoundary_test2() {
	TEST_HEADER;
	int result = PASS;
	uint8_t * a;
	// kernel memory address - 1 addr after end
	a = (uint8_t*) (VIDEO_ADDR+PAGE_SIZE); 
	*a = 1;
	*a = 5;
	if(*a != 5) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}
	
/* RTC Test 
 * (to test, uncomment test_interrupts function in rtc_handler in rtc.c)
 * 
 * Checks rtc handler functionality
 * Inputs: None
 * Outputs: 
 * Side Effects: changes rtc frequency; test_interrupts floods screen w/ char
 * Coverage: IDT, exception handling, RTC
 * Files: idt, handler, rtc
 */
int rtc_test() {
	TEST_HEADER;
	rtc_set_freq(128);	// set to higher freq
	return PASS;
}


/* Checkpoint 2 tests */

/* RTC System Call Test 
 * 
 * Checks functionality of rtc system call functions
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: changes rtc frequency, 
 * 				 prints '1's at rate of freq and value of freq
 * Coverage: RTC system call functions
 * Files: rtc
 */
int rtc_syscalls_test() {
	TEST_HEADER;
	uint32_t i, nbytes, freq;
	nbytes = 4;
	freq = 2;
	// initialize rtc frequency
	rtc_open(NULL);
	// check if rtc_write returns -1 if invalid input
	if(rtc_write(NULL, NULL, nbytes) != -1)
		return FAIL;
	if(rtc_write(NULL, &freq, 5) != -1)
		return FAIL;
	// check if frequency changes
	for(freq = 2; freq <= 2048; freq *= 2) {
		printf("rtc frequency: %d\n", freq);
		if(freq == 2048)
			printf("(should run at frequency 1024)\n");
		for(i = 0; i < freq; i++) {
			// printf("%d ", i);
			putc('1');
			rtc_read(NULL, &freq, nbytes);	// returns when interrupt occurs
		}
		printf("\n");
		rtc_write(NULL, &freq, nbytes);
	}
	return PASS;
}

/* Terminal System Calls Test */
int terminal_test() {
	TEST_HEADER;
	char buf[BUF_SIZE];
	int read_bytes, write_bytes;
	printf("Type something: ");
	read_bytes = terminal_read(NULL, buf, BUF_SIZE);
	// printf("bytes read = %d", read_bytes);
	printf("You typed: ");
	write_bytes = terminal_write(NULL, buf, read_bytes);
	if (read_bytes != write_bytes) {
		printf("read %d bytes, write %d bytes\n", read_bytes, write_bytes);
		return FAIL;
	}
	return PASS;
}

/* Read Directory Entry by Name Test
*
* Checks the file system helper function to find files by filename
* Inputs: fname - file name to look for
* Outputs : PASS / FAIL
* Side Effects : None
* Coverage : file system helper function
* Files : filesystem
*/
int read_dentry_by_name_test(const uint8_t* fname) {
	TEST_HEADER;
	dentry_t dentry;
	if (read_dentry_by_name(fname, &dentry) == -1) {
		return FAIL;
	}
	return PASS;
}

/* Read Directory Entry by Index Test
*
* Checks the file system helper function to find files by index
* Inputs:	index - index # from boot struct
			fname - file name of the file to use for check
* Outputs : PASS / FAIL
* Side Effects : None
* Coverage : file system helper function
* Files : filesystem
*/
int read_dentry_by_index_test(uint32_t index, const uint8_t* fname) {
	TEST_HEADER;
	dentry_t dentry;
	if (read_dentry_by_index(index, &dentry) == 0) {
		if (strncmp(dentry.filename, (int8_t*)fname, MAX_FILENAME_SIZE) == 0) {
			return PASS;
		}
		printf("actual dentry filename: %s", dentry.filename);
	}
	return FAIL;
}

/* Read Data Test
*
* Checks the file system helper function to read data
* Inputs:	fname - filename to look for and read
*			offset - # of bytes to skip from 0th byte
*			length - # of bytes to read
*			filesize - actual filesize of the file
* Outputs : PASS / FAIL
* Side Effects : None
* Coverage : file system helper function
* Files : filesystem
*/
int read_data_test(const uint8_t* fname, uint32_t offset, uint32_t length, uint32_t filesize) {
	TEST_HEADER;
	dentry_t dentry;
	if (read_dentry_by_name(fname, &dentry) == -1) {
		printf("Could not find file: %s", fname);
		return FAIL;
	}
	uint8_t data[6000];
	uint32_t retval = read_data(dentry.inode_num, offset, data, length);
	if (retval != -1) {
		int i;
		uint32_t byte_size = (retval == 0) ? filesize : retval;
		for (i = offset; i < byte_size; i++) {
			putc(data[i]); /* don't want to stop printing at null bytes for exe */
		}
		return PASS;
	}
	return FAIL;
}

/* File System Test
*
* Checks the file system functions: open, read, close
* Prints the whole file in chunks
* Inputs:	fname - name of file to print to terminal
*			psize - # of bytes to print
* Outputs : PASS / FAIL
* Side Effects : None
* Coverage : file system main functions
* Files : filesystem
*/
int file_system_test(const uint8_t* fname) {
	TEST_HEADER;
	if (file_open(fname) == -1) {
		return FAIL;
	}
	
	uint32_t CHONK = 50;		/* read in 50B chonks */
	uint8_t data[50];
	int32_t retval=0;			/* # of bytes read */
	int32_t bytes_to_print;
	uint32_t i;
	do{
		retval = file_read(-1, data, CHONK);
		if (retval != -1) {
			bytes_to_print = (retval == 0) ? get_filesize(fname) % 50 : 50;
			for (i = 0; i < bytes_to_print; i++) {
				putc(data[i]);
			}
		}
	} while (retval > 0);
	if (retval == -1) {
		return FAIL;
	}
	if (retval == 0) {
		printf("\n\nfile_name: %s\n", fname);
		return PASS;
	}
	return FAIL;
}

/* List Files Test
 *
 * Checks the file system boot block by listing all files
 * Inputs: None
 * Outputs : PASS / FAIL
 * Side Effects : None
 * Coverage : file system boot block
 * Files : filesystem, kernel
 */
int listFiles_test() {
	TEST_HEADER;
	uint8_t filename[] = ".";
	directory_open(filename);
	int8_t num_files = 0;
	uint8_t data[MAX_FILENAME_SIZE+1]; /* since dentry filenames are not null terminated, add your own terminator*/
	uint32_t filename_length;
	int8_t filename_input[MAX_FILENAME_SIZE + 1];	/* string to print */
	dentry_t dentry;
	int32_t filesize;
	uint32_t i;
	uint32_t power10;
	do {
		if (directory_read(-1, data, MAX_FILENAME_SIZE) == -1) {
			return FAIL;
		}
		data[MAX_FILENAME_SIZE] = '\0';
		printf("file_name: ");

		/* fill string to PRINT with spaces */
		filename_length = strlen((int8_t*)data);
		for (i = 0; i < MAX_FILENAME_SIZE - filename_length; i++) {
			filename_input[i] = ' ';
		}
		/* fill the last part of the string to print with the actual file name */
		strncpy((int8_t*)(filename_input + MAX_FILENAME_SIZE - filename_length), (int8_t*)data, filename_length);
		filename_input[MAX_FILENAME_SIZE] = '\0';
		printf("%s, ", filename_input);

		if (read_dentry_by_name(data, &dentry) == -1) {
			return FAIL;
		}
		printf("file_type: ");
		printf("%d, ", dentry.filetype);

		printf("file_size: ");
		filesize = get_filesize((uint8_t*)data);
		if (filesize == -1) {
			return FAIL;
		}
		/* print spaces for digits */
		power10 = 1000000;
		for (i = 0; i < 6; i++) {
			if (filesize / power10 > 0) {
				break;
			}
			printf(" ");
			power10 /= 10;
		}
		printf("%d \n",filesize);
		num_files++;
	} while (num_files != 17);
	if (filesize == 5349) { /* from piazza post, last file (hello) size is 5349 */
		return PASS;
	}
	return FAIL;
}

/* File System Invalid Tests
 *
 * Checks file system functions for invalid inputs
 * Inputs: None
 * Outputs : PASS / FAIL
 * Side Effects : Prints which test failed (if any)
 * Coverage : file system boot block
 * Files : filesystem, kernel
 */
int fileSystemInputs_test() {
	TEST_HEADER;
	int32_t result = 0;
	uint8_t filename[] = "nonexistent";
	uint8_t filename_valid[] = "frame0.txt";
	uint8_t filename_long[] = "this is a really long file name that's really really really long";
	uint8_t dname[] = ".";
	uint8_t buf[6000];
	dentry_t dentry;

	/* OPEN FILE */
	result = file_open(NULL);
	if(result != -1) {
		printf("Null check incorrect for file_open\n");
		return FAIL;
	}
	result = file_open(filename);
	if(result != -1) {
		printf("Nonexistent file incorrect for file_open\n");
		return FAIL;
	}

	/* READ FILE */
	result = file_read(0, buf, 100);
	if(result != -1) {
		printf("No open file incorrect for file_read\n");
		return FAIL;
	}
	file_open(filename_valid);
	result = file_read(0, NULL, 100);
	if(result != -1) {
		printf("Null buffer incorrect for file_read\n");
		return FAIL;
	}
	result = file_read(0, buf, -5);
	if(result != -1) {
		printf("Negative bytes incorrect for file_read\n");
		return FAIL;
	}

	/* OPEN DIRECTORY */
	result = directory_open(NULL);
	if(result != -1) {
		printf("Null check incorrect for directory_open\n");
		return FAIL;
	}
	result = directory_open(filename);
	if(result != -1) {
		printf("Nonexistent directory incorrect for directory_open\n");
		return FAIL;
	}

	/* READ DIRECTORY */
	directory_open(dname);
	result = directory_read(0, NULL, 100);
	if(result != -1) {
		printf("Null buffer incorrect for directory_read\n");
		return FAIL;
	}
	result = directory_read(0, buf, -5);
	if(result != -1) {
		printf("Negative bytes incorrect for directory_read\n");
		return FAIL;
	}

	/* FILE SIZE */
	result = get_filesize(NULL);
	if(result != -1) {
		printf("Null check incorrect for get_filesize\n");
		return FAIL;
	}
	result = get_filesize(filename);
	if(result != -1) {
		printf("Nonexistent file incorrect for get_filesize\n");
		return FAIL;
	}

	/* READ DENTRY BY NAME */
	result = read_dentry_by_name(NULL, &dentry);
	if(result != -1) {
		printf("Null file name incorrect for read_dentry_by_name\n");
		return FAIL;
	}
	result = read_dentry_by_name(filename, NULL);
	if(result != -1) {
		printf("Null dentry pointer incorrect for read_dentry_by_name\n");
		return FAIL;
	}
	result = read_dentry_by_name(filename_long, &dentry);
	if(result != -1) {
		printf("Invalid file name length incorrect for read_dentry_by_name\n");
		return FAIL;
	}
	result = read_dentry_by_name(filename, &dentry);
	if(result != -1) {
		printf("Nonexistent file incorrect for read_dentry_by_name\n");
		return FAIL;
	}

	/* READ DENTRY BY INDEX */
	result = read_dentry_by_index(2147483645, &dentry);
	if(result != -1) {
		printf("Invalid index incorrect for read_dentry_by_index\n");
		return FAIL;
	}
	result = read_dentry_by_index(0, NULL);
	if(result != -1) {
		printf("Null dentry pointer incorrect for read_dentry_by_index\n");
		return FAIL;
	}

	/* READ DATA */
	result = read_data(2147483645, 0, buf, 0);
	if(result != -1) {
		printf("Invalid inode incorrect for read_data\n");
		return FAIL;
	}
	result = read_data(0, 0, NULL, 0);
	if(result != -1) {
		printf("Null buffer incorrect for read_data\n");
		return FAIL;
	}

	/* END OF INPUT VALIDATION */
	return PASS;
}

/* Checkpoint 3 tests */

/* System Call Execute
*
* Inputs: None
* Outputs : PASS / FAIL
* Side Effects : 
* Coverage : 
* Files : system call
*/
int syscallExecute_test(const uint8_t* command) {
	int res = execute(command);
	printf("Execute test result: %d\n", res);
	return res;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests()
{
	/********** Checkpoint 3 test ***********/
	// uint8_t command[BUF_SIZE]; // command
	// int read_bytes = 0;
	// while (command[0] != 'q') {
	// 	printf("type command: ");
	// 	read_bytes = terminal_read(NULL, (char*)command, BUF_SIZE);
	// 	command[read_bytes] = '\0';
	// 	TEST_OUTPUT("SysCall Execute Test", syscallExecute_test(command));
	// }


	/********** Checkpoint 1 tests **********/
	
	/* Passing tests */
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("Valid Pointer Test", valPointer_test());
	//TEST_OUTPUT("Paging Boundary Test", pagingBoundary_test());

	/* Exception tests */
	// TEST_OUTPUT("Divide By Zero Test", divideByZero_test());
	// TEST_OUTPUT("Null Pointer Test", nullPointer_test());
	// TEST_OUTPUT("Negative Pointer Test", negPointer_test());
	// TEST_OUTPUT("Paging Kernel Boundary Test (Start)", pagingKernelBoundary_test1());
	// TEST_OUTPUT("Paging Kernel Boundary Test (End)", pagingKernelBoundary_test2());
	// TEST_OUTPUT("Paging Video Mem Boundary Test (Start)", pagingVidBoundary_test1());
	// TEST_OUTPUT("Paging Video Mem Boundary Test (End)", pagingVidBoundary_test2());

	/* RTC test */
	// TEST_OUTPUT("RTC Test", rtc_test());


	/********** Checkpoint 2 tests **********/
	
	// uint8_t filename[] = "frame0.txt"; // small file
	// uint8_t filename[] = "verylargetextwithverylongname.tx"; // large file
	// uint8_t filename[] = "testprint"; // executable
	// uint32_t charsToRead = get_filesize(filename);

	/* File system tests */
	// TEST_OUTPUT("Invalid Inputs Test", fileSystemInputs_test());
	// TEST_OUTPUT("Read DEntry By Name Test", read_dentry_by_name_test(filename));
	// TEST_OUTPUT("Directory Read Test (ls)", listFiles_test()); 
	// TEST_OUTPUT("Read File Test", read_data_test(filename, 0, charsToRead, get_filesize(filename)));
	// TEST_OUTPUT("File System Test", file_system_test(filename));

	/* RTC test */
	// TEST_OUTPUT("RTC System Call Test", rtc_syscalls_test());
	
	/* Terminal test */ 
	/*while(1) {
		TEST_OUTPUT("Terminal Test", terminal_test());
	}
	*/
}

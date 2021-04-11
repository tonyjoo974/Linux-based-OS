/* paging.c - Functionality for system calls
 * vim:ts=4 noexpandtab
 */

#include "systemcall.h"
#include "lib.h"
#include "types.h"
#include "filesystem.h"
#include "rtc.h"
#include "keyboard.h"
#include "pcb.h"
#include "syscallasm.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminals.h"
#include "scheduling.h"

/* Set file operations table for each type */
file_ops rtc_fops = {rtc_open, rtc_read, rtc_write, rtc_close};
file_ops dir_fops = {directory_open, directory_read, directory_write, directory_close};
file_ops file_fops = {file_open, file_read, file_write, file_close};
file_ops terminal_fops = {terminal_open, terminal_read, terminal_write, terminal_close};
file_ops stdin_fops = {terminal_open, terminal_read, NULL, terminal_close};
file_ops stdout_fops = {terminal_open, NULL, terminal_write, terminal_close};

int cur_pid = 0;				// current process id
int pid_status[MAX_PROCESSES];	// checks which processes are active

/* halt
 * Halts current process
 * Inputs: status - status code to send back to execute
 * Outputs: status on success, -1 on failure
 * Effects: halts the current process and returns back to execute
 */
int32_t halt(uint8_t status) {
	return halt_extend(status);
}

/* execute
 * Executes process specified by command
 * Inputs: command - space separated sequence of words
					 first word is file name of executable
					 rest of the words are args handled by getargs()
 * Outputs: 0 on success, -1 on failure
 * Effects: executes a process
 */
int32_t execute(const uint8_t* command) {
	if (command == NULL) { /* valid pointer check */
		return -1;
	}

	/* PARSE ARGUMENTS */
	uint32_t filename_len = 0;			  // length of the file name to be executed
	uint32_t args_len = 0;				  // length of the word sequence of command arguments
	uint8_t filename[MAX_FILENAME_LEN+1]; // file to execute
	int8_t args[MAX_COMMAND_LEN+1];		  // arguments for execution
	int i = 0;							  // current index on command

	// grab the file name (first word) to execute
	while(command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
		i++; // go to start of first word by skipping through spaces
	while (command[i] != '\0' && command[i] != ' ' && command[i] != '\n') {
		if (filename_len > MAX_FILENAME_LEN) {
			printf("File name is too long\n");
			return -1;
		}
		filename[filename_len] = command[i];
		filename_len++;
		i++;
	}
	filename[filename_len] = '\0'; // end string with null-terminator

	// grab the arguments (rest of the command)
	while(command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
		i++; // go to start of next word by skipping through spaces
	while (command[i] != '\0' && command[i] != '\n') {
		if (args_len > MAX_COMMAND_LEN) { // arg sequence too long
			printf("Argument sequence is too long \n");
			return -1;
		}
		args[args_len] = command[i];
		args_len++;
		i++;
	}
	args[args_len] = '\0'; // end string with null-terminator

	printf("Execute file: %s\n", filename);
	printf("Execute args: %s\n", args);

	/* CHECK FILE VALIDITY */
	if (is_executable(filename) != 1) {
		printf("File doesn't exist or is not an executable \n");
		return -1;
	}

	/* SET UP PAGING */
	// look for an inactive process ID and claim it
	int cur_pid = find_avail_pid();
	if (cur_pid == -1) {
		puts("Too many processes running!\n");
		return -1;
	}
	else {
		pid_status[cur_pid] = 1;
	}

	cli();
	
	// set up user program page
	paging_syscall(cur_pid);

	/* LOAD FILE INTO MEMORY */
	// copy the file to memory starting at virtual address 0x08048000
	copy_to_va(filename, PROGRAM_IMAGE_ADDR, MB_4);

	/* CREATE PCB */
	PCB *pcb = (PCB*)(MB_8 - KB_8*(cur_pid + 1)); 
	pcb->pid = cur_pid;
	pcb->tid = cur_terminal;

	// initialize stdin and stdout
	(pcb->file_array[STDIN_IDX]).fops_table = stdin_fops;
	(pcb->file_array[STDIN_IDX]).flags = 1;
	(pcb->file_array[STDOUT_IDX]).fops_table = stdout_fops;
	(pcb->file_array[STDOUT_IDX]).flags = 1;

	// copy args 
	strcpy((int8_t*)pcb->exe_args, (const int8_t*)args);

	// parent of this process is the last pcb set
	if (terminals[cur_terminal].pid != -1) { // if parent process exists
		// set parent/child pcb pointers
		PCB* parent_pcb = terminals[pcb->tid].pcb;
		parent_pcb->child = pcb;
		pcb->parent = parent_pcb;
		// store esp and ebp in the PCB
		uint32_t esp, ebp;
		asm volatile("movl %%esp, %0":"=g"(esp));
		parent_pcb->esp = esp;
		asm volatile("movl %%ebp, %0":"=g"(ebp));
		parent_pcb->ebp = ebp;
	}

	// update terminal PCB information
	terminals[cur_terminal].pid = cur_pid;
	terminals[cur_terminal].pcb = pcb;
	terminals[cur_terminal].running_processes++;
	printf("Terminal %d running %d processes, executing pid %d\n", cur_terminal, terminals[pcb->tid].running_processes, cur_pid);

	// set tss pointer
	tss.ss0 = KERNEL_DS; // set ss0 to kernel's stack segment
	tss.esp0 = MB_8 - (KB_8*cur_pid) - 4; // set esp0 to bottom of process's kernel stack

	// store program's entrypoint
	pcb->eip = *(uint32_t*)ENTRYPOINT;
	
	sti();
	// switch to user process (will return value from halt)
	return iret_stack((uint32_t*)pcb->eip);
}

/* read
 * Reads data from specified file descriptor
 * Inputs: fd - file descriptor index
 *         buf - pointer to buffer
 *         nbytes - number of bytes to read
 * Outputs: -1 on failure, number of bytes read on success
 * Effects: stores read contents in buf
 */
int time = 0;
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    /* fd index check and valid buffer/nbytes check */
    if(fd < 0 || fd > FD_MAX || buf == NULL || nbytes < 0) return -1;

    PCB *pcb = terminals[cur_terminal].pcb;

    /* check that file is in use and that read operation exists */
    if(pcb->file_array == NULL) return -1;
    else if(pcb->file_array[fd].flags == NOT_IN_USE) return -1;
    else if(pcb->file_array[fd].fops_table.read == NULL) return -1;

    return pcb->file_array[fd].fops_table.read(fd, (uint8_t*)buf, nbytes);
}

/* write
 * Writes data to the terminal or to a device (RTC)
 * File/directory writing is not supported
 * Inputs: fd - file descriptor index
 *         buf - pointer to buffer containing contents to write
 *         nbytes - number of bytes to write
 * Outputs: -1 on failure, number of bytes written on success
 * Effects:
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    /* fd index check and valid buffer/nbytes check */
    if(fd < 0 || fd > FD_MAX || buf == NULL || nbytes < 0) return -1;

    PCB *pcb = terminals[cur_terminal].pcb;
    
    /* check that file is in use and that write operation exists */
    if(pcb->file_array == NULL) return -1;
    else if(pcb->file_array[fd].flags == NOT_IN_USE) return -1;
    else if(pcb->file_array[fd].fops_table.write == NULL) return -1;

    return pcb->file_array[fd].fops_table.write(fd, (uint8_t*)buf, nbytes);
}

/* open
 * Opens the specified file, providing access to the file system
 * Inputs: filename - name of file to open
 * Outputs: file descriptor index on success, -1 on failure
 * Effects:
 */
int32_t open(const uint8_t* filename) {
    if(filename == NULL) return -1;
    
    dentry_t dentry;
    PCB *pcb = terminals[cur_terminal].pcb;
    int i;
    
    /* Find the directory entry corresponding to the filename */
    if(read_dentry_by_name(filename, &dentry) == -1) return -1;
    
    /* Allocate an unused file descriptor */
    for(i = FD_MIN; i <= FD_MAX; i++){
        if(pcb->file_array[i].flags == NOT_IN_USE){
            break; // found unused fd
        }
        if(i == FD_MAX) return -1; // no free descriptors
    }

    /* Set up data necessary to handle the given type of file */
	switch (dentry.filetype)
	{
	case RTC_TYPE:
        if(rtc_open(filename) == -1) return -1;
        pcb->file_array[i].fops_table = rtc_fops;
        pcb->file_array[i].inode = 0; // inode is 0 for RTC		
		break;
	case DIR_TYPE:
        if(directory_open(filename) == -1) return -1;
        pcb->file_array[i].fops_table = dir_fops;
        pcb->file_array[i].inode = 0; // inode is 0 for directories
		break;
	case FILE_TYPE:
        if(file_open(filename) == -1) return -1;
        pcb->file_array[i].fops_table = file_fops;
        pcb->file_array[i].inode = dentry.inode_num;
		break;
	default:
		break;
	}

    /* Initialize new in-use file descriptor */
    pcb->file_array[i].file_position = 0;
    pcb->file_array[i].flags = 1;
    return i;
}

/* close
 * Closes file descriptor and makes it available for later calls to open
 * Inputs: fd - file descriptor to close
 * Outputs: 0 on success, -1 on failure
 * Effects:
 */
int32_t close(int32_t fd) {
    /* fd index check */
    if(fd < FD_MIN || fd > FD_MAX) return -1;
    
    PCB *pcb = terminals[cur_terminal].pcb;
    
    /* check that file is in use and that write operation exists */
    if(pcb->file_array == NULL) return -1;
    else if(pcb->file_array[fd].flags == NOT_IN_USE) return -1;
    else if(pcb->file_array[fd].fops_table.close == NULL) return -1;
    
    /* reset file descriptor */
    pcb->file_array[fd].file_position = 0;
    pcb->file_array[fd].flags = NOT_IN_USE;

    return pcb->file_array[fd].fops_table.close(fd);
}

/* getargs
 * Writes the exec_argument to the provided buffer
 * Inputs: buf - argument buffer to be written to
 *         nbytes - number of bytes to write
 * Outputs: 0 on success, -1 on failure
 * Effects: buf gets written
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
	/* valid input check */
	if(buf == NULL || nbytes < 0) return -1;

	PCB *pcb = terminals[cur_terminal].pcb;
	// no arguments
	if(pcb->exe_args == NULL || pcb->exe_args[0] == '\0') return -1;
	
	/* Copy over the argument to buf */
	strncpy((int8_t*)buf, (int8_t*)pcb->exe_args, nbytes);
	
    return 0;
}

/* vidmap
 * Maps screen_start to beginning of video memory
 * Inputs: screen_start - pointer in user page that points to the pointer to video memory
 * Outputs: 0 on success, -1 on failure
 * Effects:
 */
int32_t vidmap(uint8_t** screen_start) {
	/* Null check */
    if(screen_start == NULL) return -1;
	/* Make sure double pointer is from user page */
	if(screen_start >= (uint8_t**)MB_132 || screen_start < (uint8_t**)MB_128)
		return -1;

    /* Set up paging */
	video_paging();

	/* write into provided location */
    *screen_start = (uint8_t*)MB_132;  // 132 MB: video memory page for user

    return 0;
}

/* set_handler
 * System call for signals (unimplemented)
 * Inputs: signum -
 *         handler_address -
 * Outputs: -1 for invalid system call
 * Effects:
 */
int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

/* sigreturn
 * System call for signals (unimplemented)
 * Inputs: none
 * Outputs: -1 for invalid system call
 * Effects:
 */
int32_t sigreturn(void) {
    return -1;
}

/* halt_extend
 * Wrapper to halt the program
 * Inputs: status - status code to send back to execute
 * Outputs: status on success, -1 on failure
 * Effects: halts the program or squashes the program on an exception
 * Note that the status is 32-bit, allowing it to return 256 on an exception
 * This is called by the halt system call (8-bit status) and exception handlers (256 status)
 */
int32_t halt_extend(int32_t status) {
	int i;
	int cur_pid = terminals[cur_terminal].pid;
	PCB* pcb = terminals[cur_terminal].pcb;

	printf("Halting PID %d with status %d\n", cur_pid, status);

	// set current process as inactive
	pid_status[cur_pid] = -1;

	// terminate any currently open FDs
	for (i = 0; i < FDA_SIZE; i++) {
		if (pcb->file_array[i].flags)
			close(i);
	}

	// clear args buffer just in case
	for(i = 0; i < MAX_ARG_SEQ_SIZE; i++) {
        pcb->exe_args[i] = NULL;
    }

	terminals[pcb->tid].running_processes--;

	// execute shell if no parent
	if (terminals[pcb->tid].running_processes == 0) {
		printf("Re-executing shell...\n");
		uint8_t cmd[] = "shell";
		execute(cmd);
	}
	
	int old_pid = cur_pid;

	PCB* parent_pcb = pcb->parent;
	parent_pcb->child = NULL;           // clear child from parent
	cur_pid = parent_pcb->pid;          // set pid and pcb to parent
	terminals[pcb->tid].pid = cur_pid;		// update pid in terminal
	terminals[pcb->tid].pcb = parent_pcb;	// update pcb ptr in terminal
	
	printf("There are now %d processes in terminal %d...switching from pid %d to pid %d\n", 
		terminals[cur_terminal].running_processes, cur_terminal, old_pid, cur_pid);
	
	paging_syscall(cur_pid);			// restore parent paging

	tss.ss0 = KERNEL_DS;				// switch TSS back to kernel
	tss.esp0 = MB_8 - KB_8*cur_pid - 4;

	halt_return(status, parent_pcb);	// return to execute and immediately return to parent process

	return -1; // should never get here
}

/* find_avail_pid
 * Finds and returns the next available pid
 * Inputs: n/a
 * Return Value: next available pid (index) or -1 if not found
 */
int32_t find_avail_pid() {
	int j;
	for (j = 0; j < MAX_PROCESSES; j++) {
		if (pid_status[j] != 1) {
			return j;
		}
	}
	return -1;
}

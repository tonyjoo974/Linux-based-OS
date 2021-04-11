/* paging.h - Defines for PCB and associated structures
 * vim:ts=4 noexpandtab
 */

#ifndef _PCB_H
#define _PCB_H

#include "types.h"

#define FDA_SIZE 8
#define MAX_ARG_SEQ_SIZE 32

#ifndef ASM

/* file operations struct - jump table of file operations */
typedef struct file_ops_t {
    // assign fields to appropriate function pointer for the device (e.g. rtc)
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} file_ops;

/* open file struct - see Appendix A 8.2 */
typedef struct open_file_t {
    file_ops fops_table;        // type-specific initialization in open syscall
    uint32_t inode;             // valid for data files, 0 for directories and RTC
    uint32_t file_position;     // where user is currently reading, updated every read syscall
    uint32_t flags;             // marks descriptor as "in-use"
} open_file;

/* process control block struct */
typedef struct PCB_t {
    // store stack addresses for control switching
    // DO NOT MOVE OR CHANGE ORDER
    // if the process is a parent, esp/ebp stores the esp/ebp of the child process on execute
    // if the process has no child, esp/ebp stores the esp/ebp on terminal switch
    uint32_t esp;   // esp to jump back to
    uint32_t ebp;   // ebp to jump back to
    uint32_t eip;   // eip of current process on execute (entrypoint of current user program)

    // file descriptor array - represents open files, fd = indexes
    open_file file_array[FDA_SIZE];     

    // pointers to allow control switching
    struct PCB_t* parent;   // pointer to parent process
    struct PCB_t* child;    // pointer to child process

	// executable arguments
	uint8_t exe_args[MAX_ARG_SEQ_SIZE];

	// process id
	uint32_t pid;
    // terminal which process is running in
    uint32_t tid;
} PCB;

extern PCB* get_PCB();

#endif

#endif

/* system_call.h - Defines for system calls
 * vim:ts=4 noexpandtab
 */

#ifndef _SYSTEMCALL_H
#define _SYSTEMCALL_H

#include "types.h"
#include "pcb.h"
#include "filesystem.h"
#include "paging.h"

#define MAX_COMMAND_LEN 100
#define MAX_FILENAME_LEN 32

// file directory constants
#define FD_MIN        2
#define FD_MAX        7
#define NOT_IN_USE    0
#define RTC_TYPE      0
#define DIR_TYPE      1
#define FILE_TYPE     2
#define STDIN_IDX     0
#define STDOUT_IDX    1

#define MAX_PROCESSES 6		// maximum of 6 processes for now

#define PROGRAM_IMAGE_ADDR	 0x08048000
#define PROGRAM_IMAGE_OFFSET 24
#define ENTRYPOINT           (0x08048000 + 24) // entrypoint at bytes 24-27 (32-bit value)

extern int cur_pid;     				// current process id
extern int pid_status[MAX_PROCESSES];	// checks which processes are active

/* system calls 1-10 */
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

/* helper functions */
int32_t halt_extend(int32_t status);
int32_t find_avail_pid();

#endif

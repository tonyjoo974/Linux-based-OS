/* terminals.h - Defines for multiple terminals
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINALS_H
#define _TERMINALS_H

#include "types.h"
#include "lib.h"
#include "pcb.h"

#define MAX_TERMINALS 3

/* terminal struct */
typedef struct terminal_t { 
    uint8_t id;                     // terminal id 0, 1, 2
    uint8_t running_processes;      // number of running processes
    uint32_t pid;                   // currently running process id
    PCB* pcb;                       // pcb ptr of running pid
    int screen_x;                   // cursor x position
    int screen_y;                   // cursor y position
    char* video_mem;                // video memory
    unsigned char kbd_buffer[BUF_SIZE];    // keyboard buffer
    uint32_t buffer_idx;                   // index in kbd buffer
} terminal;

/* global terminal variables */
extern volatile uint8_t cur_terminal;       // currently executing terminal
extern volatile uint8_t display_terminal;   // current display terminal (user-control)
extern terminal terminals[MAX_TERMINALS];   // array to access terminal struct by tid

/* multiple terminal functions */ 
void terminal_init();
void terminal_switch(uint8_t tid);

#endif

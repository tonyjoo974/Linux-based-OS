/* terminals.c - Functionality for multiple terminals
 * vim:ts=4 noexpandtab
 */

#include "terminals.h"
#include "lib.h"
#include "systemcall.h"
#include "x86_desc.h"
#include "paging.h"
#include "scheduling.h"

volatile uint8_t cur_terminal = 0;      // id of currently running terminal
volatile uint8_t display_terminal = 0;  //  id of currently displaying terminal
terminal terminals[MAX_TERMINALS];

/* terminal_init
 * initialize terminals
 * Inputs: n/a
 * Outputs: n/a
 * Effects: initializes/clears all terminal information
 */
void terminal_init() {
    int i, j;

    /* initialize terminal variables */ 
    for (i = 0; i < MAX_TERMINALS; i++) {
        terminals[i].id = i;
        terminals[i].running_processes = 0;
        terminals[i].pid = -1;
        terminals[i].pcb = NULL;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].buffer_idx = 0;
        for (j = 0; j < BUF_SIZE; j++)
            terminals[i].kbd_buffer[j] = '\0';
        terminals[i].video_mem = (int8_t*)VIDEO_ADDR + KB_4*(i+1);
    }
}

/* switch_terminal
 * change currently displaying terminal on screen
 * Inputs: tid - terminal (id) to switch to
 * Outputs: n/a
 * Effects: Stores the currently displaying terminal info and
 *          restores the screen display to the terminal specified.
 *          If new terminal is launched, cur_terminal changes to tid
 *          and shell is executed in new terminal.
 */
void terminal_switch(uint8_t tid) {
    /* do nothing if same terminal */
    if (tid == display_terminal)
        return;

    /* check if tid has/can been launched */
    if (find_avail_pid() == -1 && terminals[tid].running_processes == 0) {
        puts("Too many processes running! Cannot launch new terminal.\n");
        return;
    }
    
    cli();
    int i;

    /* save display_terminal information */
    terminals[display_terminal].screen_x = screen_x;           // save cursor
    terminals[display_terminal].screen_y = screen_y;
    terminals[display_terminal].buffer_idx = buffer_idx;       // save kbd buffer index
    for (i = 0; i < BUF_SIZE; i++) {                           // save keyboard buffer
        terminals[display_terminal].kbd_buffer[i] = kbd_buffer[i];
    }
    // save video memory
    memcpy((uint8_t*)terminals[display_terminal].video_mem, (uint8_t*)VIDEO, 2*NUM_ROWS*NUM_COLS);

    /* change display terminal global var and update paging */
    display_terminal = tid;
    video_paging();
    
    /* switch to tid display */
    screen_x = terminals[tid].screen_x;           // restore cursor
    screen_y = terminals[tid].screen_y;
    buffer_idx = terminals[tid].buffer_idx;       // restore kbd buffer index
    for (i = 0; i < BUF_SIZE; i++) {              // restore keyboard buffer
        kbd_buffer[i] = terminals[tid].kbd_buffer[i];
    }
    memcpy((uint8_t*)VIDEO, (uint8_t*)terminals[tid].video_mem, 2*NUM_ROWS*NUM_COLS); // restore video memory
    update_cursor();

    /* launch shell in tid for the first time */    
    if (terminals[tid].running_processes == 0) {
        save_stack(terminals[cur_terminal].pid); // save context of scheduler process
        cur_terminal = tid;                      // switch to display terminal
        video_paging();                          // set up video memory paging
        sti();
        execute((uint8_t*)"shell");
    }
    
    sti();
}

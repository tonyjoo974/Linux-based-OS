/* scheduling.c - Functionality for scheduler
 * vim:ts=4 noexpandtab
 */

#include "scheduling.h"
#include "types.h"
#include "pit.h"
#include "terminals.h"
#include "paging.h"
#include "x86_desc.h"
#include "systemcall.h"
#include "i8259.h"

/* scheduler
 * Round Robin scheduling
 * Inputs: n/a
 * Return Value: n/a
 * Effects: switches the running process to the next terminal's process
 *          changes video and process paging, as well as the stack pointers
 */
void scheduler() {
    uint8_t next_tid;
    int cur_pid;

    cur_pid = terminals[cur_terminal].pid;

    /* return if pit interrupt occurs at first terminal launch before shell has been executed */
    if (cur_pid == -1)
        return; 

    /* find next terminal to process (round-robin cycle) */
    next_tid = (cur_terminal+1) % MAX_TERMINALS;
    while (terminals[next_tid].running_processes == 0 || terminals[next_tid].pid == -1) {
        // skip terminal if no processes running
        next_tid = (next_tid+1) % MAX_TERMINALS;
        if (next_tid == cur_terminal) {
            // return if finished looping around and no other terminal processes running
            return;
        }
    }

    // prevent interrupts during context switch(?)
    cli();  // int flag restored before esp/ebp pointer change in restore_stack

    /* save old pcb stack */
    save_stack(cur_pid);

    /* switch terminal/proccess */
    cur_terminal = next_tid;
    cur_pid = terminals[cur_terminal].pid;
    
    video_paging();             // set up video memory paging
    paging_syscall(cur_pid);    // set up process paging
    
    /* context switch, restore stack */
    restore_stack(cur_pid); 
}

/* save_stack
 * Saves esp/ebp of the process to the corresponding PCB
 * Inputs: pid - process to save
 * Return Value: n/a
 * Effects: updates esp/ebp in the PCB struct
 */
void save_stack(int pid) {
    uint32_t esp, ebp;
    PCB* pcb = (PCB*)(MB_8 - KB_8*(pid + 1));
    // store esp and ebp in the PCB
    asm volatile("movl %%esp, %0":"=g"(esp));
    pcb->esp = esp;
    asm volatile("movl %%ebp, %0":"=g"(ebp));
    pcb->ebp = ebp;
}

/* restore_stack
 * Restores esp/ebp of the process to the corresponding PCB
 * Inputs: pid - process to restore
 * Return Value: n/a
 * Effects: changes the tss pointers and esp/ebp to those stored
 *          in the PCB specified by the input pid
 */
void restore_stack(int pid) {
    uint32_t esp, ebp;
    PCB* pcb = (PCB*)(MB_8 - KB_8*(pid + 1));
    // change tss pointers
    tss.ss0 = KERNEL_DS;                // set ss0 to kernel's stack segment
    tss.esp0 = MB_8 - (KB_8*pid) - 4;   // set esp0 to bottom of process's kernel stack
    // restore esp and ebp from PCB
    esp = pcb->esp;
    ebp = pcb->ebp;
    sti();
    asm volatile("movl %0, %%esp":"=g"(esp));
    asm volatile("movl %0, %%ebp":"=g"(ebp));
}



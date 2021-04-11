/* syscallasm.h - Defines for assembly helpers for system calls
 * vim:ts=4 noexpandtab
 */

#ifndef _SYSCALLASM_H
#define _SYSCALLASM_H

#include "pcb.h"

#ifndef ASM

/* set up the iret stack and jump to user process */
extern int32_t iret_stack(uint32_t *);

/* stack switch back to parent process */
extern void halt_return(int32_t, PCB *);

#endif

#endif

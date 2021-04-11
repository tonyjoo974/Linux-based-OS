/* scheduling.h - Defines for scheduler
 * vim:ts=4 noexpandtab
 */

#ifndef _SCHEDULING_H
#define _SCHEDULING_H

void scheduler();

// context switch helper functions
void save_stack(int pid);
void restore_stack(int pid);

#endif

/* cr.h - Defines for interacting with the CR registers for paging
 * vim:ts=4 noexpandtab
 */

#ifndef _CR_H
#define _CR_H

#ifndef ASM

extern void enable_paging(uint32_t *);
extern void flush_TLB();

#endif

#endif

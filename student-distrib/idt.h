/* idt.h - Defines for interrupt descriptor table
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_H
#define _IDT_H

extern void init_idt();
void set_idt_gate(uint8_t vector, uint32_t handler, uint16_t sel);

#endif

/* isr.h - Defines for interrupt service handlers
 * vim:ts=4 noexpandtab
 */

#ifndef _ISR_H
#define _ISR_H

#ifndef ASM

/* map exceptions to handlers */
extern void exception_0x00();
extern void exception_0x01();
extern void exception_0x02();
extern void exception_0x03();
extern void exception_0x04();
extern void exception_0x05();
extern void exception_0x06();
extern void exception_0x07();
extern void exception_0x08();
extern void exception_0x09();
extern void exception_0x0A();
extern void exception_0x0B();
extern void exception_0x0C();
extern void exception_0x0D();
extern void exception_0x0E();
extern void exception_0x0F();
extern void exception_0x10();
extern void exception_0x11();
extern void exception_0x12();
extern void exception_reserved();

/* map interrupts to handlers */
extern void interrupt_pit();
extern void interrupt_keyboard();
extern void interrupt_rtc();
extern void interrupt_handler();
extern void system_call_handler();

#endif

#endif

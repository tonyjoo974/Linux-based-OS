/* handler.h - Defines for interrupt/exception handlers
 * vim:ts=4 noexpandtab
 */

#ifndef _HANDLER_H
#define _HANDLER_H

// These are all the handlers for interrupts
// For now, these simply print the corresponding error
// and spin indefinitely (exceptions) or continue (other)

void divide_error();
void debug_exc();
void nmi_interrupt();
void breakpoint_exc();
void overflow_exc();
void bound_exceeded();
void invalid_opcode();
void device_unavail();
void double_fault();
void coprocessor_seg();
void invalid_tss();
void segment_not_present();
void stack_fault();
void general_protection();
void page_fault();
void x87_fp_error();
void align_check_exc();
void machine_check_exc();
void simd_fp_exc();
void exception_default();
void interrupt_default();
void system_call();

#endif

/* handler.c - Exception and interrupt handler functions
 * vim:ts=4 noexpandtab
 */

#include "handler.h"
#include "lib.h"
#include "systemcall.h"
#define  EXCEPTION 256

/* exception handler #0
 * Handles the exception for corresponding error (0x00)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void divide_error()
{
    printf("Divide Error Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
    
}

/* exception handler #1
 * Handles the exception for corresponding error (0x01)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void debug_exc()
{
    printf("Debug Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #2
 * Handles the exception for corresponding error  (0x02)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void nmi_interrupt()
{
    printf("Nonmaskable Interrupt\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #3
 * Handles the exception for corresponding error (0x03)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void breakpoint_exc()
{
    printf("Breakpoint Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #4
 * Handles the exception for corresponding error (0x04)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void overflow_exc()
{
    printf("Overflow Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #5
 * Handles the exception for corresponding error (0x05)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void bound_exceeded()
{
    printf("Bound Range Exceeded Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}
/* exception handler #6
 * Handles the exception for corresponding error (0x06)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void invalid_opcode()
{
    printf("Invalid Opcode Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #7
 * Handles the exception for corresponding error (0x07)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void device_unavail()
{
    printf("Device Not Available Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #8
 * Handles the exception for corresponding error (0x08)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void double_fault()
{
    printf("Double Fault Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #9
 * Handles the exception for corresponding error (0x09)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void coprocessor_seg()
{
    printf("Coprocessor Segment Overrun\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #10
 * Handles the exception for corresponding error (0x0A)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void invalid_tss()
{
    printf("Invalid TSS Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #11
 * Handles the exception for corresponding error (0x0B)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void segment_not_present()
{
    printf("Segment Not Present\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #12
 * Handles the exception for corresponding error (0x0C)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void stack_fault()
{
    printf("Stack Fault Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #13
 * Handles the exception for corresponding error (0x0D)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void general_protection()
{
    printf("General Protection Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #14
 * Handles the exception for corresponding error (0x0E)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void page_fault()
{
    printf("Page Fault Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #16
 * Handles the exception for corresponding error (0x0F)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void x87_fp_error()
{
    printf("x87 FPU Floating-Point Error\n");
    halt_extend(EXCEPTION);
    // while(1);
}
/* exception handler #17
 * Handles the exception for corresponding error (0x10)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void align_check_exc()
{
    printf("Alignment Check Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #18
 * Handles the exception for corresponding error (0x11)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void machine_check_exc()
{
    printf("Machine-Check Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #19
 * Handles the exception for corresponding error (0x12)
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void simd_fp_exc()
{
    printf("SIMD Floating-Point Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* exception handler #15, #20-31
 * Handles the exception for other reserved errors
 * Inputs: none
 * Outputs: none
 * Effects: prints error and loops infinitely if it is a valid interrupt
 */
void exception_default()
{
    printf("Non-handled Exception\n");
    halt_extend(EXCEPTION);
    // while(1);
}

/* Default interrupt handler
 * Handles a non-supported interrupt
 * Inputs: none
 * Outputs: none
 * Effects: prints interrupt type and continues execution
 */
void interrupt_default()
{
    printf("Non-handled Interrupt\n");
}

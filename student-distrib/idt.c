/* idt.c - Functionality for interrupt descriptor table (IDT)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "idt.h"
#include "x86_desc.h"
#include "isr.h"
#include "types.h"
#include "handler.h"

/* Kernel Data Segment*/
#define KERNEL_SEG 0x0010
#define PIT_NUMBER      32  // PIT at 0x20
#define KEYBOARD_NUMBER 33  // Keyboard at 0x21
#define RTC_NUMBER      40  // RTC at 0x28
#define SYSCALL_NUMBER  128 // System call at 0x80

/* init_idt
 * Initialize interrupt descriptor table (IDT) 
 * Inputs: none
 * Outputs: none
 * Effect: initializes IDT entries with corresponding handlers
 * See 5.14 of SPG for exception mappings
 */
void init_idt()
{
    int i;

    /* set up exceptions - entries 0 to 31 */
    set_idt_gate(0, (uint32_t)&exception_0x00, KERNEL_SEG);
    set_idt_gate(1, (uint32_t)&exception_0x01, KERNEL_SEG);
    set_idt_gate(2, (uint32_t)&exception_0x02, KERNEL_SEG);
    set_idt_gate(3, (uint32_t)&exception_0x03, KERNEL_SEG);
    set_idt_gate(4, (uint32_t)&exception_0x04, KERNEL_SEG);
    set_idt_gate(5, (uint32_t)&exception_0x05, KERNEL_SEG);
    set_idt_gate(6, (uint32_t)&exception_0x06, KERNEL_SEG);
    set_idt_gate(7, (uint32_t)&exception_0x07, KERNEL_SEG);
    set_idt_gate(8, (uint32_t)&exception_0x08, KERNEL_SEG);
    set_idt_gate(9, (uint32_t)&exception_0x09, KERNEL_SEG);
    set_idt_gate(10, (uint32_t)&exception_0x0A, KERNEL_SEG);
    set_idt_gate(11, (uint32_t)&exception_0x0B, KERNEL_SEG);
    set_idt_gate(12, (uint32_t)&exception_0x0C, KERNEL_SEG);
    set_idt_gate(13, (uint32_t)&exception_0x0D, KERNEL_SEG);
    set_idt_gate(14, (uint32_t)&exception_0x0E, KERNEL_SEG);
    set_idt_gate(15, (uint32_t)&exception_reserved, KERNEL_SEG); // 15 is skipped
    set_idt_gate(16, (uint32_t)&exception_0x0F, KERNEL_SEG);
    set_idt_gate(17, (uint32_t)&exception_0x10, KERNEL_SEG);
    set_idt_gate(18, (uint32_t)&exception_0x11, KERNEL_SEG);
    set_idt_gate(19, (uint32_t)&exception_0x12, KERNEL_SEG);

    /* set up rest of reserved entries to a default exception handler */
    for(i = 20; i < 32; i++) {
    set_idt_gate(i, (uint32_t)&exception_reserved, KERNEL_SEG);
    }

    /* set up interrupts from PIC */
    for(i = 32; i < 48; i++) {
        // pit at IRQ0 (0x20 = 32), keyboard at IRQ1 (0x21 = 33), rtc at IRQ8 (0x28 = 40)
        if (i == PIT_NUMBER)
            set_idt_gate(i, (uint32_t)&interrupt_pit, KERNEL_SEG);
        else if(i == KEYBOARD_NUMBER)
            set_idt_gate(i, (uint32_t)&interrupt_keyboard, KERNEL_SEG);
        else if(i == RTC_NUMBER)
            set_idt_gate(i, (uint32_t)&interrupt_rtc, KERNEL_SEG);
        else
            set_idt_gate(i, (uint32_t)&interrupt_handler, KERNEL_SEG);
    }

    /* set up rest of interrupts to default */
    for(i = 48; i < 256; i++) {
        set_idt_gate(i, (uint32_t)&interrupt_handler, KERNEL_SEG);
    }

    /* set up system call (0x80 = 128) */
    set_idt_gate(SYSCALL_NUMBER, (uint32_t)&system_call_handler, KERNEL_SEG);
}

/* set_idt_gate
 * Sets the idt values to the corresponding vector
 * Inputs: vector - entry in the IDT
 *         handler - handler to execute for this entry
 *         sel - segment in the kernel
 * Outputs: none
 * Effects: modifies entry "vector" in the IDT
 * See 0.1.2 of descriptors reference and 5.11 of SPG
 */
void set_idt_gate(uint8_t vector, uint32_t handler, uint16_t sel)
{
    SET_IDT_ENTRY(idt[vector], handler);
    idt[vector].seg_selector = sel;
    idt[vector].reserved4 = 0;
    /* Use trap gates for system calls (reserved3 = 1), otherwise use interrupt gate */
    idt[vector].reserved3 = (vector == SYSCALL_NUMBER) ? 1 : 0;  
    idt[vector].reserved2 = 1;
    idt[vector].reserved1 = 1;
    idt[vector].size = 1;
    idt[vector].reserved0 = 0;
    /* User level for system calls (dpl = 3), otherwise supervisor level */
    idt[vector].dpl = (vector == SYSCALL_NUMBER) ? 3 : 0;
    idt[vector].present = 1;
}

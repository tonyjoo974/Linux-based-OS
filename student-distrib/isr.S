# isr.S
# Interrupt Service Routines

#define ASM     1

.globl exception_0x00
.globl exception_0x01
.globl exception_0x02
.globl exception_0x03
.globl exception_0x04
.globl exception_0x05
.globl exception_0x06
.globl exception_0x07
.globl exception_0x08
.globl exception_0x09
.globl exception_0x0A
.globl exception_0x0B
.globl exception_0x0C
.globl exception_0x0D
.globl exception_0x0E
.globl exception_0x0F
.globl exception_0x10
.globl exception_0x11
.globl exception_0x12

.globl exception_reserved

.globl interrupt_pit
.globl interrupt_keyboard
.globl interrupt_rtc
.globl interrupt_handler

.globl system_call_handler

.align 4

/* exception_0x##: Handles the reserved exceptions
 * For each exception, we mask interrupts and save all registers
 * before calling the corresponding C function.
 * Since each function loops spins indefinitely,
 * we don't have to really concern ourselves about unmasking/restoring.
 */

exception_0x00:
        cli
        pusha
        call divide_error
        popa
        iret

exception_0x01:
        cli
        pusha
        call debug_exc
        popa
        iret

exception_0x02:
        cli
        pusha
        call nmi_interrupt
        popa
        iret

exception_0x03:
        cli
        pusha
        call breakpoint_exc
        popa
        iret

exception_0x04:
        cli
        pusha
        call overflow_exc
        popa
        iret

exception_0x05:
        cli
        pusha
        call bound_exceeded
        popa
        iret

exception_0x06:
        cli
        pusha
        call invalid_opcode
        popa
        iret

exception_0x07:
        cli
        pusha
        call device_unavail
        popa
        iret

exception_0x08:
        cli
        pusha
        call double_fault
        popa
        iret

exception_0x09:
        cli
        pusha
        call coprocessor_seg
        popa
        iret

exception_0x0A:
        cli
        pusha
        call invalid_tss
        popa
        iret

exception_0x0B:
        cli
        pusha
        call segment_not_present
        popa
        iret

exception_0x0C:
        cli
        pusha
        call stack_fault
        popa
        iret

exception_0x0D:
        cli
        pusha
        call general_protection
        popa
        iret

exception_0x0E:
        cli
        pusha
        call page_fault
        popa
        iret

exception_0x0F:
        cli
        pusha
        call x87_fp_error
        popa
        iret

exception_0x10:
        cli
        pusha
        call align_check_exc
        popa
        iret

exception_0x11:
        cli
        pusha
        call machine_check_exc
        popa
        iret

exception_0x12:
        cli
        pusha
        call simd_fp_exc
        popa
        iret

exception_reserved:
        cli
        pusha
        call exception_default
        popa
        iret

/* interrupt_: Handles the corresponding interrupt
 * For each interrupt, we mask interrupts and save all registers
 * before calling the corresponding C function.
 * We then restore the registers and unmask interrupts before returning.
 */

interrupt_pit:
        cli

        pusha
        pushl %ds
        pushl %es
        pushl %fs
        pushl %gs

        call pit_handler
        popl %gs
        popl %fs
        popl %es
        popl %ds
        popa

        sti
        iret

interrupt_keyboard:
        cli

        pusha
        pushl %ds
        pushl %es
        pushl %fs
        pushl %gs

        call keyboard_handler
        popl %gs
        popl %fs
        popl %es
        popl %ds
        popa

        sti
        iret

interrupt_rtc:
        cli

        pusha
        pushl %ds
        pushl %es
        pushl %fs
        pushl %gs

        call rtc_handler
        popl %gs
        popl %fs
        popl %es
        popl %ds
        popa

        sti
        iret

interrupt_handler:
        cli

        pusha
        pushl %ds
        pushl %es
        pushl %fs
        pushl %gs
        call interrupt_default
        popl %gs
        popl %fs
        popl %es
        popl %ds
        popa

        sti
        iret

system_call_handler:
        pushl %ebp
        movl %esp, %ebp
        # save/protect registers like in common_interrupt (see lecture)
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx      # argument 3
        pushl %ecx      # argument 2
        pushl %ebx      # argument 1

        # call number in [1, 10]
        cmpl $1, %eax
        jl syscall_error
        cmpl $10, %eax
        jg syscall_error

        # get index of system call [0, 9] - offset in jump table
        decl %eax

        # jump to function based on call number in EAX
        # note function pointers are 32 bits = 4 bytes
        call *syscall_jumptable(, %eax, 4)

syscall_end:
        # restore registers and iret
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        leave
        iret

syscall_error:
        # return -1 on error - 24(%esp) stores value of old eax
        movl $-1, %eax
        jmp syscall_end

syscall_jumptable:
        .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

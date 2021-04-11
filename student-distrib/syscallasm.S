# syscallasm.S - Functionality for assembly helpers for system calls

#   0x08000000 = 128 MB = start of program page
# + 0x00400000 = 4 MB = size of program page
# - 0x00000004 = before end of program page, last "valid" address at the bottom
# = PROGRAM_START
#define PROGRAM_START 0x083FFFFC

# from x86_desc.h
#define USER_CS       0x0023
#define USER_DS       0x002B

.globl iret_stack
.globl halt_return

/* iret_stack
 * Sets up the stack for IRET before switching stacks
 * Inputs: EIP/return address
 * Outputs: none
 * Side effects: switches stack to child process (user-space)
 * See lecture 19, MP3 6.3.4 - stack bottom to top: SS ESP EFLAGS CS RETURN/EIP
 * https://stackoverflow.com/questions/6892421/switching-to-user-mode-using-iret
 * https://www.felixcloutier.com/x86/iret:iretd
 */
iret_stack:
    pushl 	%ebp
    movl 	%esp, %ebp

    # change ds register (data segment) to user level BEFORE iret
    # iret doesn't change any data segments, have to change manually
    movl 	$USER_DS, %eax
    movw 	%ax, %ds

    # SS - stack/data segment for user in GDT
    movl 	$USER_DS, %eax
    pushl   %eax

    # ESP - point to bottom of program at [128MB, 132MB) page
    movl 	$PROGRAM_START, %eax
    pushl 	%eax

    # EFLAGS - push flags and enable interrupts (bit 9)
    pushfl
    popl 	%eax
    orl 	$0x00000200, %eax   
    pushl 	%eax

    # CS - code segment for user in GDT
    movl    $USER_CS, %eax
    pushl 	%eax

    # EIP - instruction to start at on iret
    movl 	8(%ebp), %eax
    pushl   %eax

    iret

/* halt_return
 * Switches stack back to execute call
 * Inputs: first arg - status to return from halt
 *         second arg - pointer to current PCB
 * Outputs: none
 * Side effects: returns back to execute
 */
halt_return:
    pushl   %ebp
    movl    %esp, %ebp
	cli
	xorl 	%ebx, %ebx
	movl 	8(%ebp), %ebx  # status
    movl    12(%ebp), %eax # PCB pointer
	
    # extract arguments from PCB pointer
    movl    (%eax), %esp   # esp first (restore old ESP)
	movl    4(%eax), %ebp  # ebp second (restore old EBP)

    # set EAX to return value (halt status code)
	movl 	%ebx, %eax
	sti
    # jump back to execute
	leave
    ret

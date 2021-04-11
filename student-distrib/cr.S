# cr.S - Functionality to modify CR registers to enable paging

.globl enable_paging
.globl flush_TLB

/* enable_paging
 * Enables paging by setting CR registers
 * Inputs: address of paging directory
 * Outputs: none
 * Side effects: changes CR0, CR3, CR4
 * See https://wiki.osdev.org/Setting_Up_Paging and sections 2.5 and 3.6.1 of SPG
 */
enable_paging:
    pushl %ebp
    movl %esp, %ebp
    pushl %eax

    # put address in cr3
    movl 8(%ebp), %eax
    movl %eax, %cr3

    # set bit 4 of cr4 to enable larger pages (4 MB page for kernel)
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4

    # set bit 31 and bit 0 of cr0 to enable paging in protected mode
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0

    popl %eax
    leave
    ret

/* flush_TLB
 * Flushes TLB by reloading CR3
 * Inputs: none
 * Outputs: none
 * Side effects: none
 */
flush_TLB:
    pushl %ebp
    movl %esp, %ebp
    pushl %eax

    # reload cr3
    movl %cr3, %eax
    movl %eax, %cr3

    popl %eax
    leave
    ret

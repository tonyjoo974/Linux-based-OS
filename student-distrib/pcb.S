# pcb.S - Functionality to get PCB pointer

.globl get_PCB

/* get_PCB
 * Gets pointer to PCB
 * Inputs: none
 * Outputs: pointer to PCB
 * Side effects: none
 * Note that the PCB is located at the top of the process's 8 kB kernel stack
 */
get_PCB:
  pushl %ebp
  movl %esp, %ebp

  # AND %esp to get top of 8kB stack
  # 8kB = 2^13 (align 13 least significant bits)
  movl %esp, %eax
  andl $0xFFFFE000, %eax

  leave
  ret

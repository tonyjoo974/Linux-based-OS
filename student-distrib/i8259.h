/* i8259.h - Defines used in interactions with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20    // master PIC - command
#define SLAVE_8259_PORT     0xA0    // slave PIC - command
#define MASTER_8259_DATA    0x21    // master PIC - data
#define SLAVE_8259_DATA     0xA1    // slave PIC - data

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* initial master/slave masks */
#define MASTER_MASK_INIT    0xFB   // 1111 1011 (slave at IRQ2)
#define SLAVE_MASK_INIT     0xFF   // 1111 1111

/* master slave constants */
#define MASTER_SLAVE_OFFSET 8
#define SLAVE_IRQ_NUM       2 // slave at IRQ2

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */

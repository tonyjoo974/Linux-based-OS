/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

// reference: https://wiki.osdev.org/8259_PIC, lecture 9 Linux initialization code

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* 
 * i8259_init(void)
 * Description: initializes the 8259 PIC by setting 4 initialization 
 *              control words (ICWs) to the master and slave
 * Inputs:  n/a
 * Outputs: n/a
 * Side-effects: sends 4 ICWs to the master/slave PIC ports
 */
void i8259_init(void) {
    // ICW1 - start initialization, 4 ICWs
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    // ICW2 - high bits (master mapped to 0x20-0x27, slave mapped to 0x28-0x2F)
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);

    // ICW3 - master: bit vector of slave, slave: input pin IRQ2 on master
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);

    // ICW4 - x86 ISA, normal EOI
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    // "restore saved masks"
    // initialize master/slave PIC - interrupts active low
    master_mask = MASTER_MASK_INIT; // all disabled except for slave IR2
    slave_mask = SLAVE_MASK_INIT; // all disabled
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
}

/*
 * enable_irq
 * Description: Enable (unmask/set to 0) the specified IRQ
 * Input: irq_num - interrupt number
 * Output: n/a
 * Side-effects: unmasks specified irq_num in master or slave PIC and sends data
 *               to respective ports (changes global var master/slave mask)
 */
void enable_irq(uint32_t irq_num) {
    int bitmask = 1;
    uint8_t irq_mask;

    // master IR# 0-7
    if ((irq_num >= 0) && (irq_num < MASTER_SLAVE_OFFSET)) {
        irq_mask = ~(bitmask << irq_num);   // sets all but IR# pin to 1
        master_mask = master_mask & irq_mask;
        outb(master_mask, MASTER_8259_DATA);
    }
    
    // slave IR# 8-15
    else if ((irq_num >= MASTER_SLAVE_OFFSET) && (irq_num < MASTER_SLAVE_OFFSET*2)) {
        irq_mask = ~(bitmask << (irq_num-MASTER_SLAVE_OFFSET));    // sets all but IR# pin to 1
        slave_mask = slave_mask & irq_mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/*
 * disable_irq
 * Description: Disable (mask/set to 1) the specified IRQ
 * Input: irq_num - interrupt number
 * Output: n/a
 * Side-effects: masks specified irq_num in master or slave PIC and sends data
 *               to respective ports (changes global var master/slave mask)
 */
void disable_irq(uint32_t irq_num) {
    int bitmask = 1;
    uint8_t irq_mask;

    // master IR# 0-7
    if ((irq_num >= 0) && (irq_num < MASTER_SLAVE_OFFSET)) {
        irq_mask = bitmask << irq_num;   // sets IR# pin to 1
        master_mask = master_mask | irq_mask;
        outb(master_mask, MASTER_8259_DATA);
    }

    // slave IR# 8-15
    else if ((irq_num >= MASTER_SLAVE_OFFSET) && (irq_num < MASTER_SLAVE_OFFSET*2)) {
        irq_mask = bitmask << (irq_num-MASTER_SLAVE_OFFSET);    // sets IR# pin to 1
        slave_mask = slave_mask | irq_mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* 
 * send_eoi
 * Description: Send end-of-interrupt signal for the specified IRQ
 *              (declare interrupt finished by OR-ing EOI with irq_num)
 * Input: irq_num - interrupt number
 * Output: n/a
 * Side-effects: sends EOI with IR number to PIC
 */
void send_eoi(uint32_t irq_num) {
    // master IR# 0-7
    if ((irq_num >= 0) && (irq_num < MASTER_SLAVE_OFFSET)) {
        outb(EOI|irq_num, MASTER_8259_PORT); // send EOI at interrupt to master
    }

    // slave IR# 8-15
    else if ((irq_num >= MASTER_SLAVE_OFFSET) && (irq_num < MASTER_SLAVE_OFFSET*2)) {
        outb(EOI|SLAVE_IRQ_NUM, MASTER_8259_PORT); // send EOI at slave IRQ to master
        outb(EOI|(irq_num-MASTER_SLAVE_OFFSET), SLAVE_8259_PORT); // send EOI at interrupt to slave
    }
}

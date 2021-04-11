/* pit.c - Functionality for PIT
 * vim:ts=4 noexpandtab
 */

#include "pit.h"
#include "types.h"
#include "lib.h"
#include "scheduling.h"
#include "i8259.h"

/* pit_init
 * Initializes programmable interval timer (PIT)
 * Inputs: n/a
 * Return Value: n/a
 * Effects: sets interval frequency and enables interrupt requests
 */
void pit_init() {
    // set frequency mode and value
    outb(MODE, MODE_REG);                   // initialize mode/cmd register to MODE
    outb(RELOAD_VAL & BYTE_MASK, CHANNEL0); // set low byte of reload value
    outb(RELOAD_VAL >> 8, CHANNEL0);        // right shift to set high byte of reload value
    // enable irq
    enable_irq(PIT_IRQ);
}

/* pit_handler
 * Handles interrupt requests and calls scheduler
 * Inputs: n/a
 * Return Value: n/a
 * Effects: see scheduler
 */
void pit_handler() {
    send_eoi(PIT_IRQ);
    scheduler();
}

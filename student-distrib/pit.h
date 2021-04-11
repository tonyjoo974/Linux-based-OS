/* pit.h - Defines for PIT
 * vim:ts=4 noexpandtab
 */

#ifndef _PIT_H
#define _PIT_H

#include "types.h"

/* PIT ports and constants 
 * see https://wiki.osdev.org/Programmable_Interval_Timer 
 *
 * MODE[7:6] = 00 for channel 0
 * MODE[5:4] = 11 for lobyte/hibyte to send reload_val (transfer as pair)
 * MODE[3:1] = 011 for square wave generator
 * MODE[0] = 0 for 16-bit binary
 * 
 * frequency = 1193182 / reload_value [Hz] = 50 Hz = tick every 20 ms
 */
#define PIT_IRQ     0
#define CHANNEL0    0x40   // channel 0 allows timer ticks
#define MODE_REG    0x43   // port of mode/command register
#define MODE        0x36   // square wave generator
#define BYTE_MASK   0xFF
#define FREQ        1193182 // frequency base
#define RELOAD_VAL  23864

void pit_init();
void pit_handler();

#endif

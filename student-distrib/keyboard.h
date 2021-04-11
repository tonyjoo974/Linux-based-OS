/* keyboard.h - Defines for keyboard
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

#define LIMIT       58
#define SPACE       57
#define LOWER       0
#define UPPER       1
#define DATA_PORT   0x60
#define IRQ1        0x01
#define LSHIFT_ON   0x2A
#define LSHIFT_OFF  0xAA
#define RSHIFT_ON   0x36
#define RSHIFT_OFF  0xB6
#define CAPSLOCK    0x3A   
#define L_CTRL_ON   0x1D
#define L_CTRL_OFF  0x9D
#define ALT_ON	    0x38
#define ALT_OFF		0xB8
#define ENTER_ON    0x1C
#define ENTER_OFF   0x9C
#define BACKSPACE	0x0E
#define TAB         0x0F
#define F1_ON       0x3B
#define F2_ON       0x3C
#define F3_ON       0x3D
#define F1_OFF      0xBB
#define F2_OFF      0xBC
#define F3_OFF      0xBD
#define Q           16
#define P           25
#define A           30
#define L           38
#define Z           44
#define M           50

#define CHAR1       2
#define CHAR2       13
#define CHAR3       16
#define CHAR4       53
#define LETTER_L    38
#define LETTER_C    46

/* keyboard interrupt handler*/ 
extern void keyboard_handler();
/* keyboard initialization */ 
extern void keyboard_init();

/* puts keypress value to keyboard buffer */ 
int add_key_buffer(uint8_t keyval);
/* clears keyboard buffer */
void clear_key_buffer(void);
/* update global variable keys and handle backspace */ 
void update_key_status(uint8_t scancode);
/* keyboard helper functions */
uint8_t is_letter(uint8_t scancode);
uint8_t is_char(uint8_t scancode);

/* terminal system call functions */ 
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
/* reads/stores keyboard buffer to input buffer */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
/* writes/prints buffer contents */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif

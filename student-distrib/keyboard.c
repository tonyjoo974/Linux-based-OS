/* keyboard.c - Functionality for keyboard/keypresses
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "terminals.h"

// reference: https://wiki.osdev.org/PS/2_Keyboard, Appendix B of MP3 for open/read/write/close behavior

/* Global variables */
// 1 means that respective key is pressed
static volatile uint8_t shift_status = 0;
static volatile uint8_t caps_status = 0;
static volatile uint8_t ctrl_status = 0;
static volatile uint8_t alt_status = 0;
static volatile uint8_t enter_status = 0;
static volatile uint8_t fn_status = 0;

/* keyboard scancodes excluding numpad, arrow key, home/end section */
char kbd_scan[2][58]={
    // LOWER
    {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 
     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 
     'z', 'x', 'c', 'v', 'b', 'n', 'm',',', '.', '/', 0, 0, 0, ' '},
    // UPPER
    {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 
     'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 
     'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '}
};

/* keyboard_handler
 * Handles the keyboard interrupt
 * Inputs: keycode from DATA_PORT of keyboard
 * Outputs: none
 * Effects: prints out the printed key
 */
void keyboard_handler()
{
    /* Read the scancode from the keyboard's data port */
	uint8_t scancode = inb(DATA_PORT);
    update_key_status(scancode);
    uint8_t keyval;
    int is_full = 0;
    int i;

    // change terminal on ALT-F#
    if (alt_status == 1 && fn_status != 0) {
        send_eoi(IRQ1);
        terminal_switch(fn_status-1);
    }

    // clear screen on CTRL+L
    if(ctrl_status == 1 && scancode == LETTER_L) {
        clear();
    }

    if(scancode < LIMIT) { // valid keycode
        if(scancode == TAB) { // print 4 spaces for tab
            keyval = kbd_scan[UPPER][SPACE];
            for(i = 0; i < 4; i++) {
                add_key_buffer(keyval);
                putc_keyboard(keyval);
            }
        }
        else if(is_char(scancode) == 1) { // printable character
            // caps off, shift on
            if(!caps_status && shift_status) keyval = kbd_scan[UPPER][scancode];
            // caps off, shift off
            else if(!caps_status && !shift_status) keyval = kbd_scan[LOWER][scancode];
            // caps on, shift off
            else if(caps_status && !shift_status) {
                if(is_letter(scancode)) keyval = kbd_scan[UPPER][scancode]; // letters uppercase
                else keyval = kbd_scan[LOWER][scancode];
            }
            // caps on, shift on
            else if(caps_status && shift_status) {
                if(!is_letter(scancode)) keyval = kbd_scan[UPPER][scancode];
                else keyval = kbd_scan[LOWER][scancode]; // letters lowercase
            }

            // don't do anything on CTRL+L
            if(!(ctrl_status == 1 && scancode == LETTER_L)) {
                is_full = add_key_buffer(keyval); // add to buffer
            }

            if(!(ctrl_status == 1 && scancode == LETTER_L) && is_full != -1 && keyval != 0) { 
                putc_keyboard(keyval); // print if valid key and added to buffer successfully
            }
        }
    }

    send_eoi(IRQ1);
}

/* is_char
 * Determines if the scancode is a printable character
 * Inputs: keycode from DATA_PORT of keyboard
 * Outputs: 1 for true, 0 for false
 * Effects: none
 */
uint8_t is_char(uint8_t scancode) {
    if(scancode == LSHIFT_ON || scancode == L_CTRL_ON) {
        return 0;
    }
    else if((scancode >= CHAR1 && scancode <= CHAR2) || (scancode >= CHAR3 && scancode <= CHAR4)
       || (scancode == SPACE)) {
        return 1;
    }
    return 0;
}

/* is_letter
 * Determines if the scancode is an alphabet letter
 * Inputs: keycode from DATA_PORT of keyboard
 * Outputs: 1 for true, 0 for fail
 * Effects: none
 */
uint8_t is_letter(uint8_t scancode) {
    if((scancode >= Q && scancode <= P) || (scancode >= A && scancode <= L) || 
      (scancode >= Z && scancode <= M) || (scancode == SPACE)) {
        return 1;
    }
    return 0;
}

/* update_key_status
 * Handles keys with special functionality
 * 1. Update the global variable keys if pressed
 * 2. Handle BACKSPACE functionality
 * Inputs: keycode from DATA_PORT of keyboard
 * Outputs: none
 * Effects: updates the value of the global variables
 * Recall that there are different scan codes for press and release
 */
void update_key_status(uint8_t scancode) {
    switch(scancode)
        {
            case CAPSLOCK:
                    caps_status = (caps_status) ? 0 : 1;
                    break;
            case RSHIFT_ON:
            case LSHIFT_ON:
                    shift_status = 1;
                    break;
            case RSHIFT_OFF:
            case LSHIFT_OFF:
                    shift_status = 0;
                    break;
            case L_CTRL_ON:
                    ctrl_status = 1;
                    break;
            case L_CTRL_OFF:
                    ctrl_status = 0;
                    break;
            case ALT_ON:
                    alt_status = 1;
                    break;
            case ALT_OFF:
                    alt_status = 0;
                    break;
            case F1_ON:
                    fn_status = 1;
                    break;
            case F2_ON:
                    fn_status = 2;
                    break;
            case F3_ON:
                    fn_status = 3;
                    break;
            case F1_OFF:
                    fn_status = 0;
                    break;
            case F2_OFF:
                    fn_status = 0;
                    break;
            case F3_OFF:
                    fn_status = 0;
                    break;
            case ENTER_ON:
                    enter_status = 1;
                    break;
            case ENTER_OFF:
                    enter_status = 0;
                    break;
            case BACKSPACE:
                	backspace();
                    break;
            default:
                    break;
        }
}

/* keyboard_init
 * Initializes keyboard by enabling IRQ1
 * Inputs: none
 * Outputs: none
 * Effects: enables IRQ1
 */
void keyboard_init()
{
    enable_irq(IRQ1);   // keyboard is IRQ1
    buffer_idx = 0;     // reset buffer
}

/* terminal_open 
 * Inputs: filename - n/a
 * Outputs: 0 on success
 */
int32_t terminal_open(const uint8_t* filename) {
    return 0;
}

/* terminal_read
 * Return data from one line that has been terminated by pressing Enter
 * or as much as it fits in the buffer from one such line
 * Inputs: fd - file descriptor, n/a
 *         buf - buffer to store keyboard buffer
 *         nbytes - number of bytes to read
 * Output: number of bytes read
 * Effects: clears kbd_buffer
 */ 
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    uint32_t i, j;
    uint32_t found_newline = 0;
    if (buf == NULL || nbytes < 0 || nbytes > BUF_SIZE) return -1;

    // spin while enter is not pressed or not on display on terminal
    while((!enter_status)||(display_terminal!=cur_terminal));
    enter_status = 0;
    
    // copy buffer
    for(i = 0; (i < BUF_SIZE-1) && (i < nbytes); i++) {
        ((int8_t*)buf)[i] =  saved_kbd_buffer[i];
        if(saved_kbd_buffer[i] == '\n') {
            i++;
            found_newline = 1;
            break;
        }
    }
    // put \n at end if typed message is at max length
    if (i == (BUF_SIZE-1) && !found_newline) {
        ((int8_t*)buf)[i] = '\n';
        i++;
    }
    
    // copy to terminal buffer and clear saved_kbd_buffer
    for(j = 0; j < BUF_SIZE; j++) {
        // terminals[display_terminal].kbd_buffer[j] = saved_kbd_buffer[j];
        saved_kbd_buffer[j] = NULL;
    }
    
    return i;
}

/* terminal_write 
 * display characters in buffer to terminal screen
 * Inputs: fd - file descriptor, n/a
 *         buf - buffer with characters to write
 *         nbytes - number of bytes to write
 * Output: number of bytes written
 * Effects: prints buffer characters to terminal
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    uint32_t i;
    if (buf == NULL || nbytes < 0 || nbytes > BUF_SIZE) return -1;

    // print char in buffer to screen
    for(i = 0; i < nbytes; i++) {
        putc(((int8_t*)buf)[i]);
    }

    return nbytes;
}

/* terminal_close 
 * Inputs: fd - file descriptor, n/a
 * Output: 0 on success
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}

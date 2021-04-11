/* rtc.c - Functionality for RTC
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"

// reference: https://wiki.osdev.org/RTC, Appendix B of MP3 for open/read/write/close behavior

/* 
 * rtc_handler 
 * Description: handles rtc interrupt by turning the periodic interrupt on
 * Inputs: n/a
 * Outputs: n/a
 * Side-effects: sets bit 6 of reg B
 */ 
void rtc_handler()
{
    send_eoi(RTC_IRQ_NUM); // interrupt acknowledged

    int i;
    for(i = 0; i < MAX_TERMINALS; i++) {
        rtc_int_flag[i] = 1;
    }
    // test_interrupts();

    // if reg C not read, interrupt will not happen again
    outb(RTC_REG_C, NMI_PORT); // select reg C and disable NMI
    inb(CMOS_PORT);            // throw away contents
    return;
}

/* 
 * rtc initialization 
 * Description: initializes rtc and sets initial frequency
 * Inputs: n/a
 * Outputs: n/a
 * Side-effects:  
 */ 
void rtc_init()
{
    /* turn on periodic interrupt */
    outb(RTC_REG_B, NMI_PORT);           // select reg B and disable NMI
    unsigned char prev = inb(CMOS_PORT); // read current value of reg B
    outb(RTC_REG_B, NMI_PORT);           // select reg B
    outb(prev | REG_B_MASK, CMOS_PORT);  // write prev | 0x40 to turn on bit 6 of register B

    enable_irq(RTC_IRQ_NUM);             // RTC is IRQ8

    // set initial freq to 2
    rtc_set_freq(F2);

    int i = 0;
    for(i = 0; i < MAX_TERMINALS; i++) {
        rtc_int_flag[i] = 0;
    }

    return;
}


/* 
 * rtc_set_freq
 * Description: set rtc frequency 
 * Inputs: freq - frequency to set to
 * Outputs: 0 on success, -1 otherwise
 * Side-effects: sends frequency to rtc specified port if valid
 */
int32_t rtc_set_freq(int32_t freq) {
    uint32_t freq_reg;

    // frequency =  32768 >> (rate-1);
    // rate is > 2 and <= 15
    switch(freq) {
        case F2: freq_reg = RS2; break;
        case F4: freq_reg = RS4; break;
        case F8: freq_reg = RS8; break;
        case F16: freq_reg = RS16; break;
        case F32: freq_reg = RS32; break;
        case F64: freq_reg = RS64; break;
        case F128: freq_reg = RS128; break;
        case F256: freq_reg = RS256; break;
        case F512: freq_reg = RS512; break;
        case F1024: freq_reg = RS1024; break;
        default: return -1;
    }

    outb(RTC_REG_A, NMI_PORT);       // set index to register A, disable NMI
    char prev = inb(CMOS_PORT);      // get init value of reg A
    outb(RTC_REG_A, NMI_PORT);       // reset index to A
    outb((prev & REG_A_MASK) | freq_reg, CMOS_PORT);  // write rate to reg A (rate is bottom 4 bits)
    // prev & 0xF0 = zero out lower 4 bits, keep higher 4 bits
    
    return 0;
}

/* 
 * rtc_open
 * Description: resets rtc frequency 
 * Inputs: filename - n/a
 * Outputs: 0 on success
 * Side-effects: sets rtc frequency to 2 Hz
 */
int32_t rtc_open(const uint8_t* filename) {
    return 0;
}

/* 
 * rtc_read
 * Description: reads interrupt and resets interrupt flag 
 * Inputs: fd - file descriptor, n/a
 *         buf - n/a
 *         nbytes - n/a
 * Outputs: 0 on success, -1 otherwise
 * Side-effects: sends frequency to rtc specified port if valid
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    rtc_int_flag[cur_terminal] = 0;
    while (!rtc_int_flag[cur_terminal]); // wait for rtc interrupt
    rtc_int_flag[cur_terminal] = 0;
    return 0;
}

/* 
 * rtc_write
 * Description: write data (freq) to rtc device 
 * Inputs: fd - file descriptor, n/a
 *         buf - pointer to frequency
 *         nbytes - number of bytes to write
 * Outputs: 0 on success, -1 otherwise
 * Side-effects: sets rtc frequency if valid input
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    // check if inputs (buf/nbytes) are valid
    // nbytes should be 4 to write 4 at a time
    if ((int32_t*)buf == NULL || nbytes != 4) return -1;

    // set frequency
    int32_t freq;
    freq = *((int32_t*)buf);
    return rtc_set_freq(freq);
}

/* 
 * rtc_close
 * Description: close system call
 * Inputs: fd - file descriptor, n/a
 * Outputs: 0 on success
 */
int32_t rtc_close(int32_t fd) {
    return 0;
}

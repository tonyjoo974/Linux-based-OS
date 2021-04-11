/* rtc.h - Defines for real-time clock
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "terminals.h"

/* rtc IRQ number */ 
#define RTC_IRQ_NUM 8

/* port and register numbers */ 
#define NMI_PORT    0x70 // writing to this port selects a register and disables NMI
#define CMOS_PORT   0x71 // selecting register, this port contains data of register
#define RTC_REG_A   0x8A
#define RTC_REG_B   0x8B
#define RTC_REG_C   0x0C
#define RTC_IRQ_NUM 8

/* masks */
#define REG_B_MASK  0x40
#define REG_A_MASK  0xF0

/* frequencies and their register values */
// F# - frequency in Hz
#define F2          2
#define F4          4
#define F8          8
#define F16         16
#define F32         32
#define F64         64
#define F128        128
#define F256        256
#define F512        512
#define F1024       1024
// RS# - rate/register value of respective frequency
#define RS2          15
#define RS4          14
#define RS8          13
#define RS16         12
#define RS32         11
#define RS64         10
#define RS128        9
#define RS256        8
#define RS512        7
#define RS1024       6

/* flag to check next interrupt */ 
// volatile int32_t rtc_int_flag; // set to 1 when RTC interrupt occurs
volatile int32_t rtc_int_flag[MAX_TERMINALS];

/* rtc initialization */ 
void rtc_init(void);
/* set rtc frequency */
int32_t rtc_set_freq(int32_t freq);

/* system call functions */
/* reset frequency */ 
int32_t rtc_open(const uint8_t* filename);
/* read interrupt & reset flag */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
/* set frequency given valid input */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
/* close system call (always returns 0/success) */
int32_t rtc_close(int32_t fd);

#endif

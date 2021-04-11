/* paging.h - Defines for paging, along with "global" PD/PT
 * vim:ts=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

/* constants */
#define PAGE_LEN    1024        // 1kb
#define TABLE_LEN   1024        // 1kb
#define KB_1        1024
#define PAGE_SIZE   4096        // 4kb
#define KB_4        4096
#define PAGE_P           1    // present                  0 0000 0001
#define PAGE_RW          2    // read-write               0 0000 0010
#define PAGE_US          4    // user-supervisor          0 0000 0100
#define PAGE_PWT         8    // page write-through       0 0000 1000
#define PAGE_PCD         16   // page cache disabled      0 0001 0000
#define PAGE_A           32   // accessed                 0 0010 0000
#define PAGE_D           64   // dirty                    0 0100 0000
#define PAGE_PS          128  // page size                0 1000 0000
#define PAGE_G           256  // global                   1 0000 0000
#define KERNEL_ADDR 0x00400000  // kernel memory address
#define VIDEO_ADDR  0x000B8000  // video memory address
#define VIDEO_LOCATION  184     // (B8000=753664)/4096 = 0xB8
#define USER_PAGE		32		// user program is at 128MB (128/4=32)
#define VIDEO_PAGE      33      // video page is at 132MB (right after user page)
#define MB_8		0x00800000
#define MB_4		0x00400000
#define KB_8		0x2000
#define MB_128      0x8000000
#define MB_132      0x8400000
#define MB_136      0x8800000

/* directory and table structs */
// may have to add additional structs over time
// abstract away details and only focus on KB

typedef struct directory_t {
    // array of tables
    uint32_t tables[TABLE_LEN] __attribute__((aligned(KB_4)));
} directory;

typedef struct table_t {
    // array of pages
    uint32_t pages[PAGE_LEN] __attribute__((aligned(KB_4)));
} table;

directory page_directory;
table video_table;
table user_table;

/* paging initialization */
extern void paging_init();

/* virtual memory mapping for syscall execute */
void paging_syscall(int32_t pid);

/* paging setup for virtual memory */
void video_paging();

#endif

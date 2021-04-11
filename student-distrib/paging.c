/* paging.c - Functionality for paging
 * vim:ts=4 noexpandtab
 */

#include "paging.h"
#include "lib.h"
#include "x86_desc.h"
#include "cr.h"
#include "terminals.h"

// reference: Appendix C of MP3

/* paging_init
 * Initializes and enables paging
 * Inputs: none
 * Outputs: none
 * Effects: Initializes video and kernel pages. Sets CR registers.
 * See descriptor reference and 3.7.6 of SPG for meanings of bits and rationale
 */
void paging_init() {
    int i;

    /* VIDEO MEMORY - FIRST 4 MB */
    /* VIDEO IS A SINGLE 4KB PAGE */
    // break up first 4 MB of memory into 4 KB pages
    for(i = 0; i < PAGE_LEN; i++) {
        video_table.pages[i] = KB_4 * i;  // address of pages (all unpresent)
    }

    // page base corresponds to physical memory
    for (i = 0; i < MAX_TERMINALS+1; i++) {
        video_table.pages[VIDEO_LOCATION+i] = VIDEO_ADDR + KB_4*i; // address
        video_table.pages[VIDEO_LOCATION+i] |= PAGE_P;    // valid page
        video_table.pages[VIDEO_LOCATION+i] |= PAGE_RW;   // read-write
        video_table.pages[VIDEO_LOCATION+i] |= PAGE_PCD;  // disable cache
    }
    // p = 1; // valid page
    // rw = 1; // read-write
    // us = 0; // supervisor only
    // pwt = 0; // "always want write back, should always be 0"
    // pcd = 1; // cache disabled - "should be 0 for video memory"
    // a = 0; // unused
    // d = 0; // unused
    // ps = 0; // page size is 4KB
    // g = 0; // ignored - "only set for kernel"

    // first 4 MB is entry 0
    page_directory.tables[0] = (uint32_t)video_table.pages; // page table address
    page_directory.tables[0] |= PAGE_P;
    page_directory.tables[0] |= PAGE_RW;
    page_directory.tables[0] |= PAGE_PCD; // disable cache

    /* KERNEL - 4MB to 8MB */
    // next 4MB entry is entry 1 (kernel)
    // note: us is 0 (privileged/supervisor only), pcd is 0 (should cache)
    page_directory.tables[1] = KERNEL_ADDR; // address directly to 4MB kernel page
    page_directory.tables[1] |= PAGE_P;
    page_directory.tables[1] |= PAGE_RW;
    page_directory.tables[1] |= PAGE_PS; // 4MB page size
    page_directory.tables[1] |= PAGE_G; // global for kernel

    /* NOT PRESENT - 8MB TO 4GB */
    for(i = 2; i < TABLE_LEN; i++) {
        page_directory.tables[i] = 0x00000000; // mark as not present/unused 
    }
    
    /* ENABLE PAGING REGISTERS */
    enable_paging(page_directory.tables);
}

/* paging_syscall
* Memory mapping for user level programs
* Inputs: pid - process id
* Outputs: none
* Effects: flushes TLB
* See descriptor reference and 3.7.6 of SPG for meanings of bits and rationale
* 128 MB virtual address mapped to 8 MB or 12 MB physical address
*/
void paging_syscall(int32_t pid) {
	/* allocate a 4MB user page at table index 32 (from 128 MB virtual address) */
	page_directory.tables[USER_PAGE] = (MB_8 + pid*MB_4);
	page_directory.tables[USER_PAGE] |= PAGE_P;
	page_directory.tables[USER_PAGE] |= PAGE_RW;
	page_directory.tables[USER_PAGE] |= PAGE_US;
	page_directory.tables[USER_PAGE] |= PAGE_PS;
	
	/* always flush TLB after changing paging mappings */
	flush_TLB();
}

/* video_paging
* Maps video memory into user space
* Inputs: none
* Outputs: none
* Effects: flushes TLB
* See descriptor reference and 3.7.6 of SPG for meanings of bits and rationale
*/
void video_paging() {
    /* page base corresponds video memory address */
    if (cur_terminal == display_terminal) {
        // running process is displayed, write to actual video memory
        user_table.pages[0] = VIDEO_ADDR;
    }
	else {
        // running process is not displayed, write to video buffer page instead
        user_table.pages[0] = VIDEO_ADDR + KB_4*(cur_terminal+1);
    }
	user_table.pages[0] |= PAGE_P;
	user_table.pages[0] |= PAGE_RW;
	user_table.pages[0] |= PAGE_US;

	/* allocate a 4 KB user page at table index 33 (from 132 MB virtual address) */
	page_directory.tables[VIDEO_PAGE] = (uint32_t)user_table.pages;
	page_directory.tables[VIDEO_PAGE] |= PAGE_P;
	page_directory.tables[VIDEO_PAGE] |= PAGE_RW;
	page_directory.tables[VIDEO_PAGE] |= PAGE_US;

    /* always flush TLB after changing paging mappings */
	flush_TLB();
}

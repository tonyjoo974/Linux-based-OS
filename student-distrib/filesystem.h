/* x86_desc.h - Defines for file system
 * vim:ts=4 noexpandtab
 */
#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"

/* constants - given in Appendix A 8.1 */
#define BLOCK_SIZE						4096	// block size is 4KB

/* boot block */
#define BOOT_BLOCK_RESERVED_BYTES		52 
#define NUM_DIR_ENTRIES					63		// 63 directory entries (first is reserved)
#define TOTAL_DIR_ENTRIES_SIZE_BYTES	4032	// NUM_DIR_ENTRIES * (32+4+4+24 = 64)
#define DENTRY_RESERVED_BYTES			24		// for directory entries in boot block
/* inodes */
#define MAX_FILENAME_SIZE				32		// 32 characters = 32 bytes
#define MAX_NUM_DATA_BLOCKS				1023	// 1024 - 1 = # 4B blocks in 4kB block - first length block

/* file system data structures from lecture 16 */
/* see Appendex A 8.1 for more details */
typedef struct dentry { // 64 bytes total
	int8_t filename[MAX_FILENAME_SIZE]; 	// 32 bytes for name
	uint32_t filetype;	// 0 for access to RTC, 1 for directory, 2 for regular file
	uint32_t inode_num; // index node number
	uint8_t reserved[DENTRY_RESERVED_BYTES]; // 24 bytes reserved
} dentry_t;

typedef struct boot_block {	// 4 kB block
	uint32_t dir_count; 	// number of entries in directory
	uint32_t inode_count; 	// number of inodes (N)
	uint32_t data_count; 	// number of data blocks (D)
	uint8_t reserved[BOOT_BLOCK_RESERVED_BYTES];	// 52 bytes reserved
	dentry_t direntries[NUM_DIR_ENTRIES]; 			// 64 byte directory entries
} boot_block_t;

typedef struct inode {		// 4 kB block
	uint32_t length;	// first entry is the length in bytes
	uint32_t data_block_num[MAX_NUM_DATA_BLOCKS]; // rest are data blocks
} inode_t;

/* file system initialization */
void filesystem_init(uint32_t fs_addr);

/* opens file to read */
int32_t file_open(const uint8_t* filename);
/* closes opened file */
int32_t file_close(int32_t fd);
/* writes to an opened file (doesn't do anything) */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
/* reads an opened file */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* opens directory to read */
int32_t directory_open(const uint8_t* filename);
/* closes opened directory */
int32_t directory_close(int32_t fd);
/* writes to an opened directory (doesn't do anything) */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
/* reads a filename in an opened directory (in order, one file per call) */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

/* get the file size of a file */
int32_t get_filesize(const uint8_t* filename);
/* get the file size of a file */
int32_t get_filetype(const uint8_t* filename);
/* check if file is an executable */
int32_t is_executable(const uint8_t* filename);
/* copy the file to memory starting at virtual address */
int32_t copy_to_va(const uint8_t* filename, uint32_t virtualaddress, uint32_t length);

/* helper function for reading directory entries by file name */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
/* helper function for reading directory entries by file index */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
/* helper function for reading data */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif

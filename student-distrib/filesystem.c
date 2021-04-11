/* paging.c - Functionality for file system
* vim:ts=4 noexpandtab
*/

#include "filesystem.h"
#include "lib.h"
#include "pcb.h"

// reference: Appendix A (8.1) of MP3, Appendix B of MP3 for open/read/write/close behavior

boot_block_t* fs_boot_block;	/* file system boot block */
uint32_t num_entries;			/* number of files in memory (directory entries) */
uint32_t inode_addr;			/* starting address of inode blocks */
uint32_t datablock_addr;		/* start address of data blocks */
uint32_t max_inodes;			/* N in lecture 16 */
uint32_t max_datablocks;		/* D in lecture 16 */

int32_t opfile_inode = -1;		/* inode # of the opfile opened */
int32_t opfile_bytes_read = -1;	/* number of bytes read through file_read */

// flat directory structure - only directory is "."
int32_t dir_index = 0;			/* dir_entry index of file to read in directory_read */

/*
* filesystem_init(uint32_t fs_addr) 
* Description: initializes the file system by setting the starting
*				address of inodes and datablocks, and the number of files
* Inputs:  fs_addr - starting address of the memory file system
* Outputs: n/a
* Returns: n/a
*/
void filesystem_init(uint32_t fs_addr) {
	fs_boot_block = (boot_block_t*)fs_addr; // beginning address of file system (boot block)
	num_entries = fs_boot_block->dir_count;
	max_inodes = fs_boot_block->inode_count;
	max_datablocks = fs_boot_block->data_count;

	inode_addr = fs_addr + BLOCK_SIZE; // absolute block 1
	datablock_addr = inode_addr + (max_inodes * BLOCK_SIZE); // absolute block N+1
}

/*
* int32_t file_open(const uint8_t* filename)
* Description:	looks for directory entry with the same name as filename and initializes
*				opened file vars
* Inputs:  filename - file name to look for
* Returns: 0 if opened, -1 if invalid input or could not open
* Side Effects: sets opened file vars (inode #, bytes_read) 
*/
int32_t file_open(const uint8_t* filename) {
	if (filename == NULL) { /* invalid pointer/name */
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		opfile_inode = dentry.inode_num;
		opfile_bytes_read = 0;
		return 0;
	}

	return -1;
}

/*
* int32_t file_close(int32_t fd)
* Description:	resets opened file vars (inode #, bytes_read)
* Inputs:  fd - not yet used for checkpoint 2
* Returns: 0 if closed, -1 if error
* Side Effects: resets opened file vars (inode #, bytes_read)
*/
int32_t file_close(int32_t fd) {
	opfile_inode = -1;
	opfile_bytes_read = -1;
	return 0;
}

/*
* int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
* Description: not implements for checkpoint 2
* Inputs: unused for checkpoint 2
* Returns: -1
* Side Effects: n/a
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

/*
* int32_t file_read(int32_t fd, const void* buf, int32_t nbytes)
* Description:	reads opened file 
* Inputs:  fd - index of file to read
*		   buf - address of buffer to write to
*		   nbytes - number of bytes to read
* Outputs: buf - content of opened file (nbytes)
* Returns: number of bytes read (0 = end of the file has been reached)
*		   -1 = fail or invalid input
* Side Effects: increments opened file vars (inode #, bytes_read), modified contents of buf
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
	/* input checks */
	if (buf == NULL || nbytes < 0) {
		return -1;
	}

	PCB *pcb = get_PCB();
	inode_t* found_inode = (inode_t*)(inode_addr + (pcb->file_array[fd].inode*BLOCK_SIZE));
	if(pcb->file_array[fd].file_position >= found_inode->length)
		return 0; // at end of file
	
	int32_t bytes_read = 
		read_data(pcb->file_array[fd].inode, pcb->file_array[fd].file_position, (uint8_t*)buf, nbytes);
	pcb->file_array[fd].file_position += bytes_read;

	return bytes_read;
}

/*
* int32_t directory_open(const uint8_t* filename)
* Description:	looks for directory with the same name as filename and initializes
*				directory index to read
* Inputs:  filename - directory file name to look for
* Returns: 0 if opened, -1 if invalid input or could not open
* Side Effects: sets directory index (dir_index)
*/
int32_t directory_open(const uint8_t* filename) {
	if (filename == NULL) { /* invalid pointer */
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		if (dentry.filetype == 1) { // directory filetypes = 1
			/* assuming there's only one directory, "." at index 0 */
			dir_index = 0;
			return 0;
		}
		return -1; // not a directory
	}

	/* directory not found */
	return -1;
}

/*
* int32_t directory_close(int32_t fd)
* Description: resets directory index to read (dir_index)
* Inputs:  fd - not yet used for checkpoint 2
* Returns: 0 if opened, -1 if invalid input or could not open
*/
int32_t directory_close(int32_t fd) {
	dir_index = -1;
	return 0;
}

/*
* int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
* Description:	doesn't do anything for checkpoint 2
* Inputs: not used for checkpoint 2
* Returns: -1
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

/*
* int32_t directory_read(int32_t fd, const void* buf, int32_t nbytes)
* Description:	reads one filename in the directory, include ".", per call
* Inputs: fd - unused for checkpoint 2
*		  buf - address of buffer to write to
*		  nbutes - number of bytes to read
* Outputs: writes to the buffer
* Returns: number of bytes read/copied to the buffer, -1 on failure/invalid inputs
* Side Effects: increments directory file index read (dir_index)
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes) {
	if (dir_index == -1) { /* there is no directory opened */
		return -1;
	}
	else if (buf == NULL) { /* invalid buf pointer */
		return -1;
	}
	else if (nbytes < 0) { /* must read whole name */
		return -1; 
	}

	/* truncate name depending on byte size  */
	uint32_t num_bytes_to_copy; // = (nbytes < MAX_FILENAME_SIZE) ? nbytes : MAX_FILENAME_SIZE;
	dentry_t dentry;
	if (read_dentry_by_index(dir_index, &dentry) == 0) {
		/* copy the file name into buf */
		num_bytes_to_copy = (strlen(dentry.filename) < MAX_FILENAME_SIZE) ? strlen(dentry.filename) : MAX_FILENAME_SIZE;
		strncpy((int8_t*)buf, dentry.filename, num_bytes_to_copy);
		dir_index++;
		return num_bytes_to_copy;
	}

	return 0;
}

/*
* int32_t get_filesize(const uint8_t* filename)
* Description:	given filename, get the size of the file in bytes
* Inputs: filename - name of file
* Returns: filesize in bytes, -1 if not found
*/
int32_t get_filesize(const uint8_t* filename) {
	if (filename == NULL) {
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		uint32_t inode_index = dentry.inode_num;
		inode_t* inode = (inode_t*)(inode_addr + (inode_index*BLOCK_SIZE));
		return inode->length;
	}

	/* file not found */
	return -1;
}

/*
* int32_t get_filetype(const uint8_t* filename)
* Description:	given filename, get the type of file
* Inputs: filename - name of file
* Returns: filetype (0-rtc, 1-directory, 2-file), -1 if not found
*/
int32_t get_filetype(const uint8_t* filename) {
	if (filename == NULL) {
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		return dentry.filetype;
	}

	/* file not found */
	return -1;
}

/*
* int32_t is_executable(const uint8_t* filename)
* Description:	given filename, check if file is an executable
* Inputs : filename - name of file
* Returns : 1 - executable, -1 - not executable
*/
int32_t is_executable(const uint8_t* filename) {
	if (filename == NULL) {
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		if (dentry.filetype != 2) { // dentry should have regular file type
			return -1;
		}

		uint8_t ELF_chars[4];
		read_data(dentry.inode_num, 0, ELF_chars, 4); // read first 4 characters

		// check that first 4 characters are the specified magic symbols
		if (ELF_chars[0] == 127 && ELF_chars[1] == 'E' && ELF_chars[2] == 'L' && ELF_chars[3] == 'F') {
			return 1;
		}
	}
	
	return -1;
}

/*
* int32_t copy_to_va(const uint8_t* filename, uint8_t virtualaddress)
* Description: copies file to virtual address
* Inputs : filename - name of file to copy
		   virtualaddress - virtual address to copy into
		   length - how much to copy in bytes
* Returns : 0 if copied, -1 if fail
* Side effects: modifies virtual memory
*/
int32_t copy_to_va(const uint8_t* filename, uint32_t virtualaddress, uint32_t length) {
	if (filename == NULL) {
		return -1;
	}

	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry) == 0) {
		// read data into virtual address is file is found
		read_data(dentry.inode_num, 0, (uint8_t *)virtualaddress, length);
		return 0;
	}

	return -1;
}

/* HELPER FUNCTIONS */

/*
* int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
* Description: looks for directory entry with the same name as input. 
* Inputs:  fname - file name to look for
*		   dentry - pointer to copy into
* Outputs: dentry - directory entry in file system with requested name
* Returns: 0 if found, -1 if invalid input or could not find
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
	if (fname == NULL) { /* invalid pointer/name */
		return -1;
	}
	else if (dentry == NULL) {	/* invalid pointer */
		return -1;
	}
	else if (strlen((int8_t*)fname) > MAX_FILENAME_SIZE) { /* invalid file name length */
		return -1;
	}

	int i;
	int8_t* entry_filename;
	uint8_t filename_differences;
	for (i = 0; i < num_entries; i++) { // check all entries
		entry_filename = fs_boot_block->direntries[i].filename;
		/* compare names using lib function */
		filename_differences = strncmp((int8_t*)entry_filename, (int8_t*)fname, MAX_FILENAME_SIZE);

		if (filename_differences == 0) { // match found
			/* fill in the dentry_t block passed - same as read_dentry_by_index(i, dentry) */
			strncpy(dentry->filename, (int8_t*)fname, MAX_FILENAME_SIZE);
			dentry->filetype = fs_boot_block->direntries[i].filetype;
			dentry->inode_num = fs_boot_block->direntries[i].inode_num;
			return 0;
		}
	}

	/* directory entry not found */
	return -1;
}

/*
* int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
* Description: looks for directory entry of given index in boot block 
* Inputs:  index - index within boot block entry array
*		   dentry - pointer to copy into
* Outputs: dentry - directory entry in file system with requested index
* Returns: 0 if found, -1 if invalid input or could not find
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
	if (index > num_entries - 1) { /* invalid index */
		return -1;
	}
	else if (dentry == NULL) { /* invalid pointer */
		return -1;
	}
	
	/* fill in the dentry_t block passed */
	strncpy(dentry->filename, fs_boot_block->direntries[index].filename, MAX_FILENAME_SIZE);
	dentry->filetype = fs_boot_block->direntries[index].filetype;
	dentry->inode_num = fs_boot_block->direntries[index].inode_num;
	return 0;
}

/*
* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
* Description:	reads up to length bytes starting from position offset in the file with
*				inode number inode to buf
*				assumes buf size is at least as large as length to read
* Inputs:	inode - inode number to read data blocks from
*			offset - # of bytes to skip when loading to buf
*			buf - address of buffer to write to
*			length - # of bytes to read
* Outputs: buf - data of inode starting from offset and ending at offset + length
* Returns:	# of bytes read and placed in the buffer if successful
*			-1 on failure or invalid input
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
	
	if (inode > max_inodes - 1) { /* invalid inode number */
		return -1;
	}
	else if (buf == NULL) {	/* invalid pointer */
		return -1;
	}

	inode_t* found_inode = (inode_t*)(inode_addr + (inode*BLOCK_SIZE));
	if (offset >= found_inode->length) { /* past end of file, return */
		return -1;
	}
	else if (length > found_inode->length - offset) { /* crop length to remaining data size if needed */
		length = found_inode->length - offset;
	}

	uint32_t block_offset = offset / BLOCK_SIZE;	// data block to start reading
	uint32_t byte_offset = offset % BLOCK_SIZE; 	// byte to start reading (only for 0th block read)
	uint32_t bytes_copied = 0; 						// same as offset for buffer
	uint32_t copy_size;								// number of bytes to copy
	uint32_t block_index;							// index of data block

	/* deal with the offset for the 0th block to read */
	/* if length >= BLOCK_SIZE - byte_offset, copy the rest of the 0th block (else just part of it) */
	copy_size = (length < (BLOCK_SIZE - byte_offset)) ? length : (BLOCK_SIZE - byte_offset);
	block_index = found_inode->data_block_num[block_offset];
	if (block_index > max_datablocks - 1) { /* invalid block index */
		return -1;
	}
	memcpy(buf, (int8_t*)(datablock_addr + (block_index*BLOCK_SIZE) + byte_offset), copy_size);
	bytes_copied += copy_size;

	int i = 1; // i inits at 1st block (not 0th) - i = # blocks after the 0th block read
	/* copy each data block after 0th, no need for offset after 0th block */
	while (bytes_copied < length) {
		/* if length - bytes_copied >= BLOCK_SIZE, copy the whole block (else just part of it) */
		copy_size = ((length - bytes_copied) < BLOCK_SIZE) ? (length - bytes_copied) : BLOCK_SIZE;
		block_index = found_inode->data_block_num[i + block_offset];
		if (block_index > max_datablocks - 1) { /* invalid block index */
			return -1;
		}
		memcpy((buf+bytes_copied), (int8_t*)(datablock_addr + (block_index*BLOCK_SIZE)), copy_size);
		bytes_copied += copy_size;
		i++; // move to next block
	}

	return bytes_copied;
}

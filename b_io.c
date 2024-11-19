/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Robel Ayelew
 * Student ID::	922419937
 * GitHub-Name:: RobelKasahun
 * Project:: Assignment 5 – Buffered I/O read
 *
 * File:: b_io.c
 *
 * Description:: The aim of this assignment is to be the routines 
 * 				 that we have been using to open a file, read 
 * 				 number of bytes from the file, and closing 
 * 				 freeing used resources, therefore, the assignment
 * 			     is to implement the open(), read(), and close()
 * 				 functions from scratch.
 *
 **************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20 // The maximum number of files open at one time

// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
{
	fileInfo *fi; // holds the low level systems file info

	// Add any other needed variables here to track the individual open file
	// will contain the number of bytes the buffer currently holding
	int current_bytes_buffer_holding;

	// each file control block will have its own buffer
	// as specified in the assignment instructions
	char *buffer;

} b_fcb;

// static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init()
{
	if (startup)
		return; // already initialized

	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].fi = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free File Control Block FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].fi == NULL)
		{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;						 // Not thread safe but okay for this project
		}
	}

	return (-1); // all in use
}

// b_open is called by the "user application" to open a file.  This routine is
// similar to the Linux open function.
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.
// For this assignment the flags will be read only and can be ignored.
b_io_fd b_open(char *filename, int flags)
{
	if (startup == 0)
		b_init(); // Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer

	// This is where you are going to want to call GetFileInfo and b_getFCB
	// returns the fcbArray index where the free file control block stored
	b_io_fd file_descriptor = b_getFCB();

	if (file_descriptor == -1)
	{
		printf("All file control blocks are in use.\n");
		return -1;
	}

	fileInfo *file_info = GetFileInfo(filename);

	if (file_info == NULL)
	{
		printf("The file named %s could not be found.\n", filename);
		return -1;
	}

	/**
	 * at this point, the free file control block has been found,
	 * therefore, the file descriptor should be ready for the b_read() by
	 * initializing default values to buffer to store only 512 bytes,
	 * the number of bytes so far has been red to 0, and file information
	 * such as file name, size, and its location
	 */
	(*(fcbArray + file_descriptor)).current_bytes_buffer_holding = 0;
	(*(fcbArray + file_descriptor)).fi = file_info;
	(*(fcbArray + file_descriptor)).buffer = malloc(B_CHUNK_SIZE);

	return file_descriptor;
}

// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File
int b_read(b_io_fd fd, char *buffer, int count)
{
	//*** TODO ***//
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// and check that the specified FCB is actually in use
	if (fcbArray[fd].fi == NULL) // File not open for this descriptor
	{
		return -1;
	}

	int bytes_to_transfer = 0; // bytes to transfer from my buffer into the caller's buffer
	int transfered_bytes = 0;  // total bytes copied from my buffer to caller's buffer

	if (count > 0)
	{
		// get the remaining bytes count if reading more bytes than the actual file size
		int total_bytes_so_far = fcbArray[fd].current_bytes_buffer_holding + count;
		if (total_bytes_so_far > (*fcbArray[fd].fi).fileSize)
		{
			// we should not read bytes more than the file size
			// thus get the last bytes from the file
			count = (*fcbArray[fd].fi).fileSize - fcbArray[fd].current_bytes_buffer_holding;
		}

		while (true)
		{
			if (count <= transfered_bytes)
			{
				break;
			}

			uint32_t remainder_bytes = fcbArray[fd].current_bytes_buffer_holding % B_CHUNK_SIZE;
			// should not read bytes more than B_CHUNK_SIZE
			int remaining_bytes = B_CHUNK_SIZE - remainder_bytes;

			// bytes so far red plus current bytes
			// should not exceed the remaining bytes
			// from B_CHUNK_SIZE
			if ((count - transfered_bytes) < remaining_bytes)
			{
				// compute the exact number of bytes to transfer from my buffer to
				// the callers buffer
				bytes_to_transfer = count - transfered_bytes;
			}
			else
			{
				// transfer the remaining bytes from my buffer into the caller's buffer
				bytes_to_transfer = remaining_bytes;
			}

			// reading 1 block at a time from certain position from the file into my buffer
			uint64_t lba_position_block = (*fcbArray[fd].fi).location +
										  fcbArray[fd].current_bytes_buffer_holding / B_CHUNK_SIZE;

			LBAread(fcbArray[fd].buffer, 1, lba_position_block);

			// copying [ bytes ] bytes from my buffer into the caller's buffer
			const void *source = fcbArray[fd].buffer + remainder_bytes;
			memcpy(buffer, source, bytes_to_transfer);

			// number of bytes I copied from my buffer into the caller's buffer
			transfered_bytes += bytes_to_transfer;

			// total number of bytes I copied from my buffer into the caller's buffer
			fcbArray[fd].current_bytes_buffer_holding += bytes_to_transfer;
		}

		// transfered bytes into the caller's buffer
		return transfered_bytes;
	}
	else
	{
		// nothing to read from the file
		// when count is less than 0 or is 0
		return count;
	}
}

// b_close frees and allocated memory and places the file control block back
// into the unused pool of file control blocks.
int b_close(b_io_fd fd)
{
	//*** TODO ***//  Release any resources
	// free the storage allocated for buffer when buffer it is not NULL
	if (fcbArray[fd].buffer != NULL)
	{
		free((*(fcbArray + fd)).buffer);
		(*(fcbArray + fd)).buffer = NULL;
	}
}
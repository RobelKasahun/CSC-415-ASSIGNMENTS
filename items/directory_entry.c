


#ifndef _DIRECTORY_ENTRY_C
#define _DIRECTORY_ENTRY_C

#include <time.h>
#include<stdint.h>

#define NAME 100

typedef struct DirectoryEntry
{
    char name[NAME + 1];                    // Name of the directory entry
    uint32_t LBAlocation;                   // location where the directory entry is stored
    time_t timeCreation;                    // the time the directory entry created
    uint32_t size;                          // size of the directory entry
    uint32_t permissions;                   // permission for who can access the directory
} DirectoryEntry;

#endif











// #ifndef _VOLUME_CONTROL_BLOCK_C
// #define _VOLUME_CONTROL_BLOCK_C
// #include <stdint.h>

// typedef struct VolumeControlBlock
// {
//     uint32_t block_size;           // the size of an individual block in bytes
//     uint32_t total_blocks;         // the total number of blocks in our volume control blocks
//     uint32_t free_blocks;          // current blocks that are not occupied
//     uint32_t signature;            // signiture for file system type [For instance: NTFS, FAT32, or ext4]
//     uint32_t root_directory_block; // (block number) the location of the root directory
// } VolumeControlBlock;

// #endif










/**************************************************************
* Class::  CSC-415-02 Spring 2024
* Name:: Leonardo Meza Martinez, Robel Ayelew, Andres Garcia-Barros, Thomas Bercasio
* Student IDs:: 922698683, 922419937, 921107769, 920652106
* GitHub-Name::
* Group-Name:: Forks
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*
**************************************************************/
#include "mfs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/stat.h>


#define DIRECTORY_NAME_MAX 256


char * cwdPath;
DirectoryEntry * currentWorkingDirectory;
DirectoryEntry * rootDirectory;




//0 is valid path
//-1 is error
int parsePath (char * pathname, struct ppRetInfo * ppInfo){
    DirectoryEntry * startParent;
    if(pathname == NULL)
        return -1;//No path given
    if(ppInfo == NULL)
        return -1;
    ppInfo->lastElementName = (char*)malloc(MAX_PATH_NAME_SIZE);
    if(pathname[0] == '/'){
        //make parent directory the root
        strcpy(ppInfo->lastElementName, "/");
        startParent = rootDirectory;
    }else{
        //make parent directory the current working directory
        startParent = currentWorkingDirectory;
        strcpy(ppInfo->lastElementName, currentWorkingDirectory->path);
        //prevents double slash (//) in path name
        if(startParent != rootDirectory){
            strcat(ppInfo->lastElementName, "/");
        }
    }
    DirectoryEntry * parent;
    parent = startParent;

   
    char * saveptr;
    char * token = __strtok_r(pathname, "/", &saveptr);

    if(token == NULL){
        if(pathname[0] == '/'){
            //path is root
            ppInfo->lastElementName = NULL;
            ppInfo->lastElementIndex = -2;
            ppInfo->parent = parent;
            return 0;
        }
        else{
            //invalid
            return -1;
        }

    }
    while(1){
        int index = findInDir(token, parent);
        char * token2 = __strtok_r(NULL, "/", &saveptr);

        if(token2 == NULL){//token on last name on pathnane
            //ppInfo->lastElementName = token;
            //if index is -1 then pathname is valid and last directory/file is not present - for mkdir
            ppInfo->lastElementIndex = index;
            ppInfo->parent = parent;
            return 0;
        }
        if(index == -1){//Could not find directory in the middle of path name - invalid path
            return -1;
        }
        if(isDirectory(parent, index) != IS_DIRECTORY){
            return -1;
        }
        DirectoryEntry * tempParent = loadDirectory(parent, index);
        if(parent != startParent)
            free(parent);
        parent = tempParent;
        token = token2;  
        strcat(ppInfo->lastElementName, token);
        strcat(ppInfo->lastElementName, "/");
    }
    

}

//Returns index of dir with name if found and -1 otherwise
int findInDir(char * name, DirectoryEntry * parent){
    DirectoryEntry * temp = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    for(int i = 2; i < parent->numOfEntriesInitialized;i++){
        LBAread(temp,1,parent->entries[i]);
        if(strcmp(temp->name, name) == 0){
            int retLocation = temp->location;
            free(temp);
            return i;
        }
    }
    printf("DIRECTORY NAME %s NOT FOUND\n", name);
    free(temp);
    return -1;
}

DirectoryEntry * loadDirectory(DirectoryEntry * parent, int index){
    DirectoryEntry * dir = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    LBAread(dir, 1, parent->entries[index]);
    return dir;
}


int isDirectory(DirectoryEntry * parent, int entryIndex){
    DirectoryEntry * entry = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    int isDir;
    LBAread(entry, 1, parent->entries[entryIndex]);//entry now holds the info at block entries[entryIndex]
    isDir = entry->isDir;//There are #defines IS_DIRECTORY(1) and NOT_DIRECTORY(0)
    free(entry);
    return isDir;
    
}


//These two are for fsinit
void setRootDir(DirectoryEntry * root){
    rootDirectory = root;
}
void setLoadedDir(DirectoryEntry * dir){
    currentWorkingDirectory = dir;
}
void intCWDPath(char * path){
    cwdPath = (char*)malloc(MAX_PATH_NAME_SIZE);
    strcpy(cwdPath, path);
}

//extracts the last token on a path
char * extractName(char * path){
    char * token;
    char * save;
    token = __strtok_r(path, "/", &save);
    while(1){
        char * token2 = __strtok_r(NULL,"/", &save);
        if(token2 == NULL){
            return token;
        }
        token = token2;
    }

}

int fs_mkdir(const char *pathname, mode_t mode) {
    struct ppRetInfo * dirInfo = (struct ppRetInfo *)malloc(sizeof(struct ppRetInfo));
    int directoryFound = parsePath( (char *)pathname, dirInfo);
    if(directoryFound == -1){
        free(dirInfo);
        printf("INVALID PATH NAME\n");
        return -1;
    }else{
        if(dirInfo->lastElementIndex != -1){
            printf("DIRECTORY PATH ALREADY EXISTS!\n");
            free(dirInfo);
            return -1;
        }else{
            
            DirectoryEntry * DirectoryToMake = createDirectory(10, dirInfo->parent);
            //No directory entry was found to be unused
            if(DirectoryToMake == NULL){
                printf("DIRECTORY PARENT IS FULL - UNABLE TO MAKE DIRECTORY\n");
                return -1;
            }
            char * newDirName = extractName((char *) pathname);
            strcpy(DirectoryToMake->name, newDirName);
            strcat(dirInfo->lastElementName, newDirName);
            strcpy(DirectoryToMake->path, dirInfo->lastElementName);
            printf("WRITING %s DIRECTORY TO DISK AT BLOCK %d\n", pathname, DirectoryToMake->location);
            LBAwrite(DirectoryToMake, 1, DirectoryToMake->location);
            free(DirectoryToMake);
        }
    }
    return 0;
}



int fs_rmdir(const char *pathname)
{
    // return -1 if the pathname is NULL
    if (pathname == NULL)
    {
        fprintf(stderr, "Error: Pathname is NULL.\n");
        return EXIT_FAILURE;
    }

    // remove the directory using the specified pathname
    // return -1 if the pathname does not exit
    if (rmdir(pathname) == -1)
    {
        fprintf(stderr, "Error removing directory.\n");
        return EXIT_FAILURE;
    }

    // the directory removed, therefore return success code
    return EXIT_SUCCESS;
}

fdDir *fs_opendir(const char *pathname) {
    // Open the directory
    DIR *directoryPointer = opendir(pathname);

    if (directoryPointer == NULL) {

        printf("Failed to open directory.\n");
        return NULL;
        
    }

    // Allocate memory for the directory structure
    fdDir *openDir = (fdDir *)malloc(sizeof(fdDir));

    if (openDir == NULL) {

        printf("Failed to allocate memory for directory structure.\n");
        closedir(directoryPointer);
        return NULL;

    }
    openDir->dirEntryPosition = 0;
    openDir->directoryPointer = directoryPointer;
    return openDir;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    if (dirp == NULL || dirp->directoryPointer == NULL) {
        printf("Invalid directory structure.\n");
        return NULL;
    }
    
    // Attempt to read a directory entry
    struct dirent *entry = readdir(dirp->directoryPointer);
    
    // Check if readdir reached the end of the directory
    if (entry == NULL) {

        if (readdir(dirp->directoryPointer) == NULL) {
            return NULL;
        } else {
            perror("Failed to read directory entry.\n");
            return NULL;
        }
    }
    
    // Allocate memory for directory item information structure
    struct fs_diriteminfo *info = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (info == NULL) {
        printf("Failed to allocate memory for directory item information structure.\n");
        return NULL;
    }
    
    // Copy directory entry information to the directory item info structure
    strncpy(info->d_name, entry->d_name, DIRECTORY_NAME_MAX - 1);
    info->d_name[DIRECTORY_NAME_MAX - 1] = '\0';
    
    return info;
}


int fs_closedir(fdDir *dirp){
    // Check if the directory structure is valid
    if (dirp != NULL) {

        closedir(dirp->directoryPointer); 
        free(dirp); 

    }
    return 0; 
}

char *fs_getcwd(char *pathname, size_t size) {
    int cwdStringSize = strlen(cwdPath);
    if(cwdStringSize > size){
        return NULL;
    }else{
        strncpy(pathname, cwdPath, size);
        return pathname;
    }
}

int fs_setcwd(char *pathname){
    struct ppRetInfo * dirInfo = (struct ppRetInfo *)malloc(sizeof(struct ppRetInfo));
    int directoryFound = parsePath(pathname, dirInfo);
    if(directoryFound == -1){
        free(dirInfo);
        printf("INVALID PATH NAME\n");
        return -1;
    }else{
        if(dirInfo->lastElementIndex == -2){
            printf("MAKING CWD THE ROOT DIRECTORY\n");
            currentWorkingDirectory = rootDirectory;
            strcpy(cwdPath, "/");
            setLoadedDir(rootDirectory);
            free(dirInfo);
        }else{
            if(dirInfo->lastElementIndex == -1){
                free(dirInfo);
                printf("DIRECTORY NOT FOUND\n");
                return -1;
            }
            DirectoryEntry *newCWD = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
            LBAread(newCWD, 1, dirInfo->parent->entries[dirInfo->lastElementIndex]);
         
            strcpy(cwdPath, newCWD->path);
            setLoadedDir(newCWD);
            free(dirInfo);
        }
    }
    return 0;
}

int fs_isFile(char *filename){

    // Check if the file exists
    if (access(filename, F_OK) != -1) {
        
        struct stat fileStat;

        // Get the file information
        if (stat(filename, &fileStat) == 0) {

            // Check if it's a regular file
            if (S_ISREG(fileStat.st_mode)) { 
                return 1;
            }
        }
    }
    return 0;
}

int fs_isDir(char *pathname){

    // Check if the path exists
    struct stat pathStat;

    if (stat(pathname, &pathStat) == 0) {

        /// Check if it's a directory
        if (S_ISDIR(pathStat.st_mode)) {
          
            return 1;
        }
    }
    
    return 0;
}

//getEntryLocation will return the location in the file system of the passed pathname if it exists
fdDir * getEntryLocation(char * pathname){
    printf("Searching for file or directory at : '%s'\n", pathname);
    for(int i = 0; i < MAX_NUMBER_OF_ENTRIES; i++){
        printf("Placeholder text\n");
    }
    return NULL;
    //TODO: Traverse nodes looking for a directory entry with a matching pathname as the one given.
}

int fs_delete(char *filename)
{
    if (filename == NULL)
    {
        fprintf(stderr, "Error: filename is NULL.\n");
        return EXIT_FAILURE;
    }

    if (remove(filename) == -1)
    {
        fprintf(stderr, "Error removing the file.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int fs_stat(const char *path, struct fs_stat *buf) {
    // Check if the path and buffer are valid
    if (path == NULL || buf == NULL) {

        printf("Invalid path or buffer.\n");
        return -1;

    }

    // Get the file information
    struct stat fileStat;

    // Check if the file exists
    if (stat(path, &fileStat) == -1) {

        printf("Failed to get file information.\n");
        return -1;

    }

    // Copy the file information to the buffer
    buf->st_size = fileStat.st_size;
    buf->st_blksize = fileStat.st_blksize;
    buf->st_blocks = fileStat.st_blocks;
    buf->st_accesstime = fileStat.st_atime;
    buf->st_modtime = fileStat.st_mtime;
    buf->st_createtime = fileStat.st_ctime;

    printf("Stat called successfully.\n");
    return 0;
}


int writeToDisk(void * buffer, int sizeOfBuffer, int blocksArray []){
    int positionInBuffer = 0;
    int numOfBlocksToWrite = calculateBlocks(sizeOfBuffer);
    for(int i = 0; i < numOfBlocksToWrite; i++){
        LBAwrite(buffer + positionInBuffer, 1, blocksArray[i]);
        positionInBuffer += getVCBBlockSize();
    }
    return 0;
}
DirectoryEntry * createDirectory(int numOfEntries, DirectoryEntry * parent){
    int blocksForEntries = calculateBlocks(numOfEntries * sizeof(DirectoryEntry));
    int sizeForAllocation = blocksForEntries * getVCBBlockSize();
    blocksForEntries = sizeForAllocation / sizeof(DirectoryEntry);

    DirectoryEntry * newDirectory = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    int newDirectoryLocation[calculateBlocks(sizeof(DirectoryEntry))];
    if(parent == NULL){
        allocateBlocks(newDirectoryLocation, 1);
        newDirectory->location = newDirectoryLocation[0];
    }else{
        newDirectoryLocation[0] = parent->entries[findInDir("\0", parent)];
        if( newDirectoryLocation[0] == -1){
            return NULL; //No available directory entry in parent
        }else{
            newDirectory->location = newDirectoryLocation[0];
        }
    }
    newDirectory->isDir = IS_DIRECTORY;
    newDirectory->creationTime = -1;
    newDirectory->permissions = -1;
    newDirectory->size = -1;

    DirectoryEntry * dirEntry;
    for(int i = 2; i < numOfEntries;i++){
        dirEntry = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
        int newEntryLocation[1];
        allocateBlocks(newEntryLocation, 1);
        dirEntry->location = newEntryLocation[0];
        strcpy(dirEntry->name, "\0");
        dirEntry->entries[1] = newDirectoryLocation[0];
        dirEntry->entries[0] = newEntryLocation[0];
        newDirectory->entries[i] = newEntryLocation[0];
        dirEntry->isDir = UNDEFINED_ENTRY;
        dirEntry->creationTime = -1;
        dirEntry->permissions = -1;
        dirEntry->size = -1;
        if(parent == NULL){
            strcpy(dirEntry->path, "/");
        }else{
            strcpy(dirEntry->path, parent->path);
            strcat(dirEntry->path, "/");
        }
        writeToDisk(dirEntry, sizeof(DirectoryEntry), newEntryLocation);
        free(dirEntry);
    }
    newDirectory->numOfEntriesInitialized = numOfEntries;

    //Both create a whole new directory entry but their location redirects to self and parent
    //Initialize the first entry "."
    DirectoryEntry * selfEntry = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    int newEntryLocation[1];
    allocateBlocks(newEntryLocation, 1);
    selfEntry->location = newDirectory->location;
    const char * self = ".";
    strcpy(selfEntry->name, self);
    newDirectory->entries[0] = newDirectoryLocation[0];
    writeToDisk(selfEntry, sizeof(DirectoryEntry), newEntryLocation);

    //Initialize the second entry ".."
    DirectoryEntry * parentEntry = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    const char * parentName = "..";
    if(parent != NULL){
        int parentEntryLocation[1];
        allocateBlocks(parentEntryLocation, 1);
        parentEntry->location = parent->location;
        strcpy(parentEntry->name, parentName);
        newDirectory->entries[1] = parentEntryLocation[0];
        parentEntry->isDir = IS_DIRECTORY;
        parentEntry->creationTime = -1;
        parentEntry->permissions = -1;
        parentEntry->size = -1;
        writeToDisk(parentEntry, sizeof(DirectoryEntry), newEntryLocation);
    }else{//Directory being created is root
        int newEntryLocation[1];
        allocateBlocks(newEntryLocation, 1);
        parentEntry->location = newDirectory->location;
        strcpy(parentEntry->name, parentName);
        newDirectory->entries[1] = newDirectoryLocation[0];
        parentEntry->isDir = IS_DIRECTORY;
        parentEntry->creationTime = -1;
        parentEntry->permissions = -1;
        parentEntry->size = -1;
        strcpy(newDirectory->path, "/");
        writeToDisk(parentEntry, sizeof(DirectoryEntry), newEntryLocation);
    }

    return newDirectory;
}

// Calculate number of blocks needed for a given size
int calculateBlocks(int totalSize)
{
    return (totalSize + (getVCBBlockSize() - 1)) / getVCBBlockSize();
}

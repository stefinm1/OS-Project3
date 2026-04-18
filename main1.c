/*  
    Maria Stefiniv & Milagros Villalba
    CSC345-2
    Project 3
*/

/* Part 1: No page replacement - FIFO-based TLB update */

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define NUM_FRAMES 256
#define NUM_BYTES 256

typedef struct pageTableEntry{
    int frameNumber; /* Frame in physical memory */
    int inMemory; /* Valid-Invalid Bit */
}pageTableEntry;

pageTableEntry pageTable[PAGE_SIZE]; /* Page Table */

typedef struct tlbEntry{ 
    int pageNumber; /* Page number */
    int frameNumber; /* Frame number */
    int occupied; /* Frame status: occupied or not */
}tlbEntry;

int tlbEntriesCount = 0;
tlbEntry TLB[TLB_SIZE]; /* TLB */

char physicalMemory[NUM_FRAMES][NUM_BYTES]; /* Physical Memory: 65,536 bytes */
int memoryUsed[NUM_FRAMES]; /* Keeps track of used physical memory frames */

/* Update TLB after reading from BACKING_STORE */
void updateTLB(int pageNum, int frame){
    /* Add to TLB table */
    TLB[tlbEntriesCount].pageNumber = pageNum;
    TLB[tlbEntriesCount].frameNumber = frame;
    TLB[tlbEntriesCount].occupied = 1;

    /* Increase table entry count */
    if(tlbEntriesCount >= 16) tlbEntriesCount = 0;
    else tlbEntriesCount++;
}

/* Update Page table after reading from BACKING_STORE */
void updatePageTable(int pageNumber, int frame){
    pageTable[pageNumber].frameNumber = frame;
    pageTable[pageNumber].inMemory = 1;
}

/* Read page from BACKING_STORE and store in physical memory */
int insertIntoMemory(int pageNumber, FILE *backingStore){
    int pageByte = pageNumber * PAGE_SIZE; /* Page byte value */

    if(fseek(backingStore, pageByte, SEEK_SET) != 0){ /* Moves to page */
        printf("Count not seek page in BACKING_STORE\n"); 
        return 1; /* Returns error if seek fails */
    }

    size_t byteCount;
    for(int i = 0; i < NUM_FRAMES; i++){
        if(memoryUsed[i] == -1){
            memoryUsed[i] =  1; /* Frame set to used */
            byteCount = fread(physicalMemory[i], 1, NUM_BYTES, backingStore); /* Read bytes from backing store */
            if(byteCount < NUM_BYTES){
                printf("Error reading from BACKING_STORE\n");
                return 1; /* Return error if read fails */
            }else{
                updateTLB(pageNumber, i);
                updatePageTable(pageNumber, i);
                return i;
            }
        }
    }

    return -1; /* No free frames found */
}

/* Check page table for frame number */
int checkPageTable(int pageNumber){
    if(pageTable[pageNumber].inMemory){ /* Checks if loaded in memory */
         /* Add to TLB table */
        TLB[tlbEntriesCount].pageNumber = pageNumber;
        TLB[tlbEntriesCount].frameNumber = pageTable[pageNumber].frameNumber;
        TLB[tlbEntriesCount].occupied = 1;

        /* Increase table entry count */
        if(tlbEntriesCount >= 16) tlbEntriesCount = 0;
        else tlbEntriesCount++;

        return pageTable[pageNumber].frameNumber; /* Hit: return frame number */
    }

    return -1; /* Miss: must read from backing store */
}

/* Check TLB for frame number */
int checkTLB(int pageNumber){
    /* Loop through TLB table for page number */
    for(int i = 0; i < TLB_SIZE; i++){
        if(TLB[i].occupied && TLB[i].pageNumber == pageNumber)
            return TLB[i].frameNumber; /* TLB hit */
    }

   return -1; /* TLB Miss: check page table */
}

/* Translate logical address to physical address */
void translateLogicalAddr(int logicalAddr, int pageNum, int offset, FILE *backingStore){
    int frame = checkTLB(pageNum); /* Check TLB table */
    if(frame == -1) frame = checkPageTable(pageNum); /* Check page table */
    if(frame == -1) frame = insertIntoMemory(pageNum, backingStore); /* Page fault: */
    int physicalAddress = (frame << 8) | offset; /* Calculate physical address */
} 

int main(int argc, char *arg[]){
    /* Initialize page table */
    for(int i = 0; i < PAGE_SIZE; i++){
        pageTable[i].inMemory = 0;
        pageTable[i].frameNumber = -1;
    }

    /* Initialize TLB table */
    for(int i = 0; i < TLB_SIZE; i++) TLB[i].occupied = 0;

    /* Initialize memoryUsed array */
    for(int i = 0; i < NUM_FRAMES; i++) memoryUsed[i] = -1;

    /* Check command line argument: */
    if (argc != 2){
        printf("Usage: ./main1 addresses.txt\n");
        return 1;
    }

    /* Open the file: */
    FILE *addresses = fopen(arg[1], "r");
    if (addresses == NULL){
        printf("Error opening file.\n");
        return 1;
    }

    /* Open backing store: */
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL){
        printf("Error opening backing store.\n");
        return 1;
    }

    /* Read logical addresses: */
    int logical_address;

    while (fscanf(addresses, "%d", &logical_address) != EOF) {

        int page = (logical_address >> 8) & 0xFF; /* Page number */
        int offset = logical_address & 0xFF; /* Page offset */

        printf("logical=%d page=%d offset=%d\n", logical_address, page, offset);

        translateLogicalAddr(logical_address, page, offset, backing_store); /* Translate logical to physical addr */
    }

    /* Close files: */
    fclose(addresses);
    fclose(backing_store);

    return 0;
}
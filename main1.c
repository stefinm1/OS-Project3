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
    int inMemory; /* Memory status */
    int frameNumber; /* Frame in physical memory */
}pageTableEntry;

int pageEntriesCount = 0;
pageTableEntry pageTable[PAGE_SIZE]; /* Page Table */

typedef struct tlbEntry{ 
    int pageNumber; /* Page number */
    int frameNumber; /* Frame number */
    int occupied; /* Frame status: occupied or not */
}tlbEntry;

int tlbEntriesCount = 0;
tlbEntry TLB[TLB_SIZE]; /* TLB */

char physicalMemory[NUM_FRAMES][NUM_BYTES]; /* Physical Memory: 65,536 bytes */

void addTLBEntry(int pageNumber){

    tlbEntriesCount++;
}

void addPageEntry(int pageNumber){
    pageEntriesCount++;
}

int insertIntoMemory(int pageNumber){
    /*
    Ex: if logical address with page number 15 = page fault, read in page 15 from
        Backing_store and store it in a page frame in physical memory (page table & TLB updated)

    BACKING_STORE:
        - treat as random-access file to randomly seek to certain positions for r
        - fopen(), fread(), fseek(), fclose() 

    1. read in 256-byte page (page number) from Backing_store 
    2. Store in an available page frame in physical memory
    3. Update TLB and page table
    */
}

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

    return -1; /* Return -1 if page fault */
}

int checkTLB(int pageNumber){
    /* Loop through TLB table for page number */
    for(int i = 0; i < TLB_SIZE; i++){
        if(TLB[i].occupied && TLB[i].pageNumber == pageNumber)
            return TLB[i].frameNumber; /* TLB hit */
    }

   return -1; /* TLB Miss: check page table */
}

/* Translate logical address to physical address */
void translateLogicalAddr(int logicalAddr, int pageNum, int offset){
    int frame = checkTLB(pageNum); /* Check TLB table */
    if(frame == -1) frame = checkPageTable(pageNum); /* Check page table */
    if(frame == -1) frame = insertIntoMemory(pageNum); /* Page fault: */
    int physicalAddress = (frame << 8) | offset; /* Calculate physical address */
} 

int main(int argc, char *arg[]){
    /* Initialize page table */
    for(int i = 0; i < TLB_SIZE; i++){
        pageTable[i].inMemory = 0;
        pageTable[i].frameNumber = -1;
    }

    /* Initialize TLB table */
    for(int i = 0; i < TLB_SIZE; i++) TLB[i].occupied = 0;

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

        translateLogicalAddr(logical_address, page, offset); /* Translate logical to physical addr */
    }

    /* Close files: */
    fclose(addresses);
    fclose(backing_store);

    return 0;
}







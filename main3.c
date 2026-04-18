/*  
    Maria Stefiniv & Milagros Villalba
    CSC345-2
    Project 3
*/

/* Part 3: Page replacement (LRU-based) */

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256 /* Page size: 2^8 bytes = 256 bytes */
#define TLB_SIZE 16 /* TLB: 16 entries */
#define NUM_FRAMES 256 /* Physical memory: 256 frames */
#define NUM_BYTES 256 /* Physical memory: 256 bytes per frame */

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
int translateLogicalAddr(int logicalAddr, int pageNum, int offset, FILE *backingStore,
                          FILE *out1, FILE *out2, FILE *out3){
    int frame = checkTLB(pageNum); /* Check TLB table */
    if(frame == -1) frame = checkPageTable(pageNum); /* Check page table */
    if(frame == -1) frame = insertIntoMemory(pageNum, backingStore); /* Page fault: */
    
    if(frame == -1){ /* No frame found */
        printf("No Frame found");
        return 1;
    }

    int physicalAddress = (frame << 8) | offset; /* Calculate physical address */
    fprintf(out1, "%d\n", logicalAddr); /* out1: logical address being translated */
    fprintf(out2, "%d\n", physicalAddress); /* out2: corresponding physical address */
    fprintf(out3, "%d\n", (signed char)physicalMemory[frame][offset]); /* out3: signed byte value in physical memory at physical address */

    printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddr, physicalAddress, (signed char)physicalMemory[frame][offset]);
    
    return 0;
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

    /* Output file: logical address being translated */
    FILE *out1 = fopen("out1.txt", "w");
    if(out1 == NULL){
        printf("Error creating out1 \n");
        return 1;
    }

    /* Output file: corresponding physical address */
    FILE *out2 = fopen("out2.txt", "w");
    if(out2 == NULL){
        printf("Error creating out2 \n");
        return 1;
    }

    /* Output file: signed byte value stored in physical memory at translated physical address */
    FILE *out3 = fopen("out3.txt", "w");
    if(out3 == NULL){
        printf("Error creating out3 \n");
        return 1;
    }

    /* Read logical addresses: */
    int logical_address;

    while (fscanf(addresses, "%d", &logical_address) != EOF) {

        int page = (logical_address >> 8) & 0xFF; /* Page number */
        int offset = logical_address & 0xFF; /* Page offset */

        translateLogicalAddr(logical_address, page, offset, backing_store, out1, out2, out3); /* Translate logical to physical addr */
    }

    /* Close files: */
    fclose(addresses);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);

    return 0;
}
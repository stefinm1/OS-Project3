/*  
    Maria Stefiniv & Milagros Villalba
    CSC345-2
    Project 3
*/

/* Part 2: Page replacement (FIFO-based) */

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256   /* Page size: 2^8 bytes = 256 bytes */
#define TLB_SIZE 16     /* TLB: 16 entries */
#define NUM_FRAMES 128  /* Trigger page replacement after memory fills up */
#define NUM_BYTES 256   /* Physical memory: 256 bytes per frame */

typedef struct pageTableEntry{
    int frameNumber;    /* Frame in physical memory */
    int inMemory;       /* Valid-Invalid Bit */
}pageTableEntry;

typedef struct tlbEntry{ 
    int pageNumber;     
    int frameNumber;    
    int occupied;       /* Frame status: occupied or not */
}tlbEntry;

/* Global structures*/
pageTableEntry pageTable[PAGE_SIZE];   
tlbEntry TLB[TLB_SIZE]; 
char physicalMemory[NUM_FRAMES][NUM_BYTES]; /* Physical Memory: 128*256 bytes */
int memoryUsed[NUM_FRAMES];                 /* Keeps track of used physical memory frames */

int tlbEntriesCount = 0;
int nextFrame = 0; /* Index for FIFO page replacement */

/* Stats */
int page_faults = 0;
int tlb_hits = 0;
int total_addresses = 0;

/* Update TLB */
void updateTLB(int pageNum, int frame){

    TLB[tlbEntriesCount].pageNumber = pageNum;
    TLB[tlbEntriesCount].frameNumber = frame;
    TLB[tlbEntriesCount].occupied = 1;

    tlbEntriesCount = (tlbEntriesCount + 1) % TLB_SIZE;
}

/* Update Page table */
void updatePageTable(int pageNumber, int frame){
    pageTable[pageNumber].frameNumber = frame;
    pageTable[pageNumber].inMemory = 1;
}

/* Check page table for frame number */
int checkPageTable(int pageNumber){
    if(pageTable[pageNumber].inMemory){ /* Checks if loaded in memory */
        return pageTable[pageNumber].frameNumber; /* Hit */
    }

    return -1; /* Miss */
}

/* Check TLB for frame number */
int checkTLB(int pageNumber){
    for(int i = 0; i < TLB_SIZE; i++){
        if(TLB[i].occupied && TLB[i].pageNumber == pageNumber){
            tlb_hits++;
            return TLB[i].frameNumber; /* TLB hit */
        }
    }

   return -1; /* TLB Miss */
}

/* Load page from BACKING_STORE into memory */
int insertIntoMemory(int pageNumber, FILE *backingStore){
    int frame = -1;

    /* Check for free frame */
    for(int i = 0; i < NUM_FRAMES; i++){
        if(memoryUsed[i] == -1){
            memoryUsed[i] =  1;         /* Frame set to used */
            frame = i;
            break;
        }
    }

    /* If no free frame, replace using FIFO */
    if(frame == -1){
        frame = nextFrame; /* Get next frame to replace */
        nextFrame = (nextFrame + 1) % NUM_FRAMES; /* Update next frame index */

        /* Find page currently in the frame and update page table */
        for(int i = 0; i < PAGE_SIZE; i++){
            if(pageTable[i].inMemory && pageTable[i].frameNumber == frame){
                pageTable[i].inMemory = 0; /* Mark page as not in memory */
                break;
            }
        }
    }

    int pageByte = pageNumber * PAGE_SIZE;

    if(fseek(backingStore, pageByte, SEEK_SET) != 0){   /* Moves to page */
        printf("Count not seek page in BACKING_STORE\n"); 
        return -1; /* Returns error if seek fails */
    }

    size_t byteCount = fread(physicalMemory[frame], sizeof(char), NUM_BYTES, backingStore);
    if(byteCount < NUM_BYTES){
        printf("Error reading from BACKING_STORE\n");
        return -1; /* Return error if read fails */
    }

    updatePageTable(pageNumber, frame);
    page_faults++;

    return frame; 
}

/* Translate logical address to physical address */
int translateLogicalAddr(int logicalAddr, int pageNum, int offset, FILE *backingStore,
                          FILE *out1, FILE *out2, FILE *out3){
    int frame = checkTLB(pageNum); /* Check TLB table */

    if(frame == -1) frame = checkPageTable(pageNum); /* Check page table */
    
    if(frame == -1) {
        frame = insertIntoMemory(pageNum, backingStore);
        updateTLB(pageNum, frame);
    }

    if(frame == -1){ 
        printf("No Frame found");
        return 1;
    }

    int physicalAddress = (frame << 8) | offset; /* Calculate physical address */
    fprintf(out1, "%d\n", logicalAddr); /* out1: logical address being translated */
    fprintf(out2, "%d\n", physicalAddress); /* out2: corresponding physical address */
    fprintf(out3, "%d\n", (signed char)physicalMemory[frame][offset]); /* out3: signed byte value in physical memory at physical address */

    total_addresses++;
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
        printf("Usage: ./main2 addresses.txt\n");
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

    /* Output file 1: logical address being translated */
    FILE *out1 = fopen("out1.txt", "w");
    if(out1 == NULL){
        printf("Error creating out1 \n");
        return 1;
    }

    /* Output file 2: corresponding physical address */
    FILE *out2 = fopen("out2.txt", "w");
    if(out2 == NULL){
        printf("Error creating out2 \n");
        return 1;
    }

    /* Output file 3: signed byte value stored in physical memory at translated physical address */
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

    printf("\n(FIFO-Based Page Replacement)\n");
    printf("Page Faults = %d / %d\n", page_faults, total_addresses);
    printf("Page Fault Rate = %.3f\n", (double)page_faults / total_addresses);
    printf("TLB Hits = %d / %d\n", tlb_hits, total_addresses);
    printf("TLB Hit Rate = %.3f\n", (double)tlb_hits / total_addresses);

    /* Close files: */
    fclose(addresses);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);

    return 0;
}
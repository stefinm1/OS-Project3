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
    int valid; /* Memory status */
    int frameNumber; /* Frame in physical memory */
}pageTableEntry;

pageTableEntry pageTable[PAGE_SIZE]; /* Page Table */

typedef struct tlbEntry{ 
    int pageNumber; /* Page number */
    int frameNumber; /* Frame number */
    int valid; /* Usage Status */
}tlbEntry;

tlbEntry TLB[TLB_SIZE]; /* TLB */

char physicalMemory[NUM_FRAMES][NUM_BYTES]; /* Physical Memory */

void insertIntoMemory(){
    /*
    Ex: if logical address with page number 15 = page fault, read in page 15 from
        Backing_store and sore it in a page frame in physical memory (page table & TLB updated)

    BACKING_STORE:
        - treat as random-access file to randomly seek to certain positions for r
        - fopen(), fread(), fseek(), fclose() 

    1. read in 256-byte page (page number) from Backing_store 
    2. Store in an available page frame in physical memory
    3. Update TLB and page table
    */
}
void checkPageTable(){
    /*
    1. Check table for page Number 
    2. extract the corresponding frame number from the page table
            - Hit: frame number obtained
                   frame number + offset = physical address
            - Miss: Page fault occurs
    4. 
    */

   insertIntoMemory();
}

void checkTLB(){
    /*
    Consult TLB
        - TLB hit: frame number is obtained from the TLB
        - TLB Miss: consult page table 
    */
   checkPageTable();
}

int main(int argc, char *arg[]){

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
    }

    /* Close files: */
    fclose(addresses);
    fclose(backing_store);

    return 0;
}







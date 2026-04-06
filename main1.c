/*  
    Maria Stefiniv & Milagros Villalba
    CSC345-2
    Project 3
*/

/* Part 1: No page replacement - FIFO-based TLB update */


#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *arg[]){

    // Check command line argument:
    if (argc != 2){
        printf("Usage: ./main1 addresses.txt\n");
        return 1;
    }

    // Open the file:
    FILE *addresses = fopen(arg[1], "r");
    if (addresses == NULL){
        printf("Error opening file.\n");
        return 1;
    }

    // Open backing store:
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL){
        printf("Error opening backing store.\n");
        return 1;
    }

    // Read logical addresses:
    int logical_address;

    while (fscanf(addresses, "%d", &logical_address) != EOF) {

        int page = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        printf("logical=%d page=%d offset=%d\n", logical_address, page, offset);
    }

    // Close files:
    fclose(addresses);
    fclose(backing_store);
    return 0;
}







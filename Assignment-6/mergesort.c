#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "goodmalloc.h"

int blocks_in_use = 0;
int total_blocks;

void merge(char *arr, int l, int m, int r, int free)
{
    // int i, j, k;
    // int n1 = m - l + 1;
    // int n2 = r - m;
    createList("i", 1, 0);
    createList("j", 1, 0);
    createList("k", 1, 0);
    createList("n1", 1, m-l+1);
    createList("n2", 1, r-m);


 
    // Create temp arrays
    // int L[n1], R[n2];
    createList("L", getVal("n1", 0), 0);
    createList("R", getVal("n2", 0), 0);

    
 
    // Copy data to temp arrays
    // L[] and R[]
    for (assignVal("i", 0, 0); getVal("i", 0) < getVal("n1", 0); assignVal("i", 0, getVal("i", 0)+1))
        // L[i] = arr[l + i];
        assignVal("L", getVal("i", 0), getVal(arr, l + getVal("i", 0)));
    // for (j = 0; j < n2; j++)
    //     R[j] = arr[m + 1 + j];

    for (assignVal("j", 0, 0); getVal("j", 0) < getVal("n2", 0); assignVal("j", 0, getVal("j", 0)+1))
        // L[i] = arr[l + i];
        assignVal("R", getVal("j", 0), getVal(arr, m + 1 + getVal("j", 0)));
 
    // Merge the temp arrays back
    // into arr[l..r]
    // Initial index of first subarray
    assignVal("i", 0 , 0);
 
    // Initial index of second subarray
    assignVal("j", 0 , 0);
    
 
    // Initial index of merged subarray
    assignVal("k", 0 , l);
    // getVal("i", 0)
    while (getVal("i", 0) < getVal("n1", 0) && getVal("j", 0) < getVal("n2", 0))
    {
        if (getVal("L", getVal("i",0)) <= getVal("R", getVal("j",0)))
        {
            // arr[k] = L[i];
            assignVal(arr, getVal("k", 0), getVal("L", getVal("i",0)));
            // i++;
            assignVal("i", 0, getVal("i", 0)+1);
        }
        else
        {
            // arr[k] = R[j];
            // j++;
            assignVal(arr, getVal("k", 0), getVal("R", getVal("j",0)));
            // i++;
            assignVal("j", 0, getVal("j", 0)+1);
        }
        assignVal("k", 0, getVal("k", 0)+1);
    }
 
    // Copy the remaining elements
    // of L[], if there are any
    while (getVal("i", 0) < getVal("n1", 0)) {
        assignVal(arr, getVal("k", 0), getVal("L", getVal("i",0)));
        assignVal("i", 0, getVal("i", 0)+1);
        assignVal("k", 0, getVal("k", 0)+1);
    }
 
    // Copy the remaining elements of
    // R[], if there are any
    while (getVal("j", 0) < getVal("n2", 0))
    {
        assignVal(arr, getVal("k", 0), getVal("R", getVal("j",0)));
        assignVal("j", 0, getVal("j", 0)+1);
        assignVal("k", 0, getVal("k", 0)+1);
    }

    int bl_use = total_blocks - getCurrentFreeBlocks();
    if(bl_use > blocks_in_use) blocks_in_use = bl_use;

    if(free){
        freeElem("L");
        freeElem("R");
    }

    freeElem("i");
    freeElem("j");
    freeElem("k");
    freeElem("n1");
    freeElem("n2");
}


void mergeSort(char *arr, int l, int r, int free)
{
    if (l < r)
    {
        // int m = l + (r - l) / 2;
        createList("m", 1, l + (r - l) / 2);
        // Sort first and second halves
        mergeSort(arr, l, getVal("m", 0), free);
        mergeSort(arr, getVal("m", 0) + 1, r, free);
 
        merge(arr, l, getVal("m", 0), r, free);
        freeElem("m");
    }
}

int main(int argc, char **argv){

    // this size is in bytes
    srand(time(0));

    int size = 10;
    if(argc > 1){
        size = atoi(argv[1]);
    }

    // printf("Sizeof  stackData = %ld and sizeof stackentries = %ld and sizeof pagetable entries = %ld\n", sizeof(Stack), sizeof(stackNode), sizeof(pageTableEntry));
    total_blocks = createMem(size);
    
    int memsize = 50000;

    createList("arr", memsize, 0);
    printPageTable("arr");

    for (int i = 0; i < memsize; i++)
    {
        assignVal("arr", i, random()%100000 + 1);
    }
    
    if(memsize <= 50) printList("arr");

    mergeSort("arr", 0, memsize-1, 0);

    if(memsize <= 50) printList("arr");

    float percent = ((float)(blocks_in_use)/(float)(total_blocks))* 100.0;
    printf("Maximum blocks used by program = %d (%f)\%\n", blocks_in_use, percent);
    return 0;
}
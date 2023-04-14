#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<unistd.h>  
#include<sys/shm.h>  
#include <string.h>
#include "goodmalloc.h"

int blocks_in_use = 0;
int total_blocks;
float time_elapsed;

clock_t startm, stopm;
#define START                          \
    if ((startm = clock()) == -1)      \
    {                                  \
        printf("Error calling clock"); \
        exit(1);                       \
    }
#define STOP                           \
    if ((stopm = clock()) == -1)       \
    {                                  \
        printf("Error calling clock"); \
        exit(1);                       \
    }
#define PRINTTIME                                                   \
    time_elapsed = ((double)(stopm - startm) / CLOCKS_PER_SEC);       \
    printf("%6.3f seconds.\n", (time_elapsed));                      \


void merge(char *arr, int l, int m, int r, int free)
{
    // int i, j, k;
    // int n1 = m - l + 1;
    // int n2 = r - m;
    createList("i", 1, 0);
    createList("j", 1, 0);
    createList("k", 1, 0);
    createList("n1", 1, m - l + 1);
    createList("n2", 1, r - m);

    // Create temp arrays
    // int L[n1], R[n2];
    createList("L", getVal("n1", 0), 0);
    createList("R", getVal("n2", 0), 0);

    // comment it if too large
    printPageTable("L");
    printPageTable("R");

    // Copy data to temp arrays
    // L[] and R[]
    for (assignVal("i", 0, 0); getVal("i", 0) < getVal("n1", 0); assignVal("i", 0, getVal("i", 0) + 1))
        // L[i] = arr[l + i];
        assignVal("L", getVal("i", 0), getVal(arr, l + getVal("i", 0)));
    // for (j = 0; j < n2; j++)
    //     R[j] = arr[m + 1 + j];

    for (assignVal("j", 0, 0); getVal("j", 0) < getVal("n2", 0); assignVal("j", 0, getVal("j", 0) + 1))
        // L[i] = arr[l + i];
        assignVal("R", getVal("j", 0), getVal(arr, m + 1 + getVal("j", 0)));

    // Merge the temp arrays back
    // into arr[l..r]
    // Initial index of first subarray
    assignVal("i", 0, 0);

    // Initial index of second subarray
    assignVal("j", 0, 0);

    // Initial index of merged subarray
    assignVal("k", 0, l);
    // getVal("i", 0)
    while (getVal("i", 0) < getVal("n1", 0) && getVal("j", 0) < getVal("n2", 0))
    {
        if (getVal("L", getVal("i", 0)) <= getVal("R", getVal("j", 0)))
        {
            // arr[k] = L[i];
            assignVal(arr, getVal("k", 0), getVal("L", getVal("i", 0)));
            // i++;
            assignVal("i", 0, getVal("i", 0) + 1);
        }
        else
        {
            // arr[k] = R[j];
            // j++;
            assignVal(arr, getVal("k", 0), getVal("R", getVal("j", 0)));
            // i++;
            assignVal("j", 0, getVal("j", 0) + 1);
        }
        assignVal("k", 0, getVal("k", 0) + 1);
    }

    // Copy the remaining elements
    // of L[], if there are any
    while (getVal("i", 0) < getVal("n1", 0))
    {
        assignVal(arr, getVal("k", 0), getVal("L", getVal("i", 0)));
        assignVal("i", 0, getVal("i", 0) + 1);
        assignVal("k", 0, getVal("k", 0) + 1);
    }

    // Copy the remaining elements of
    // R[], if there are any
    while (getVal("j", 0) < getVal("n2", 0))
    {
        assignVal(arr, getVal("k", 0), getVal("R", getVal("j", 0)));
        assignVal("j", 0, getVal("j", 0) + 1);
        assignVal("k", 0, getVal("k", 0) + 1);
    }

    int bl_use = total_blocks - getCurrentFreeBlocks();
    if (bl_use > blocks_in_use)
        blocks_in_use = bl_use;

    if (free)
    {
        freeElem("L");
        freeElem("R");
    }

    freeElem("i");
    freeElem("j");
    freeElem("k");
    freeElem("n1");
    freeElem("n2");
}

void fastmerge(char *arr, int l, int m, int r, int free)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temp arrays
    // int L[n1], R[n2];
    createList("L", n1, 0);
    createList("R", n2, 0);

    // comment it if too large
    printPageTable("L");
    printPageTable("R");

    // Copy data to temp arrays
    // L[] and R[]
    for (i = 0; i < n1; i++)
        // L[i] = arr[l + i];
        assignVal("L", i, getVal(arr, l + i));
    for (j = 0; j < n2; j++)
        assignVal("R", j, getVal(arr, m + 1+ j));

    // Merge the temp arrays back
    // into arr[l..r]
    // Initial index of first subarray
    i=0;

    // Initial index of second subarray
    j=0;

    // Initial index of merged subarray
    k = l;
    // getVal("i", 0)
    while (i< n1 && j < n2)
    {
        if (getVal("L", i) <= getVal("R",j))
        {
            // arr[k] = L[i];
            assignVal(arr, k, getVal("L", i));
            i++;
            // assignVal("i", 0, getVal("i", 0) + 1);
        }
        else
        {
            // arr[k] = R[j];
            // j++;
            assignVal(arr, k, getVal("R", j));
            j++;
            // assignVal("j", 0, getVal("j", 0) + 1);
        }
        k++;
    }

    // Copy the remaining elements
    // of L[], if there are any
    while (i < n1)
    {
        assignVal(arr, k, getVal("L", i));
        // assignVal("i", 0, getVal("i", 0) + 1);
        i++;
        k++;
        // assignVal("k", 0, getVal("k", 0) + 1);
    }

    // Copy the remaining elements of
    // R[], if there are any
    while (j < n2)
    {
        assignVal(arr, k, getVal("R",j));
        j++;
        k++;
        // assignVal("j", 0, getVal("j", 0) + 1);
        // assignVal("k", 0, getVal("k", 0) + 1);
    }

    int bl_use = total_blocks - getCurrentFreeBlocks();
    if (bl_use > blocks_in_use)
        blocks_in_use = bl_use;

    if (free)
    {
        freeElem("L");
        freeElem("R");
    }
}

void mergeSort(char *arr, int l, int r, int free)
{
    if (l < r)
    {

        createList("m", 1, l + (r - l) / 2);
        // int m =  l + (r - l) / 2;
        mergeSort(arr, l, getVal("m", 0), free);
        mergeSort(arr, getVal("m", 0) + 1, r, free);

        merge(arr, l, getVal("m", 0), r, free);
        freeElem("m");
    }
}

void fastmergeSort(char *arr, int l, int r, int free)
{
    if (l < r)
    {
        int m =  l + (r - l) / 2;
        fastmergeSort(arr, l, m, free);
        fastmergeSort(arr, m + 1, r, free);

        fastmerge(arr, l, m, r, free);
    }
}

int main(int argc, char **argv)
{

    // this size is in bytes
    srand(time(0));

    int size = 10;
    if (argc > 1)
    {
        size = atoi(argv[1]);
    }

    // printf("Sizeof  stackData = %ld and sizeof stackentries = %ld and sizeof pagetable entries = %ld\n", sizeof(Stack), sizeof(stackNode), sizeof(pageTableEntry));
    total_blocks = createMem(size);

    
    // Test input
    /*
    createList("a", 4, 0);
    createList("b", 2, 0);
    createList("c", 4, 0);
    freeElem("b");
    createList("d", 4, 0);
    printEntirePageTable();
    freeElem("a");
    reallocList("d", 8);
    printPageTable("d");
    */
    

    
    int memsize = 50000;
    int iteration = 5;
    int free = 1;

    createList("arr", memsize, 0);
    printPageTable("arr");

    // // for (int i = 0; i < iteration; i++)
    // // {
    for (int i = 0; i < memsize; i++)
    {
        assignVal("arr", i, random() % 100000 + 1);
    }

    if (memsize <= 20)
        printList("arr");

    START
    fastmergeSort("arr", 0, memsize - 1, free);
    STOP

    if (memsize <= 20) printList("arr");

    printf("Time taken by program : ");
    PRINTTIME;

    void *shared_mem;
    int shmid;

    key_t key = 2345;
    shmid = shmget(key, 20, 0666 | IPC_CREAT);
    shared_mem = shmat(shmid,NULL,0); 
    char buf[20];
    gcvt(time_elapsed, 6, buf);
    strcpy(shared_mem,buf); 

    float percent = (float)blocks_in_use / (float)total_blocks * 100.0;
    if (free)
        printf("Maximum blocks used after freeing = %d (%f percent)\n", blocks_in_use, percent);
    else
        printf("Maximum blocks used without freeing = %d (%f percent)\n", blocks_in_use, percent);
    printf("\n");
    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "goodmalloc.h"

int num_mem_blocks = 0;

int computeHash(char *name){
    int hash = 5381;
    int c;

    while (c = *name++)
        hash = (((hash << 5) + hash) + c)%HASH_TABLE_SIZE; 

    return hash;
}

void free_stack_node(stackNode *node){
    pthread_mutex_lock(&stackMemLock);
    node->size = 0;
    node->next = stackData->freehead;
    stackData->freehead = node;
    pthread_mutex_unlock(&stackMemLock);
}

void free_ptable_node(pageTableEntry *node){

    pthread_mutex_lock(&pageTableMemLock);
    node->isallocated = 0;
    node->next = tableData->freehead;
    tableData->freehead = node;
    pthread_mutex_unlock(&pageTableMemLock);

}

stackNode *get_empty_entry_for_node(){
    
    pthread_mutex_lock(&stackMemLock);

    stackNode *head = stackData->freehead;
    if(!head){
        pthread_mutex_unlock(&stackMemLock);
        return head;
    }
    stackData->freehead = head->next;
    pthread_mutex_unlock(&stackMemLock);

    return head;
}

// insert stack Node in stack
// we pop the stack top
void insert_in_hash_table(char *name, size_t size, pageTableEntry *ptr){

    int index = computeHash(name);
    stackNode *node = get_empty_entry_for_node();

    if(!node){
        perror("CreateList Error : No space for stack nodes\n");
        exit(0);
    }
    strcpy(node->listName, name);
    node->pTable = ptr;
    node->size = size;

    // putting this node in the hashtable at index
    // impose hash table lock
    pthread_mutex_lock(&hashTableLock[index]);
    if(!hashTable[index]){
        hashTable[index] = node;
        node->next = NULL;

        pthread_mutex_unlock(&hashTableLock[index]);

        return;
    }

    node->next = hashTable[index];
    hashTable[index] = node;

    pthread_mutex_unlock(&hashTableLock[index]);

}

// delete stack Node from stack
stackNode *delete_from_hash_table(char *name){
    int index = computeHash(name);
    stackNode *ptr = hashTable[index];
    stackNode *prev = NULL;

    pthread_mutex_lock(&hashTableLock[index]);
    
    while(ptr){
        if(strcmp(ptr->listName, name) == 0){
            // delete this
            if(!prev){
                // first node to be deleted
                hashTable[index] = ptr->next;
            }else{
                prev->next = ptr->next;
            }

            // ptr->size = 0; do size = 0 after extracting the page table address
            pthread_mutex_unlock(&hashTableLock[index]);

            return ptr;
        }
        prev = ptr;
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&hashTableLock[index]);


    return NULL;
}



int createMem(size_t size){

    int maxBlocks = (int)(size)/(int)(sizeof(node));
    size_t stackSize = maxBlocks*sizeof(stackNode);
    size_t ptableszie = maxBlocks*sizeof(pageTableEntry);

    // demanding the memory
    ptr = malloc(sizeof(Stack) + size + sizeof(freeListInfo) + stackSize + ptableszie);

    // initialize hashtable and mutex locks
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hashTable[i] = NULL;
        pthread_mutex_init(&hashTableLock[i], NULL);
    }
    
    pthread_mutex_init(&pageTableMemLock, NULL);
    pthread_mutex_init(&stackMemLock, NULL);
    pthread_mutex_init(&freeListLock, NULL);
    
    // Assigning pointers per the position
    // void *ptr;
    // Stack *stackData;
    // stackNode *stack_entry_start;
    // pageTableEntry *ptable_entry_start;
    // freeListInfo *freeList;
    // node *MemStart;

    // defining pointers
    stackData = (Stack *)ptr;
    stack_entry_start = (stackNode *)(stackData + 1);
    tableData = (Table *)(stack_entry_start + maxBlocks);
    ptable_entry_start = (pageTableEntry *)(tableData + 1);
    freeList = (freeListInfo *)(ptable_entry_start + maxBlocks);
    MemStart = (node *)(freeList + 1);


    stackNode *stackstart = stack_entry_start;
    while(stackstart + 1 < (stackNode *)tableData){
        if(stackstart + 2 < (stackNode *)tableData){
            stackstart->next = stackstart + 1;
        }else{
            stackstart->next = NULL;
            break;
        }

        stackstart = stackstart->next;
    }
    stackData->freehead = stack_entry_start;

   
    pageTableEntry *pagestart = ptable_entry_start;

    while(pagestart + 1 < (pageTableEntry *)freeList){
        if(pagestart + 2 < (pageTableEntry *)freeList){
            pagestart->next = pagestart + 1;
        }else{
            pagestart->next = NULL;
            break;
        }

        pagestart = pagestart->next;
    }
    tableData->freehead = ptable_entry_start;


    // filling entry
    

    // populating the memory with structures
    node *temp = MemStart;
    freeList->head = temp;
    node *prev = NULL;
    node *next = NULL;

    while(temp + 1 <= MemStart + maxBlocks){
        // this node can be allocated
        // cnt is the pointer to the node
        node *cnt = temp;
        cnt->val = 0;


        if(num_mem_blocks == 0){
            cnt->prev = NULL;
        }else{
            cnt->prev = prev;
        }

        if(temp + 2 <= MemStart + maxBlocks){
            cnt->next = temp + 1;
        }else{
            cnt->next = NULL;
        }

        prev = cnt;
        num_mem_blocks++;
        temp = temp + 1;
    }

    // updating the freeList node 
    freeList->size = num_mem_blocks;
    if(!prev){
        perror("Why it is null\n");
        exit(0);
    }
    freeList->tail = prev;


    if(num_mem_blocks == 0){
        freeList->head = NULL;
        perror("Please specify larger memory\n");
        free(ptr);
        return num_mem_blocks;
    }

    printf("Memory to accomodate %d blocks allocated\n", num_mem_blocks);
    return num_mem_blocks;
}

void freeListIteration(freeListInfo *list){
    node *head = list->head;
    int i = 0;
    while(head){
        i++;
        printf("This is free node %d\n", i);
        head = head->next;
    }
}

pageTableEntry *get_empty_entry_for_pt(){
    
    pthread_mutex_lock(&pageTableMemLock);

    pageTableEntry *head = tableData->freehead;
    if(!head){
        pthread_mutex_unlock(&pageTableMemLock);
        return head;
    }
    tableData->freehead = head->next;
    pthread_mutex_unlock(&pageTableMemLock);

    return head;
}



// page table for the list will be created here
// also update the number of free blocks
void createList(char *name, size_t size, int val){
    // get the free memory of size t and return the first pointer
    if(size > freeList->size){
        perror("Not enough space in memory\n");
        exit(0);
    }

    node *start = freeList->head;
    node *prev = start;

    pageTableEntry *firstEntry = get_empty_entry_for_pt();
    if(!firstEntry){
        perror("No space for page table entries\n");
        exit(0);
    }

    pageTableEntry *prevEntry = firstEntry;
    firstEntry->start = 0;
    firstEntry->startNode = start;

    int index = 0;

    // here we have a problem
    while(index < size){
        if(index  && (prev+1 != start)){
            // make a new page table entry

            pageTableEntry *newEntry = get_empty_entry_for_pt();
            if(!newEntry){
                perror("No space for page table entries\n");
                exit(0);
            }

            // fill up the previous entry
            prevEntry->end = index-1;
            prevEntry->next = newEntry;
            
            // configure the current entry
            newEntry->start = index;
            newEntry->next = NULL;
            newEntry->startNode = start;
            prevEntry = newEntry;

        }

        prev = start;
        start->val = val;
        start = start->next;
        index++;
    }
    // configure the page table nodes
    prevEntry->end = index-1;
    prevEntry->next = NULL;

    // prev is the last pointer of the list
    // start is just next pointer in the free list
    prev->next = NULL;

    pthread_mutex_lock(&freeListLock);
    freeList->head = start;
    freeList->size -= size;
    if(freeList->size == 0) freeList->tail = NULL;

    pthread_mutex_unlock(&freeListLock);



    // push this into hashtable with the pointer to the page table entry
    insert_in_hash_table(name,size,firstEntry);
}

pageTableEntry* getPageTableHead(char *name, int *s){

    int index = computeHash(name);
    stackNode *snode = hashTable[index];
    pageTableEntry *head = NULL;
    int size;

    while(snode){
        if(strcmp(snode->listName, name) == 0){
            head = snode->pTable;
            size = snode->size;
            break;
        }

        snode = snode->next;
    }

    // checking
    *s = size;
    return head;
}

int getVal(char *name, int offset){
    // search the name in the stack
    int size;
    pageTableEntry *head = getPageTableHead(name, &size);

    if(!head){
        perror("No such list in getVal\n");
        exit(0);
    }

    if(offset > size-1){
        perror("Invalid Offset in getVal\n");
        exit(0);
    }

    int value = 0;
    while(head){
        if(offset >= head->start && offset <= head->end){
            // offset in this node
            node *temp = head->startNode;
            temp = temp + (offset - head->start);
            value = temp->val;
            break;
        }

        head = head->next;
    }

    return value;
}

void assignVal(char *name, int offset, int val){
    // get the page table entry
    int size;
    pageTableEntry *head = getPageTableHead(name, &size);

    // checking
    if(!head){
        perror("No such list\n");
        exit(0);
    }

    if(offset > size-1){
        perror("Invalid offset\n");
        exit(0);
    }

    // get the address from the page table linked list

    pthread_mutex_lock(&pageTableMemLock);
    while(head){
        if(offset >= head->start && offset <= head->end){
            // offset in this node
            node *temp = head->startNode;
            temp = temp + (offset - head->start);

            // update the value
            temp->val = val;
            break;
        }

        head = head->next;
    }

    pthread_mutex_unlock(&pageTableMemLock);


}

// also update the number of free blocks
void freeElem(char *name){
    // get the stack node and delete it
    
    stackNode *deleted_node = delete_from_hash_table(name);
    pageTableEntry *head = NULL;
    if(!deleted_node){
        perror("No such list\n");
        exit(0);
    }
    
    int size = deleted_node->size;
    head = deleted_node->pTable;
    free_stack_node(deleted_node);

    // get the ptable entries, get lsat node pointer and free them
    // maybe use a lock
    node *startnode = head->startNode;
    node *endnode;
    while(head){
        if(head->next == NULL){
            endnode = head->startNode + (head->end - head->start);
        }
        pageTableEntry *temp = head->next;
        free_ptable_node(head);
        head = temp;
    }

    // from startnode and endnode, attach this list to the free list
    pthread_mutex_lock(&freeListLock);

    if(freeList->size == 0){
        freeList->head = startnode;
        startnode->prev = NULL;
    }
    else{
        freeList->tail->next = startnode;
        startnode->prev = freeList->tail;
    }
    
    freeList->tail = endnode;
    endnode->next = NULL;
    // update the size of the free list
    freeList->size += size;

    pthread_mutex_unlock(&freeListLock);

}

void reallocList(char *name, size_t newsize){
    int index = computeHash(name);
    stackNode *snode = hashTable[index];
    int oldsize = snode->size;

    if(newsize == 0){
        freeElem(name);
        return;
    }

    if(newsize == oldsize) return;

    pthread_mutex_lock(&freeListLock);

    if(newsize > oldsize){
        if(newsize - oldsize > freeList->size){
            perror("Realloc failed : No space in memory\n");
            exit(0);
        }

        int new_blocks = newsize - oldsize;
        node *freehead = freeList->head;
        node *freetail = freeList->tail;

        pageTableEntry *head = snode->pTable;
        snode->size = newsize;

        pageTableEntry *prev = head;
        while(head){
            prev = head;
            head = head->next;
        }

        // prev is the last pointer of the page table
        pageTableEntry *firstEntry;
        node *endnode = (prev->startNode + prev->end - prev->start);
        int last_index = prev->end;

        // only the end of this page table entry will change
        if(endnode +1 == freehead) firstEntry = prev;
        else{

            // configuring first entry struct
            firstEntry = get_empty_entry_for_pt();
            firstEntry->start = last_index+1;
            firstEntry->startNode = freehead;
            prev->next = firstEntry;
        }

        pageTableEntry *prevEntry = firstEntry;

        int index = 0;
        node *prevptr = freehead;

        // here we have a problem
        while(index < new_blocks){
            if(index  && (prevptr + 1 != freehead)){
                // make a new page table entry

                pageTableEntry *newEntry = get_empty_entry_for_pt();
                if(!newEntry){
                    perror("Realloc Error : No space for page table entries\n");
                    exit(0);
                }

                // fill up the previous entry
                prevEntry->end = last_index + index;
                prevEntry->next = newEntry;
                
                // configure the current entry
                newEntry->start = last_index + 1+ index;
                newEntry->next = NULL;
                newEntry->startNode = freehead;
                prevEntry = newEntry;

            }

            prevptr = freehead;
            freehead->val = 0;
            freehead = freehead->next;
            index++;
        }
        // configure the page table nodes
        prevEntry->end = last_index + index;
        prevEntry->next = NULL;

        // prev is the last pointer of the list
        // start is just next pointer in the free list
        prevptr->next = NULL;

        freeList->head = freehead;
        freeList->size -= new_blocks;
        if(freeList->size == 0) freeList->tail = NULL;
    }

    if(newsize < oldsize){
        pageTableEntry *head = snode->pTable;
        int num_blocks = 0;
        snode->size = newsize;
        while(head){
            num_blocks += head->end - head->start + 1;

            if(num_blocks >= newsize){
                // free all the nodes from head->next
                pageTableEntry *temp = head->next;
                head->next = NULL;

                // free nodes frmo temp to last
                while(temp){
                    pageTableEntry *nextnode = temp->next;
                    free_ptable_node(temp);

                    // free the corresponding nodes from the memory
                    int block_size = temp->end - temp->start + 1;
                    node *startnode = temp->startNode;
                    node *endnode = startnode + temp->end - temp->start;

                    // attach these nodes in the free list and update the size
                    if(freeList->size == 0){
                        freeList->head = startnode;
                        startnode->prev = NULL;
                    }
                    else{
                        freeList->tail->next = startnode;
                        startnode->prev = freeList->tail;
                    }
                    
                    endnode->next = NULL;
                    freeList->tail = endnode;
                    // update the size of the free list
                    freeList->size += block_size;


                    temp = nextnode;
                }

                // update the page table and free remaining nodes from the current ptable
                // currently at head, updating the list
                node *startnode = head->startNode;
                node *endnode = startnode + head->end - head->start;
                int block_size = head->end - head->start + 1;
                int end_index = head->end - (num_blocks - newsize);

                int num_nodes_to_keep = block_size - (num_blocks - newsize);
                node *lastnode = startnode + num_nodes_to_keep - 1;
                node *firstremove = lastnode->next;

                lastnode->next = NULL;

                // as nodes from other block have already been freed
                // no need to do it again
                if(num_blocks == newsize) break;

                // remove from firstremove to endnode
                // firstremove will never be NULL

                if(freeList->size == 0){
                    freeList->head = firstremove;
                    firstremove->prev = NULL;
                }
                else{
                    freeList->tail->next = firstremove;
                    firstremove->prev = freeList->tail;
                }
                    
                endnode->next = NULL;
                freeList->tail = endnode;
                // update the size of the free list
                freeList->size += num_blocks - newsize;
                head->next = NULL;

                // updating the current block pointers
                head->end = end_index;
                break;
            }

            head = head->next;
            
        }
    }

    pthread_mutex_unlock(&freeListLock);


}

// make this function better aligned
void printPageTable(char *name){
    int size;
    pageTableEntry *head = getPageTableHead(name, &size);
    
    if(!head){
        perror("No such list\n");
        exit(0);
    }

    printf("                      PAGE TABLE entries for %s\n", name);
    printf("+--------------------------------------------------------------------+\n");
    printf("|    %-25s%-25s%-10s    |\n", "Start Index", "End Index", "Pointer"); 
    printf("+--------------------------------------------------------------------+\n");


    // printf("Start Index            End Index            Pointer\n");
    while(head){    
        printf("|    %-25d%-20d%-10p     |\n", head->start, head->end, head->startNode);
        head = head->next;
    }
    printf("+--------------------------------------------------------------------+\n");

}

void printList(char *name){
    int size;
    pageTableEntry *head = getPageTableHead(name, &size);
    node *startnode = head->startNode;
    // checking
    if(!head){
        perror("No such list\n");
        exit(0);
    }

    printf("%s : ", name);
    while(startnode){
        if(startnode->next) printf("%d - ", startnode->val);
        else printf("%d\n", startnode->val);
        startnode = startnode->next;
    }
}

int getCurrentFreeBlocks(){
    return freeList->size;
}

void printEntirePageTable(){
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        if(hashTable[i]){
            stackNode *ptr = hashTable[i];
            while(ptr){
                printPageTable(ptr->listName);
                ptr = ptr->next;
            }
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 1000


typedef struct node{
    int val;
    struct node *next;
    struct node *prev;
}node;

typedef struct freeListInfo{
    size_t size;
    struct node *head;
    struct node *tail;
}freeListInfo;


typedef struct pageTableEntry{
    // start and end continuous offsets
    int start;
    int end;
    // which start at this node
    struct node *startNode;
    // struct node *endNode;
    int isallocated;
    struct pageTableEntry *next;
}pageTableEntry;

// we create a linked list of these stackNode
typedef struct stackNode{
    char listName[100];
    size_t size;
    struct pageTableEntry *pTable;
    struct stackNode *next;
}stackNode;

typedef struct Stack{
    stackNode *top;
    stackNode *bottom;
    size_t size;
}Stack;

void *ptr;
Stack *stackData;
stackNode *stack_entry_start;
pageTableEntry *ptable_entry_start;
freeListInfo *freeList;
node *MemStart;
int num_mem_blocks = 0;

// hashtable implementation
// one lock for every entry of the hash table
stackNode *hashTable[HASH_TABLE_SIZE];

int computeHash(char *name){
    int hash = 5381;
    int c;

    while (c = *name++)
        hash = (((hash << 5) + hash) + c)%HASH_TABLE_SIZE; 

    return hash;
}

stackNode *get_empty_entry_for_node(){
    stackNode *start = stack_entry_start;

    while(start + 1 < (stackNode *)ptable_entry_start){
        // if size is zero, then the block is unallocated
        if(start->size == 0){
            return start;
        }

        start = start+1;
    }

    return NULL;
}

// insert stack Node in stack
// we pop the stack top
void insert_in_hash_table(char *name, size_t size, pageTableEntry *ptr){

    int index = computeHash(name);
    stackNode *node = get_empty_entry_for_node();

    if(!node){
        perror("No space for stack nodes\n");
        exit(0);
    }
    strcpy(node->listName, name);
    node->pTable = ptr;
    node->size = size;

    // putting this node in the hashtable at index
    if(!hashTable[index]){
        hashTable[index] = node;
        node->next = NULL;
        return;
    }

    node->next = hashTable[index];
    hashTable[index] = node;

    // if(stackData->size == 0){
    //     stackData->top = stack_entry_start;
    //     stackData->bottom = stack_entry_start;
    //     strcpy(stack_entry_start->listName, name);
    //     stack_entry_start->pTable = ptr;
    //     stack_entry_start->size = size;
    //     stack_entry_start->next = NULL;
    // }else{
    //     stackNode *temp = stackData->top;
    //     temp = temp + 1;

    //     strcpy(temp->listName, name);
    //     temp->pTable = ptr;
    //     temp->size = size;
    //     temp->next = stackData->top;
    //     stackData->top = temp;
    // }

    // stackData->size++;
}

// delete stack Node from stack
stackNode *delete_from_hash_table(char *name){
    int index = computeHash(name);
    stackNode *ptr = hashTable[index];
    stackNode *prev = NULL;
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
            return ptr;
        }
        prev = ptr;
        ptr = ptr->next;
    }

    return NULL;
}



void createMem(size_t size){

    int maxBlocks = (int)(size)/(int)(sizeof(node));
    size_t stackSize = maxBlocks*sizeof(stackNode);
    size_t ptableszie = maxBlocks*sizeof(pageTableEntry);

    // demanding the memory
    ptr = malloc(sizeof(Stack) + size + sizeof(freeListInfo) + stackSize + ptableszie);

    // initialize hashtable
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hashTable[i] = NULL;
    }
    
    
    // Assigning pointers per the position
    // void *ptr;
    // Stack *stackData;
    // stackNode *stack_entry_start;
    // pageTableEntry *ptable_entry_start;
    // freeListInfo *freeList;
    // node *MemStart;


    stackData = (Stack *)ptr;
    stackData->size = 0;

    stack_entry_start = (stackNode *)(stackData + 1);
    ptable_entry_start = (pageTableEntry *)(stack_entry_start + maxBlocks);

    freeList = (freeListInfo *)(ptable_entry_start + maxBlocks);

    MemStart = (node *)(freeList + 1);

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
        return;
    }

    printf("Memory to accomodate %d blocks allocated\n", num_mem_blocks);
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
    pageTableEntry *start = ptable_entry_start;

    while(start + 1 < (pageTableEntry *)freeList){
        if(start->isallocated == 0){
            start->isallocated = 1;
            return start;
        }

        start = start+1;
    }

    return NULL;
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
    freeList->head = start;
    freeList->size -= size;
    if(freeList->size == 0) freeList->tail = NULL;


    // push this into hashtable with the pointer to the page table entry
    insert_in_hash_table(name,size,firstEntry);
}

int getVal(char *name, int offset){
    // search the name in the stack
    int index = computeHash(name);
    stackNode *temp = hashTable[index];
    pageTableEntry *head = NULL;
    int size;
    while(temp){
        if(strcmp(temp->listName, name) == 0){
            head = temp->pTable;
            size = temp->size;
            break;
        }

        temp = temp->next;
    }

    if(!head){
        printf("No such list\n");
        return 0;
    }

    if(offset > size-1){
        printf("Invalid Offset\n");
        return 0;
    }

    // int cnt = offset;
    // while(cnt--){
    //     startNode = startNode->next;
    // }
    // search for the offset in the linked list
    int value = -10000;
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
    int index = computeHash(name);
    stackNode *snode = hashTable[index];
    // createList("adi", 4, 11);ble[index];
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
    if(!head){
        perror("No such list\n");
        exit(0);
    }

    if(offset > size-1){
        perror("Invalid offset\n");
        exit(0);
    }

    // get the address from the page table linked list
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
    deleted_node->size = 0;

    // get the ptable entries, get lsat node pointer and free them
    // maybe use a lock
    node *startnode = head->startNode;
    node *endnode;
    while(head){
        if(head->next == NULL){
            endnode = head->startNode + (head->end - head->start);
        }
        head->isallocated = 0;
        head = head->next;
    }

    // from startnode and endnode, attach this list to the free list
    if(freeList->size == 0){
        freeList->head = startnode;
        startnode->prev = NULL;
        freeList->tail = endnode;
        endnode->next = NULL;
    }
    else{
        freeList->tail->next = startnode;
        startnode->prev = freeList->tail;
        freeList->tail = endnode;
        endnode->next = NULL;
    }
   
    // update the size of the free list
    freeList->size += size;
}

int main(int argc, char **argv){

    // this size is in bytes
    int size = 10;
    if(argc > 1){
        size = atoi(argv[1]);
    }

    // printf("Sizeof  stackData = %ld and sizeof stackentries = %ld and sizeof pagetable entries = %ld\n", sizeof(Stack), sizeof(stackNode), sizeof(pageTableEntry));
    createMem(size);
    // freeListIteration(freeList);
    createList("wow", 4, 0);
    createList("aditya", 2, 30);
    createList("adi", 4, 11);
    createList("a", 2, 69);
    printf("value is %d\n", getVal("adi", 3));  // 11
    assignVal("aditya", 1, 23);
    printf("value is %d\n", getVal("aditya", 1));  // 23
    freeElem("aditya");
    createList("a", 4, 25);
    printf("value is %d\n", getVal("a", 2));

    freeElem("adi");
    createList("ab", 4, 25);
    assignVal("ab", 1, 501);
    printf("value is %d\n", getVal("ab", 1));

    return 0;
}
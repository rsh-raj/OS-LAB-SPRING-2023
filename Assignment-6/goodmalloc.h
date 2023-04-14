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

// hashtable implementation
// one lock for every entry of the hash table
stackNode *hashTable[HASH_TABLE_SIZE];

// initializing mutex locks
pthread_mutex_t hashTableLock[HASH_TABLE_SIZE];
pthread_mutex_t stackMemLock;
pthread_mutex_t pageTableMemLock;
pthread_mutex_t freeListLock;

int computeHash(char *);
stackNode *get_empty_entry_for_node();
void insert_in_hash_table(char *, size_t , pageTableEntry *);
stackNode *delete_from_hash_table(char *);
void createMem(size_t );
void freeListIteration(freeListInfo *);
pageTableEntry *get_empty_entry_for_pt();
void createList(char *, size_t , int);
pageTableEntry* getPageTableHead(char *, int *);
int getVal(char *, int);
void assignVal(char *, int , int);
void freeElem(char *);
void reallocList(char *, size_t);
void printPageTable(char *);
void printList(char *);




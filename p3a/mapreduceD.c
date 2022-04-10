#include "mapreduce.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hashmap.h"
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#ifndef NUM_REDUCERS
    #define PARTITIONS 1
#else
    #define PARTITIONS NUM_REDUCERS
#endif

// const int partitions = 100;

typedef struct mapTask {
    void (*map)(char*);
    char* filename;
} mapTask;

typedef struct reduceTask {
    char* (*get_next)(char*, int);
    void (*reduce)(char*, Getter, int);
    char* key;
    int part_num;
} reduceTask;

mapTask mQueue[1000];
reduceTask rQueue[1000];
int mapTaskCount = 0;
int reduceTaskCount = 0;
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;
pthread_mutex_t pLocks[PARTITIONS];

Partitioner pner = NULL;


void submitMapTask(struct mapTask task) {
    pthread_mutex_lock(&mutexQueue);
    mQueue[mapTaskCount] = task;
    mapTaskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void submitReduceTask(struct reduceTask task) {
    pthread_mutex_lock(&mutexQueue);
    rQueue[reduceTaskCount] = task;
    reduceTaskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void* startMapThread(void* args) {
    while(1) {
	// printf("Entering while\n");
        mapTask task;
	pthread_mutex_lock(&mutexQueue);
	if(mapTaskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
	}
	task = mQueue[0];
	if(task.filename == NULL) {
            // printf ("Exiting thread..\n");
            pthread_cond_signal(&condQueue);
	    break;
        }
	for(int i = 0; i < mapTaskCount; i++) {
            mQueue[i] = mQueue[i+1];
	}
	mapTaskCount--;
	pthread_mutex_unlock(&mutexQueue);
	task.map(task.filename);
    }
    pthread_mutex_unlock(&mutexQueue);
    return NULL;
}

void* startReduceThread(void* args) {
    while(1) {
        // printf("Entering While\n");
	reduceTask task;
        pthread_mutex_lock(&mutexQueue);
        if(reduceTaskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        task = rQueue[0];
        if(task.key == NULL)
            break;
        for(int i = 0; i < reduceTaskCount; i++) {
            rQueue[i] = rQueue[i+1];
        }
        reduceTaskCount--;
        pthread_mutex_unlock(&mutexQueue);
        pthread_mutex_lock(&pLocks[task.part_num]);
	task.reduce(task.key, task.get_next, task.part_num);
        pthread_mutex_unlock(&pLocks[task.part_num]);

    }
    pthread_mutex_unlock(&mutexQueue);
    return NULL;
}

struct node {
    char* key;
    char* val;
    struct node *next_key;
    struct node *next_val;
};

struct node* head = NULL;
struct node* current = NULL;
struct node* arr_part[PARTITIONS];

struct node* search(char* key, int partition) {
    head = arr_part[partition];
    if(head == NULL) {
        return NULL;
    }
    current = head;
    //navigate through list
    while(current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next_key;
    }           
    return NULL;
}

int add(char* key, char* val, int partition){
    struct node* result = search(key, partition);
    struct node* insert_val = (struct node*) malloc(sizeof(struct node));
    insert_val->val = strdup(val);
    insert_val->key = strdup(key);  
    if(result != NULL){
        insert_val->next_val = result->next_val;
        result->next_val = insert_val; 
	return 0;
    }
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp->key = strdup(key);
    temp->val = strdup(val);
    temp->next_val = NULL;
    if(arr_part[partition] == NULL) {
        arr_part[partition] = temp;
    }
    else { 
        temp->next_key = arr_part[partition];
        arr_part[partition] = temp;
    }
    return 0;
}

struct node* findPrev(char* key, int partition_number) {
    struct node* prev = NULL;
    struct node* curr = arr_part[partition_number];
    while(curr != NULL) {
	if(strcmp(curr->key,key) >= 0 ) {
	    return prev;
	} 
	prev = curr;
	curr = curr->next_key;
    }
    return prev;
}

char* Get(char *key, int partition_number) {
    struct node* result = search(key, partition_number);
    struct node* temp = NULL;
    struct node* prev = NULL;
    if(result == NULL){
	return NULL;
    } else {
	if(result->next_val != NULL) {
	    temp = result->next_val;
	    result->next_val = result->next_val->next_val;
            return temp->val;
	} else {
	    prev = findPrev(key, partition_number);
            if(prev == NULL){
	        arr_part[partition_number] = result->next_key;	    
	    } else {
                prev->next_key = result->next_key;
	    }
	    return result->val;
	}
    }
    return NULL;
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

// --------------------------------------------------------------------

// C code for linked list merged sort
// #include <stdio.h>
// #include <stdlib.h>
 
/* Link list node */
// struct node {
//     int data;
//     struct node* next;
// };
 
/* function prototypes */
struct node* SortedMerge(struct node* a, struct node* b);
void FrontBackSplit(struct node* source,
                    struct node** frontRef, struct node** backRef);
 
/* sorts the linked list by changing next pointers (not data) */
void MergeSort(struct node** headRef)
// void MergeSort()
{
    struct node* head = *headRef;
    struct node* a;
    struct node* b;
 
    /* Base case -- length 0 or 1 */
    if ((head == NULL) || (head->next_key == NULL)) {
        return;
    }
 
    /* Split head into 'a' and 'b' sublists */
    FrontBackSplit(head, &a, &b);
 
    /* Recursively sort the sublists */
    MergeSort(&a);
    MergeSort(&b);
 
    /* answer = merge the two sorted lists together */
    *headRef = SortedMerge(a, b);
}
 
/* See https:// www.geeksforgeeks.org/?p=3622 for details of this
function */
struct node* SortedMerge(struct node* a, struct node* b)
{
    struct node* result = NULL;
 
    /* Base cases */
    if (a == NULL)
        return (b);
    else if (b == NULL)
        return (a);
 
    /* Pick either a or b, and recur */
    if (strcmp(a->key , b->key) <= 0) {
        result = a;
        result->next_key = SortedMerge(a->next_key, b);
    }
    else {
        result = b;
        result->next_key = SortedMerge(a, b->next_key);
    }
    return (result);
}
 
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves,
    and return the two lists using the reference parameters.
    If the length is odd, the extra node should go in the front list.
    Uses the fast/slow pointer strategy. */
void FrontBackSplit(struct node* source,
                    struct node** frontRef, struct node** backRef)
{
    struct node* fast;
    struct node* slow;
    slow = source;
    fast = source->next_key;
 
    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL) {
        fast = fast->next_key;
        if (fast != NULL) {
            slow = slow->next_key;
            fast = fast->next_key;
        }
    }
 
    /* 'slow' is before the midpoint in the list, so split it in two
    at that point. */
    *frontRef = source;
    *backRef = slow->next_key;
    slow->next_key = NULL;
}
 
/* Function to print nodes in a given linked list */
void printList(struct node* node)
{
    while (node != NULL) {
        printf("%s ", node->key);
        node = node->next_key;
    }
}
 
/* Function to insert a node at the beginning of the linked list */
void push(struct node** head_ref, char* new_data)
{
    /* allocate node */
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
 
    /* put in the data */
    new_node->key = strdup(new_data);
 
    /* link the old list off the new node */
    new_node->next_key = (*head_ref);
 
    /* move the head to point to the new node */
    (*head_ref) = new_node;
}

// --------------------------------------------------------------------

/*
void bubbleSort() {
    struct node* current;
    struct node* next;
    struct node* prev = NULL;
    for(int i = 0; i < PARTITIONS; i++) {
        current = arr_part[i];
        for(int j = 0; current != NULL; j++) {
            next = current->next_key;
            for(int l = j + 1; next != NULL; l++) {
                if(strcmp(current->key, next->key) > 0) {
                    if(prev == NULL) {
                        current->next_key = next->next_key;
                        next->next_key = current;
                        arr_part[i] = next;
                    } else {
                        current->next_key = next->next_key;
                        next->next_key = current;
                        prev->next_key = next;
                    }
                }
                next = next->next_key;
            }
            prev = current;
            current = current->next_key;
        }
    }
}
*/

void printStruct() {
    struct node* currKey;
    struct node* currVal;
    for (int i = 0; i < PARTITIONS; i++) {
        currKey = arr_part[i];
        while(currKey != NULL) {
            currVal = currKey;
            do {
                printf("partNum: %d, ", i);
                printf("key: %s", currKey->key);
                printf(", value: %s ", currVal->val);
                currVal = currVal->next_val;
            } 
            while(currVal != NULL);
            currKey = currKey->next_key;
            printf("\n");
        }
    }
}

void printStructKey() {
    struct node* currKey;
//    struct node* currVal;
    for (int i = 0; i < PARTITIONS; i++) {
        currKey = arr_part[i];
        printf ("partition %d: ", i);
        while(currKey != NULL) {
            printf("key: %s, ", currKey->key);
            currKey = currKey->next_key;
        }
        printf("\n");
    }
}

void MR_Emit(char *key, char *value) {
    int part_num = 0;
    // part_num = MR_DefaultHashPartition(key, PARTITIONS);
    part_num = pner(key, PARTITIONS);
    pthread_mutex_lock(&pLocks[part_num]);
    add(key, value, part_num);
    pthread_mutex_unlock(&pLocks[part_num]);
}


void MR_Run(int argc, char *argv[], Mapper map, int num_mappers,
            Reducer reduce, int num_reducers, Partitioner partition) {
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    pthread_t mapT[num_mappers];
    pthread_t reduceT[num_reducers];

    mapTaskCount = 0;
    reduceTaskCount = 0;

    for(int i = 0; i < PARTITIONS; i++) {
        pthread_mutex_init(&pLocks[i], NULL);
    }
    // printf("MR_RUN B4 create\n");
    for(int i = 0; i < num_mappers; i ++) {
        if(pthread_create(&mapT[i], NULL, &startMapThread, NULL) != 0) {
            perror("Failed to create Thread.");
	}
    }

    pner = partition;

    // printf("MR_RUN after create\n");
    for(int i = 1; i < argc; i++) {
	mapTask task;
	task.map = map;
	task.filename = strdup(argv[i]);
	submitMapTask(task);
    }

    // printf("MR_RUN after submit 1\n");
   
    mapTask task;
    task.filename = NULL;
    submitMapTask(task);
    
    // printf("MR_RUN after submit 2\n");

    for(int i = 0; i < num_mappers; i++) {
        if(pthread_join(mapT[i], NULL) != 0) {
            perror("Failed to join Thread.");
        }
    }
    
    // printf("MR_RUN after mapper\n");

    // bubbleSort();
    for (int i = 0 ; i < PARTITIONS; i ++) {
        MergeSort(&arr_part[i]);
    }
    // printf("MR_RUN after sort\n");

    for(int i = 0; i < num_reducers; i++) {
        if(pthread_create(&reduceT[i], NULL, &startReduceThread, NULL) != 0) {
            perror("Failed to create Thread.");
        }
    }

    // printf("MR_RUN after reducer create\n");

    struct node* curr = NULL;
    for(int i = 0; i < PARTITIONS; i++) {
	curr = arr_part[i];
	while(curr != NULL) {
            reduceTask task;
	    task.reduce = reduce;
	    task.key = strdup(curr->key);
	    task.part_num = i;
	    task.get_next = Get;
	    submitReduceTask(task);
            curr = curr->next_key;
	}
    }

    reduceTask rtask;
    rtask.key = NULL;
    submitReduceTask(rtask);
    // printf("MR_RUN after reducer submit\n");
    
    for(int i = 0; i < num_reducers; i++) {
        if(pthread_join(reduceT[i], NULL) != 0) {
            perror("Failed to join Thread.");
        }
    }
    // printf("MR_RUN after reducer join\n");

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    for(int i = 0; i < PARTITIONS; i++) {
        pthread_mutex_destroy(&pLocks[i]);
    }
    //for (int i = 1; i < argc; i++) {
      //  (*map)(argv[i]);
    //}
    //bubbleSort();
    //struct node* curr = NULL;
    //for(int i = 0; i < PARTITIONS; i++) {
      //  curr = arr_part[i];
	//while(curr != NULL) {
          //  (*reduce)(curr->key, Get, i);
            //curr = curr->next_key;
//	}
  //  }
}


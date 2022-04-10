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
        mapTask task;
	pthread_mutex_lock(&mutexQueue);
	if(mapTaskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
	}
	task = mQueue[0];
	if(task.filename == NULL) {
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
	reduceTask task;
        pthread_mutex_lock(&mutexQueue);
        if(reduceTaskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }
        task = rQueue[0];
        if(task.key == NULL) {
	    break;
	}
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
    insert_val->next_key = NULL;
    insert_val->next_val = NULL;
    if(result != NULL){
        insert_val->next_val = result->next_val;
        result->next_val = insert_val; 
	return 0;
    }
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp->key = strdup(key);
    temp->val = strdup(val);
    temp->next_val = NULL;
    temp->next_key = NULL;
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

struct node* insertHelper(struct node* sortedHead, struct node* curr) {
    if(sortedHead == NULL || (strcmp(sortedHead->key, curr->key) > 0)) {
        curr->next_key = sortedHead;
	sortedHead = curr;
    } else {
        struct node* temp = sortedHead;
	while(temp->next_key != NULL && (strcmp(temp->next_key->key, curr->key) < 0)) {
	    temp = temp->next_key;
	}
	curr->next_key = temp->next_key;
	temp->next_key = curr;
    }
    return sortedHead;
}

void insertionSort() {
    for(int i = 0; i < PARTITIONS; i++) {
        struct node* sortedListHead = NULL;
        struct node* curr = arr_part[i];
        while(curr != NULL) {
	    struct node* next = curr->next_key;
            sortedListHead = insertHelper(sortedListHead, curr);
	    curr = next;
	}
        arr_part[i] = sortedListHead;
    }
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
    for(int i = 0; i < num_mappers; i ++) {
        if(pthread_create(&mapT[i], NULL, &startMapThread, NULL) != 0) {
            perror("Failed to create Thread.");
	}
    }
    pner = partition;
    for(int i = 1; i < argc; i++) {
	mapTask task;
	task.map = map;
	task.filename = strdup(argv[i]);
	submitMapTask(task);
    }

    mapTask task;
    task.filename = NULL;
    submitMapTask(task);
    
    for(int i = 0; i < num_mappers; i++) {
        if(pthread_join(mapT[i], NULL) != 0) {
            perror("Failed to join Thread.");
        }
    }
    insertionSort();
    for(int i = 0; i < num_reducers; i ++) {
        if(pthread_create(&reduceT[i], NULL, &startReduceThread, NULL) != 0) {
            perror("Failed to create Thread.");
        }
    }
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
    for(int i = 0; i < num_reducers; i++) {
        if(pthread_join(reduceT[i], NULL) != 0) {
            perror("Failed to join Thread.");
        }
    }
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    for(int i = 0; i < PARTITIONS; i++) {
        pthread_mutex_destroy(&pLocks[i]);
    }
}

void MR_Emit(char *key, char *value) {
    int part_num = 0;
    part_num = pner(key, PARTITIONS);
    pthread_mutex_lock(&pLocks[part_num]);
    add(key, value, part_num);
    pthread_mutex_unlock(&pLocks[part_num]);
}


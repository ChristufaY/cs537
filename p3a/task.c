#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define THREAD_NUM 4

typedef struct Task {
    void (*taskFunction)(int, int);
    int arg1, arg2;
} Task;


Task taskQueue[256];
int taskCount = 0;
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

void sumAndProduct() {
    int a = rand() % 100;
    int b = rand() % 100;
    int sum = a + b;
    int product = a * b;
    printf("Sum and Product of %d and %d is %d and %d respectively\n",
		    a, b, sum, product);
}

void sum(int a, int b) {
    int sum = a + b;
    printf("Sum of %d and %d is %d\n", a, b, sum);
}

void product(int a, int b) {
    int prod = a * b;
    printf("Product of %d and %d is %d\n", a, b, prod); 
}

void executeTask(struct Task* task){
    task->taskFunction(task->arg1, task->arg2);
    //int result = task->a + task->b;
    //printf("The sum of %d and %d is %d\n", task->a, task->b, result);
}

void submitTask(struct Task task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void* startThread(void* args) {
    while(1) {
        Task task;
	int found = 0;
	pthread_mutex_lock(&mutexQueue);
	if(taskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
	}
	task = taskQueue[0];
	if(task.arg1 == -1)
	    break;	
	for(int i = 0; i < taskCount; i++) {
            taskQueue[i] = taskQueue[i+1];
      	}
     	taskCount--;
	pthread_mutex_unlock(&mutexQueue);
	executeTask(&task);
    }
    pthread_mutex_unlock(&mutexQueue);
    return NULL;
}

int main(int argc, char* argv[]) {
    //Task t1;
    //t1.a = 5;
    //t1.b = 10;

    //executeTask(&t1);

    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    for(int i = 0; i < THREAD_NUM; i++) {
        if(pthread_create(&th[i], NULL, &startThread, NULL) != 0) {
            perror("Failed to create Thread.");
	}
    }

    srand(time(NULL));
    for(int i = 0; i < 100; i++) {
        Task t;
	t.taskFunction = i % 2 == 0 ? &sum : &product;
        t.arg1 = rand() % 100;
	t.arg2 = rand() % 100;
	submitTask(t);
    }
    
    Task t;
    t.arg1 = -1;
    submitTask(t);

    for(int i = 0; i < THREAD_NUM; i++) {
        if(pthread_join(th[i], NULL) != 0) {
            perror("Failed to join Thread.");
        }
    }
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node {
   char* key;
   char* value;
   struct node *next;
};

struct node* head = NULL;
struct node* current = NULL;

struct node* search(char* key) {
    if(head == NULL) {
        return NULL;
    }
    current = head;
    //navigate through list
    while(current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }           
    return NULL;
}

int add(char* key, char* value){
    struct node* result = search(key);
    if(result != NULL){
        result->value = value;
	return 0;
    }
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp->key = key;
    temp->value = value;
    temp->next = head;
    head = temp;
    return 0;
}
    
int delete(char* key) {
    current = head;
    struct node* previous = NULL;	
    if(head == NULL) {
       return 0;
    }
    while(current != NULL) {
	if(strcmp(current->key, key) == 0) {
            if(current == head) {
            head = current->next;
	    }
	    else {
	    previous->next = current->next;
	    }
	    return 0;
	}
	else {
            previous = current;
	    current = current->next;
	}
    }
    return 1;
}

void printall() {
    struct node* ptr = head;
    while (ptr != NULL) {
      printf("(%s,%s) \n",ptr->key,ptr->value);
      ptr = ptr->next;
   }
}
int main() {
    add("ll", "/usr/bin/ls");
    printall();
    add("abc", "def");
    printall();
    add("abc", "123");
    printall();
    delete("ll");
    printall();
    delete("1");
    printall();
    delete("abc");
    printall();
}

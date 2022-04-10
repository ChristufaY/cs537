// Copyright 2022 Christopher Yang
#define _FILE_OFFSET_BITS 64
#define _MAX_LENGTH 512
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
struct node {
    char* key;
    char* value;
    struct node *next;
};
struct node* head = NULL;
struct node* current = NULL;
struct node* search(char* key) {
    if (head == NULL) {
        return NULL;
    }
    current = head;
    // navigate through list
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
int add(char* key, char* value) {
    struct node* result = search(key);
    if (result != NULL) {
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
    if (head == NULL) {
       return 0;
    }
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (current == head) {
                head = current->next;
            } else {
                previous->next = current->next;
            }
            return 0;
        } else {
            previous = current;
            current = current->next;
        }
    }
    return 1;
}
void printall() {
    struct node* ptr = head;
    char msg[_MAX_LENGTH];
    while (ptr != NULL) {
        snprintf(msg, _MAX_LENGTH, "%s %s\n", ptr->key, ptr->value);
        write(STDOUT_FILENO, msg, strlen(msg));
        ptr = ptr->next;
    }
}
int main(int argc, char* argv[]) {
    char msg[_MAX_LENGTH];
    char input[_MAX_LENGTH];
    char newInput[900];
    char* token;
    char* toks[100];
    char** next = toks;
    char* delim = " 	";
    int pid = 0;
    int status = 0;
    char* temp;
    char* key;
    char* value;
    struct node* res;
    char* gTCheck;
    char* redirHalf;
    char* filename;
    if (argc > 2) {
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", strlen("Usage: mysh [batch-file]\n"));
        exit(1);
    } else if (argc == 1) {
        // interactive mode
        while (1) {
            snprintf(msg, _MAX_LENGTH, "mysh> ");
            write(STDOUT_FILENO, msg, strlen(msg));
                temp = fgets(input, _MAX_LENGTH, stdin);
                if (temp == NULL) {
                    exit(0);
                }
                input[strcspn(input, "\n")] = 0;
                next = toks;
            filename = NULL;
                gTCheck = strchr(input, '>');
            if (gTCheck != NULL) {
                    redirHalf = (gTCheck + 1);
            gTCheck = strchr(redirHalf, '>');
            if (gTCheck != NULL) {
                        snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                write(STDERR_FILENO, msg, strlen(msg));
                continue;
            }
            filename = strtok(redirHalf, delim);
            if (filename == NULL) {
                        snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                        write(STDERR_FILENO, msg, strlen(msg));
                        continue;
            }
            token = strtok(NULL, delim);
            if (token != NULL) {
                        snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                        write(STDERR_FILENO, msg, strlen(msg));
                        continue;
            }
                gTCheck = strchr(input, '>');
            key = strdup(input);
            token = strtok(key, delim);
            if (strcmp(token, ">") == 0) {
                        snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                write(STDERR_FILENO, msg, strlen(msg));
                free(key);
                continue;
            }
            *(gTCheck) = '\0';
            }
            token = strtok(input, delim);
            if (token == NULL) {
                    continue;
                }
            if (token != NULL && strcmp(token, "exit") == 0) {
                    exit(0);
                }
            if (token != NULL && strcmp(token, ">") == 0) {
                   snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                   write(STDERR_FILENO, msg, strlen(msg));
                   continue;
                }
            // alias
            if (token != NULL && strcmp(token, "alias") == 0) {
                    key = strtok(NULL, delim);
            if (key == NULL) {
                        printall();
                continue;
            }
            value = strtok(NULL, "");
            if (value == NULL) {
                        res = search(key);
                if (res != NULL) {
                            snprintf(msg, _MAX_LENGTH, "%s %s\n", res->key, res->value);
                write(STDOUT_FILENO, msg, strlen(msg));
                }
                continue;
            }
            add(strdup(key), strdup(value));
            continue;
            }
            // unalias
            if (token != NULL && strcmp(token, "unalias") == 0) {
                    key = strtok(NULL, delim);
            if (key == NULL) {
                snprintf(msg, _MAX_LENGTH, "unalias: Incorrect number of arguments.\n");
                write(STDOUT_FILENO, msg, strlen(msg));
                continue;
            }
            value = strtok(NULL, "");
            if (value != NULL) {
                        snprintf(msg, _MAX_LENGTH, "unalias: Incorrect number of arguments.\n");
                        write(STDOUT_FILENO, msg, strlen(msg));
                        continue;
            }
                    delete(key);
            continue;
            }
            res = search(token);
            if (res != NULL) {
            key = strtok(NULL, "");
            if (key == NULL) {
                    snprintf(newInput, _MAX_LENGTH, "%s", res->value);
            } else {
                    snprintf(newInput, _MAX_LENGTH, "%s %s", res->value, key);
            }
            token = strtok(newInput, delim);
            }
                while (token != NULL) {
                    *next++ = token;
                    token = strtok(NULL, delim);
                }
                *next = NULL;
                // puts("Checking:");
                // for (next = toks; *next != 0; next++)
                //     puts(*next);
            pid = fork();
                if (pid == 0) {
                    // child process
            if (filename != NULL) {
                        close(STDOUT_FILENO);
                int oFile = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
                        if (oFile < 0) {
                        perror("open");
                        _exit(1);
                        }
            }
                    execv(toks[0], toks);
            snprintf(msg, _MAX_LENGTH, "%s: Command not found.\n", toks[0]);
            write(STDERR_FILENO, msg, strlen(msg));
            _exit(1);
                } else {
                    // parent process
                    if (waitpid(pid, &status, 0) == -1) {
                        snprintf(msg, _MAX_LENGTH, "Child process %i failed.\n", pid);
                write(STDERR_FILENO, msg, strlen(msg));
                exit(1);
            }
                }
            }
    } else if (argc == 2) {
        // batch mode
        // read from file
        // execute file instructions
        FILE* fp;
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            snprintf(msg, _MAX_LENGTH, "Error: Cannot open file %s.\n", argv[1]);
            write(STDERR_FILENO, msg, strlen(msg));
        exit(1);
        }
        while (1) {
            temp = fgets(input, _MAX_LENGTH, fp);
            if (temp == NULL) {
                exit(0);
            }
            input[strcspn(input, "\n")] = 0;
            fprintf(stdout, "%s\n", input);
            fflush(stdout);
        next = toks;
            filename = NULL;
            gTCheck = strchr(input, '>');
            if (gTCheck != NULL) {
                redirHalf = (gTCheck + 1);
                gTCheck = strchr(redirHalf, '>');
                if (gTCheck != NULL) {
                    snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                    write(STDERR_FILENO, msg, strlen(msg));
                    continue;
                }
                filename = strtok(redirHalf, delim);
                if (filename == NULL) {
                    snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                    write(STDERR_FILENO, msg, strlen(msg));
                    continue;
                }
                token = strtok(NULL, delim);
                if (token != NULL) {
                    snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                    write(STDERR_FILENO, msg, strlen(msg));
                    continue;
                }
                gTCheck = strchr(input, '>');
        key = strdup(input);
                token = strtok(key, delim);
                if (strcmp(token, ">") == 0) {
                    snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
                    write(STDERR_FILENO, msg, strlen(msg));
                    free(key);
                    continue;
                }
                *(gTCheck) = '\0';
            }
        token = strtok(input, delim);
            if (token == NULL) {
                continue;
        }
        if (token != NULL && strcmp(token, "exit") == 0) {
                exit(0);
            }
        if (token != NULL && strcmp(token, ">") == 0) {
               snprintf(msg, _MAX_LENGTH, "Redirection misformatted.\n");
           write(STDERR_FILENO, msg, strlen(msg));
           continue;
        }
        // alias
            if (token != NULL && strcmp(token, "alias") == 0) {
                key = strtok(NULL, delim);
                if (key == NULL) {
                    printall();
                    continue;
                }
                value = strtok(NULL, "");
                if (value == NULL) {
                    res = search(key);
                    if (res != NULL) {
                        snprintf(msg, _MAX_LENGTH, "%s %s\n", res->key, res->value);
                        write(STDOUT_FILENO, msg, strlen(msg));
                    }
                    continue;
                }
                add(strdup(key), strdup(value));
                continue;
            }
            // unalias
            if (token != NULL && strcmp(token, "unalias") == 0) {
                key = strtok(NULL, delim);
                if (key == NULL) {
                    snprintf(msg, _MAX_LENGTH, "unalias: Incorrect number of arguments.\n");
                    write(STDOUT_FILENO, msg, strlen(msg));
                    continue;
                }
                value = strtok(NULL, "");
                if (value != NULL) {
                    snprintf(msg, _MAX_LENGTH, "unalias: Incorrect number of arguments.\n");
                    write(STDOUT_FILENO, msg, strlen(msg));
                    continue;
                }
                delete(key);
                continue;
            }
            res = search(token);
            if (res != NULL) {
                key = strtok(NULL, "");
                if (key == NULL) {
                snprintf(newInput, _MAX_LENGTH, "%s", res->value);
                } else {
                snprintf(newInput, _MAX_LENGTH, "%s %s", res->value, key);
                }
                token = strtok(newInput, delim);
            }
            while (token != NULL) {
                *next++ = token;
                token = strtok(NULL, delim);
            }
            *next = NULL;
            // puts("Checking:");
            // for (next = toks; *next != 0; next++)
            //     puts(*next);
            pid = fork();
            if (pid == 0) {
                // child process
        if (filename != NULL) {
                    close(STDOUT_FILENO);
                    int oFile = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
                    if (oFile < 0) {
                    perror("open");
                    _exit(1);
                    }
                }
                execv(toks[0], toks);
                snprintf(msg, _MAX_LENGTH, "%s: Command not found.\n", toks[0]);
                write(STDERR_FILENO, msg, strlen(msg));
                _exit(1);
            } else {
                // parent process
                if (waitpid(pid, &status, 0) == -1) {
                    snprintf(msg, _MAX_LENGTH, "Child process %i failed.\n", pid);
                    write(STDERR_FILENO, msg, strlen(msg));
                    exit(1);
                }
            }
         }
    }
    return 0;
}

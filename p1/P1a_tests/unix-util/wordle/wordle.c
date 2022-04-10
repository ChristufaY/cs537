// Copyright 2022 Christopher Yang
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int wordle(char* filename, char* str) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("wordle: cannot open file\n");
        exit(1);
    }
    int count = 0;
    char line[255];
    while (fgets(line, 255, fp) != NULL) {
        line[strcspn(line, "\n")] = 0;
        // printf("%s\n",line);
        if (strlen(line) == 5) {
            // printf("%s\n",line);
            for (int i = 0; i < strlen(line); i++)
                for (int j = 0; j < strlen(str); j++)
                    if (line[i] == str[j]) {
                        break;
                    } else {
                        count++;
                    }
        }
        // printf("%d\n", count);
        if (count == 5*strlen(str))
            printf("%s\n", line);
        count = 0;
    }
    fclose(fp);
    return 0;
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("wordle: invalid number of args\n");
        exit(1);
    }
    char* filename = argv[1];
    char* str = argv[2];
    wordle(filename, str);
    return 0;
}

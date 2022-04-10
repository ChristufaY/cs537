// Copyright 2022 Christopher Yang
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int output(char* input, char* filename) {
    FILE* fp;
    if (filename != NULL) {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            printf("my-look: cannot open file\n");
            exit(1);
        }
    } else {
          fp = stdin;
    }
    int index = 0;
    char temp[255];
    char line[255];
    while (fgets(line, 255 , fp) != NULL) {
        for (int i = 0; i < strlen(line) - 1; i++) {
            if ((line[i] >= '0' && line[i] <= '9') ||
                (line[i] >= 'a' && line[i] <= 'z') ||
                    (line[i] >= 'A' && line[i] <= 'Z')) {
                        temp[index] = line[i];
                        index++;
            }
        }
        temp[index] = '\0';
        index = 0;
        if (strncasecmp(input, temp, strlen(input)) == 0) {
            printf("%s", line);
        }
    }
    fclose(fp);
    return 0;
}
int main(int argc, char* argv[]) {
    /* read and parse input from command line */
    char* filename = NULL;
    char c;
    char* inputStr = NULL;
    while ((c = getopt(argc, argv, ":Vhf:")) != -1) {
        switch (c) {
            case 'V':
                printf("my-look from CS537 Spring 2022\n");
                exit(0);
            case 'h':
                printf("%s: displays lines beginning with given input string.\n"
                    , argv[0]);
                printf("usage: %s [-Vh] [-f filename] string\n", argv[0]);
                exit(0);
            case 'f':
                filename = optarg;
                break;
            default:
                printf("my-look: invalid command line\n");
    // printf("usage: %s [-Vh] [-f filename] string\n", argv[0]);
                exit(1);
        }
    }
    if (optind >= argc) {
           fprintf(stderr, "Expected argument after options\n");
           exit(1);
    }
    inputStr = argv[optind];
    // printf("filename = %s\n", filename);
    // printf("input string = %s\n", inputStr);
    output(inputStr, filename);
}

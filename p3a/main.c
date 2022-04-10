#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mapreduce.h"
#include "hashmap.h"

extern struct node** arr_part;
extern int partitions;

void Map(char *file_name) {
 FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r,.")) != NULL) {
          //  if (!strcmp(token, ""))
          //      break;

            MR_Emit(token, "1");
        }
    }
    free(line);
    fclose(fp);

}

void Reduce(char *key, Getter get_func, int partition_number) {
    int count = 0;
    char* value;
    while ((value = get_func(key, partition_number)) != NULL)
        count++;
    printf("%s %d\n", key, count);
}

int main(int argc, char *argv[]) {
//    arr_part = (struct node**) malloc(sizeof(struct node*) * partitions);
    MR_Run(argc, argv, Map, 1, Reduce, 1, MR_DefaultHashPartition);

}

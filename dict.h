// Created by tang on 2019-04-17.


#include <stdint.h>
#include <stdio.h>
#include "heap.h"

#ifndef URL_COUNTER_DICT_H
#define URL_COUNTER_DICT_H

#define DICT_INITIAL_SIZE 16

typedef struct dictEntry {
    uint64_t hash;
    int64_t offset, len, count;
    struct dictEntry *next;
} dictEntry;

typedef struct dict {
    dictEntry **table;
    FILE *fp;
    int size;
    int used;
} dict;

dict *dictCreate(FILE* fp);
int64_t dictAdd(dict *d, dictEntry* e);
void dictDumpToHeap(dict *d, heap *h);
void dictRelease(dict *d);

#endif //URL_COUNTER_DICT_H

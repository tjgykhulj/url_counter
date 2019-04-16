// Created by tang on 2019-04-15.
//

#ifndef URL_COUNTER_HASH_H
#define URL_COUNTER_HASH_H

#include <zconf.h>

typedef struct entry {
    unsigned int key;
    int64_t offset, len, count;
    struct entry *next;
} entry;

#endif //URL_COUNTER_HASH_H

// Created by tang on 2019-04-17.

#include "dict.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>

uint64_t rehash(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

dict* dictCreate(FILE* fp) {
    dict *d = malloc(sizeof(dict));
    d->fp = fp;
    d->size = DICT_INITIAL_SIZE;
    d->used = 0;
    d->table = malloc(d->size * sizeof(dictEntry*));
    memset(d->table, 0, d->size * sizeof(dictEntry*));
    return d;
}

void dictExpand(dict *d) {
    dict n;
    n.fp = d->fp;
    n.size = d->size * 2;
    n.used = d->used;
    n.table = malloc(n.size * sizeof(dictEntry*));
    memset(n.table, 0, n.size * sizeof(dictEntry*));
    for (int i=0; i<d->size; i++) {
        for (dictEntry *e = d->table[i]; e;) {
            dictEntry *next = e->next;
            uint64_t idx = rehash(e->hash) & (n.size - 1);
            e->next = n.table[idx];
            n.table[idx] = e;
            e = next;
        }
    }
    free(d->table);
    *d = n;
}

int64_t dictAdd(dict *d, dictEntry* e) {
    uint64_t idx = rehash(e->hash) & (d->size-1);
    for (dictEntry* i=d->table[idx]; i; i=i->next) {
        if (i->hash == e->hash && i->len == e->len) {
            return ++i->count;
        }
    }
    if (++d->used == d->size) {
        dictExpand(d);
    }
    e->next = d->table[idx];
    d->table[idx] = e;
    return e->count = 1;
}

void dictRelease(dict *d) {
    for (int i=0; i<d->size; i++) {
        for (dictEntry *e = d->table[i]; e;) {
            dictEntry *next = e->next;
            free(e);
            e = next;
        }
    }
    free(d->table);
    free(d);
}

void dictDumpToHeap(dict *d, heap *h) {
    for (int i=0; i<d->size; i++) {
        for (dictEntry *e = d->table[i]; e; e=e->next) {
            dictEntry *copy = malloc(sizeof(dictEntry));
            *copy = *e;
            heapInsert(h, copy);
        }
    }
}
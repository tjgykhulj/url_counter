//
// Created by tang on 2019-04-19.
//

#ifndef URL_COUNTER_HEAP_H
#define URL_COUNTER_HEAP_H


// 小根堆
typedef struct heap {
    int cap;
    int size;
    void **ele;
    int (*cmp) (const void *, const void *);
} heap;

heap* heapCreate(int cap, int (*cmp) (const void *,const void *));
void** heapRelease(heap *h);
void heapInsert(heap *h, void *ele);

#endif //URL_COUNTER_HEAP_H

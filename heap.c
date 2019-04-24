// Created by tang on 2019-04-19.
//

#include "heap.h"
#include <stdlib.h>
#include <string.h>

void swap(heap *h, int i, int j) {
    void *t = h->ele[i];
    h->ele[i] = h->ele[j];
    h->ele[j] = t;
}

void* pop(heap *h) {
    if (!h->size) {
        return NULL;
    }
    void *res = h->ele[0];
    h->ele[0] = h->ele[--h->size];
    for(int i=0, j=(i<<1)+1; j<h->size; i=j,j=(i<<1)+1) {
        if (j+1 < h->size && h->cmp(h->ele[j+1], h->ele[j]) < 0) j++;
        if (h->cmp(h->ele[i], h->ele[j]) <= 0) {
            break;
        }
        swap(h, i, j);
    }
    return res;
}

void push(heap *h, void *ele) {
    int *a = ele;
    h->ele[h->size++] = ele;
    for (int i=h->size-1; i; i=(i-1)>>1) {
        int j = (i-1)>>1;
        if (h->cmp(h->ele[i], h->ele[j]) < 0) {
            void *t = h->ele[i];
            h->ele[i] = h->ele[j];
            h->ele[j] = t;
        }
    }
}

heap* heapCreate(int cap, int (*cmp) (const void *,const void *)) {
    heap *h = malloc(sizeof(heap));
    h->cap = cap;
    h->cmp = cmp;
    h->size = 0;
    h->ele = malloc(cap * sizeof(void*));
    memset(h->ele, 0, cap * sizeof(void*));
    return h;
}

// 插入元素void *ele并维持总量在cap限制内（无需维护的元素会主动释放）
void heapInsert(heap *h, void *ele) {
    if (h->size < h->cap || h->cmp(ele, h->ele[0]) > 0) {
        if (h->size == h->cap) {
            void *x = pop(h);
            if (x) {
                free(x);
            }
        }
        push(h, ele);
    } else {
        free(ele);  // 若方法不插入ele数据，将其free掉以免空间浪费
    }
}

void** heapRelease(heap *h) {
    void **res = malloc(h->size * sizeof(void*));
    for (int i=h->size; i; i--) {
        res[i-1] = pop(h);
        int *x = res[i-1];
    }
    free(h->ele);
    free(h);
    return res;
}
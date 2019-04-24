// Created by tang on 2019-04-15.
//
#include "heap.h"
#include "dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#define LOADBUF_LEN 1024
#define HASH_INIT_VAL 1125899906842597L

int SEG_FILE_NUM = 12;

typedef long long i64;

void recordUrlInfo(FILE *fp, i64 offset, i64 len, unsigned long hash) {
    dictEntry e = {hash, offset, len};
    fwrite(&e, 24, 1, fp);
}

int cmpDictEntry(const void *first, const void *second) {
    const dictEntry *a = first, *b = second;
    if (a->count == b->count) {
        return 0;
    }
    return a->count > b->count? 1: -1;
}

// 切分数据到FILE_NUM个文件
void segData(char *filename) {
    uint64_t now = (uint64_t) time(NULL);
    FILE *fp = fopen(filename, "r");
    FILE **segFps = malloc(sizeof(FILE*) * SEG_FILE_NUM);
    for (int i=0; i<SEG_FILE_NUM; i++) {
        char segFilename[512];
        snprintf(segFilename, 512, "%s_%d", filename, i);
        segFps[i] = fopen(segFilename, "w");
    }

    dictEntry e;
    while (fread(&e, 24, 1, fp)) {
        uint64_t idx = (e.hash + now) & (SEG_FILE_NUM-1);
        recordUrlInfo(segFps[idx], e.offset, e.len, e.hash);
    }
    fclose(fp);
    for (int i=0; i<SEG_FILE_NUM; i++) {
        fclose(segFps[i]);
    }
    free(segFps);
}

// 读取指定文件中的map<url, count>计数并更新到heap *h中，若数量过大需要切分
void readAndSegData(char *filename, heap *h) {
    fprintf(stdout, "filename %s begin\n", filename);
    FILE *fp = fopen(filename, "r");
    dict *d = dictCreate(fp);
    dictEntry e;
    while (fread(&e, 24, 1, fp) > 0) {
        i64 ret = dictAdd(d, e);
        if (ret == -1) {
            // 失败，单文件不同的url量过大，释放资源
            dictRelease(d);
            fclose(fp);
            // 尝试继续切割文件
            fprintf(stdout, "filename %s need to seg\n", filename);
            segData(filename);
            char segFilename[512];
            for (int i=0; i<SEG_FILE_NUM; i++) {
                snprintf(segFilename, 512, "%s_%d", filename, i);
                readAndSegData(segFilename, h);
            }
            return;
        }
    }
    dictDumpToHeap(d, h);
    dictRelease(d);
    fclose(fp);
}

void countData(char *filename) {
    clock_t start_time = clock();

    heap *h = heapCreate(100, cmpDictEntry);
    readAndSegData(filename, h);

    void **top = heapRelease(h);
    for (int i=0; i<100; i++) {
        dictEntry *x = top[i];
        fprintf(stdout, "hash=%llu, off=%lld, len=%lld, count=%lld\n", x->hash, x->offset, x->len, x->count);
        free(x);
    }
    free(top);
    fprintf(stdout, "read data success, cost: %.4fs\n", (double) (clock()-start_time) / CLOCKS_PER_SEC);
}

// 解析文件，转为hash,offset,len数据
char* analyseData(char* filename) {
    clock_t start_time = clock();
    char *dstFn = malloc(512);
    snprintf(dstFn, 512, "%s~", filename);
    FILE *fp = fopen(filename, "r");
    FILE *dstFp = fopen(dstFn, "w");

    unsigned long hash = HASH_INIT_VAL;
    size_t size = 0;
    i64 start = 0, end = 0;
    unsigned char buf[LOADBUF_LEN+1];
    while ((size = fread(buf, 1, LOADBUF_LEN, fp)) > 0) {
        for (i64 i=0; i<size; i++, end++) {
            if (buf[i] != '\r' && buf[i] != '\n') {
                hash = ((hash << 5) + hash) + buf[i];
                continue;
            }
            recordUrlInfo(dstFp, start, end-start, hash);
            while (buf[i+1] == '\r' || buf[i+1] == '\n') {  //不同的换行可能性
                i++;
                end++;
            }
            start = end + 1;
            hash = HASH_INIT_VAL;
        }
        memset(buf, 0, LOADBUF_LEN);
    }
    fclose(fp);
    fclose(dstFp);

    fprintf(stdout, "analyse file success, cost: %.4fs\n", (double) (clock()-start_time) / CLOCKS_PER_SEC);
    return dstFn;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage ./url_counter [filename]\n");
        return 0;
    }
    if (strlen(argv[1]) == 0 || strlen(argv[1]) > 200) {
        fprintf(stderr, "filename len should between (0, 200)");
        return 0;
    }
    char* filename = argv[1];

    char* analyseFn = analyseData(filename);
    countData(analyseFn);
    free(analyseFn);
    return 0;
}

// Created by tang on 2019-04-15.
//
#include "mock.h"
#include "heap.h"
#include "dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#define LOADBUF_LEN 1024
#define HASH_INIT_VAL 1125899906842597L
#define SEG_FILE_NUM 256
#define SEG_FILE_MASK (SEG_FILE_NUM - 1)

typedef long long i64;

// 创建FILE_NUM个文件
FILE** createSegFile(char *srcFilename, const char *mode) {
    FILE **fps = malloc(sizeof(FILE*) * SEG_FILE_NUM);
    for (int i=0; i<SEG_FILE_NUM; i++) {
        char segFilename[256];
        snprintf(segFilename, 256, "%s_%d", srcFilename, i);
        fps[i] = fopen(segFilename, mode);
    }
    return fps;
}

void recordUrlInfo(FILE *fp, i64 start, i64 end, unsigned long hash) {
    dictEntry e = {hash, start, end-start+1};
    fwrite(&e, 24, 1, fp);
}

/* 若长度较小，使用uchar str[LOADBUF_LEN]；单条url过长冲破str，直接从src指定位置开始逐段读出，写入dst
void recordUrl(FILE *src, FILE *dst, i64 start, i64 end, unsigned char *str) {
    if (end-start <= LOADBUF_LEN) {
        str[end-start] = '\n';
        fwrite(str, 1, (size_t) end-start+1, dst);
        return;
    }

    fprintf(stdout, "buffer is too long(%lld->%lld), copy from file to file", start, end);
    // 记录src流原本指针位置
    i64 srcPos = ftell(src), i = start;
    fseek(src, start, SEEK_SET);

    unsigned char buf[LOADBUF_LEN+1];
    while (i < end) {
        size_t len = (size_t) min(end-i, LOADBUF_LEN);
        if (fread(buf, 1, len, src) > 0) {
            buf[len] = '\n';
            fwrite(buf, 1, len + 1, dst);
            i += len;
        }
    }
    // 回到src流原本指针位置
    fseek(src, srcPos, SEEK_SET);
}
 */

// 刮分数据到FILE_NUM个文件
int segData(char* filename) {
    clock_t start_time = clock();

    FILE *fp = fopen(filename, "r");
    FILE **segFps = createSegFile(filename, "w");

    unsigned long hash = HASH_INIT_VAL;
    size_t size = 0;
    i64 start = 0, end = 0;
    unsigned char buf[LOADBUF_LEN+1], str[LOADBUF_LEN+1];
    while ((size = fread(buf, 1, LOADBUF_LEN, fp)) > 0) {
        for (i64 i=0; i<size; i++, end++) {
            if (buf[i] != '\r' && buf[i] != '\n') {
                hash = ((hash << 5) + hash) + buf[i];
                continue;
            }
            recordUrlInfo(segFps[hash & SEG_FILE_MASK], start, end, hash);
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
    for (int i=0; i<SEG_FILE_NUM; i++) {
        fclose(segFps[i]);
    }

    fprintf(stdout, "seg file success, cost: %.4fs\n", (double) (clock()-start_time) / CLOCKS_PER_SEC);
    return 0;
}

void _readSegData(heap *h, FILE *fp, FILE *segFp) {
    dict *d = dictCreate(fp);
    for (int i=0; 1; i++) {
        dictEntry *e = malloc(sizeof(dictEntry));
        if (fread(e, 24, 1, segFp) == 0) {
            free(e);
            break;
        }
        dictAdd(d, e);
    }
    dictDumpToHeap(d, h);
    dictRelease(d);
}

int cmpDictEntry(const void *first, const void *second) {
    const dictEntry *a = first, *b = second;
    if (a->count == b->count) {
        return 0;
    }
    return a->count > b->count? 1: -1;
}

// 读切割后的文件
void readSegData(char *filename) {
    clock_t start_time = clock();

    FILE *fp = fopen(filename, "r");
    FILE **segFps = createSegFile(filename, "r");
    heap *h = heapCreate(100, cmpDictEntry);
    for (int i=0; i<SEG_FILE_NUM; i++) {
        fprintf(stdout, "[file %d]\n", i);
        _readSegData(h, fp, segFps[i]);
        fclose(segFps[i]);
    }
    void **top = heapRelease(h);
    for (int i=0; i<100; i++) {
        dictEntry *x = top[i];
        fprintf(stdout, "hash=%llu, off=%lld, len=%lld, count=%lld\n", x->hash, x->offset, x->len, x->count);
    }
    fprintf(stdout, "read seg data success, cost: %.4fs\n", (double) (clock()-start_time) / CLOCKS_PER_SEC);
}

/* 思路是读取指定文件，切割为SEG_FILE_NUM份信息，每份中带有offset, len, hash
每个文件解析为一个map，并更新到一个小根堆中，小根堆里最终是全量中的count top 100
若某一份切割文件解出的map的key数量过大，可以将SEG_FILE_NUM++并重试。

 由于使用hash+len判断值，有一个较小的错判率。为了保证100%正确，有两种方向：
 1. 当两个字串的hash与len均相同时，读src input文件中指定offset来判断字串是否真的相同
 2. 做一个hash2值，使错判断小到忽略不计
  */
int main(int argc, char **argv) {
    char *filename = "out/url";
    //mock_data(filename, (i64)10<<30);  // mock数据
    //segData(filename);      //1278s：切割指定文件为SEG_FILE_NUM份
    readSegData(filename);  //5909s：解析指定文件，打出top 100

    return 0;
}
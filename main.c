// Created by tang on 2019-04-15.
//
#include "mock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOADBUF_LEN 1024
#define HASH_INIT_VAL 1125899906842597L
#define FILE_NUM 256
#define FILE_MASK (FILE_NUM - 1)

#define min(a,b) ((a)<(b)?(a):(b))
typedef long long i64;

// 创建FILE_NUM个文件
FILE** createSegFile(char *srcFilename) {
    FILE **fps = malloc(sizeof(FILE*) * FILE_NUM);
    for (int i=0; i<FILE_NUM; i++) {
        char segFilename[256];
        snprintf(segFilename, 256, "%s_%d", srcFilename, i);
        fps[i] = fopen(segFilename, "w");
    }
    return fps;
}

void recordUrlInfo(FILE *fp, i64 start, i64 end, unsigned long hash) {
    i64 p[3] = {start, end, (i64) hash};
    fwrite(p, 8, 3, fp);
}

// 若长度较小，使用uchar str[LOADBUF_LEN]；单条url过长冲破str，直接从src指定位置开始逐段读出，写入dst
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

// 刮分数据到FILE_NUM个文件
int segData(char* filename) {
    FILE *fp = fopen(filename, "r");
    FILE **segFp = createSegFile(filename);

    unsigned long hash = HASH_INIT_VAL;
    size_t size = 0;
    i64 start = 0, end = 0;
    unsigned char buf[LOADBUF_LEN+1], str[LOADBUF_LEN+1];
    while ((size = fread(buf, 1, LOADBUF_LEN, fp)) > 0) {
        for (i64 i=0; i<size; i++, end++) {
            if (buf[i] != '\r' && buf[i] != '\n') {
                /*
                if (end-start <= LOADBUF_LEN) {
                    str[end-start] = buf[i];
                }
                 */
                hash = ((hash << 5) + hash) + buf[i];
                continue;
            }
            recordUrlInfo(segFp[hash & FILE_MASK], start, end, hash);
            //recordUrl(fp, segFp[hash & FILE_MASK], start, end, str);
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
    for (int i=0; i<FILE_NUM; i++) {
        fclose(segFp[i]);
    }
    return 0;
}

// 100GB的文件，平均长度为31.61633时可能性最多样，理论最大可能不同的url有3,292,037,176个（33亿不到）
int main(int argc, char **argv) {
    clock_t start = clock();

    //mock_data(filename, (i64)1<<30);
    char *filename = "out/url";
    if (argc == 2) {
        filename = argv[1];
        if (strlen(filename) > 255) {
            fprintf(stderr,"filename is too long\n");
            return 0;
        }
    } else if (argc > 2) {
        fprintf(stderr,"Usage: ./main [input_file]\n");
        return 0;
    }
    segData(filename);

    printf("duration: %.4fs", (double) (clock()-start) / CLOCKS_PER_SEC);
    return 0;
}
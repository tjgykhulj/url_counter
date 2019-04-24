//
// Created by tang on 2019-04-15.
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

typedef long long i64;

void mock_data(char *filename, i64 size) {
    srand((unsigned) time(NULL));
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "can't open file: %s\n", filename);
        return;
    }

    char buf[1024];
    size_t len = 0, l = 0;
    i64 s = size;
    while (s--) {
        int c = rand() % 15;
        if (c < 14) {
            buf[len++] = (char) ('a' + c);
            l++;
        } else {
            if (l > 3) {
                buf[len++] = '\n';
                l = 0;
            }
        }
        if (len == 1024 || s == 0) {
            fwrite(buf, len, 1, fp);
            len = 0;
        }
    }
    fprintf(stdout, "finish writing mock data file: %s, %lld Byte\n", filename, size);
    fclose(fp);
}


// mock数据到argv[1]中，数据量为argv[2] MB个字符
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage ./mock [filename] [mock size in MB]\n");
        return 0;
    }
    char *filename = argv[1], *tmp;
    i64 mb = strtoll(argv[2], &tmp, 10);
    if (strlen(filename) <= 0) {  // 控制在1M~100GB，文件合法性检验
        fprintf(stderr, "Invalid filename, check if filename is empty.\n");
        return 0;
    }
    if (mb < 1 || mb > (100<<10)) {
        fprintf(stderr, "Invalid mock size, make sure mock size in [1, 100<<10] (MB).\n");
    }
    mock_data(filename, mb<<20); // MB->Byte
}


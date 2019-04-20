//
// Created by tang on 2019-04-15.
//
#include "mock.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void mock_data(char *filename, i64 size) {
    srand((unsigned) time(NULL));
    FILE *fp = fopen(filename, "w");

    char buf[1024];
    size_t len = 0, l = 0;
    while (size--) {
        i64 c = random() % 15;
        if (c < 14) {
            buf[len++] = (char) ('a' + c);
            l++;
        } else {
            if (l > 3) {
                buf[len++] = '\n';
                l = 0;
            }
        }
        if (len == 1024 || size == 0) {
            fwrite(buf, len, 1, fp);
            len = 0;
        }
    }
    fclose(fp);
}


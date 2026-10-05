#include "lib/mystdio.h"

int main(int argc, char **argv) {
    char* msg = (char*)malloc(64);
    if (msg) {
        printf_x("malloc succeeded! addr=%x\n", (unsigned int)msg);
        free(msg);
    }
    return 0;
}

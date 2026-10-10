#include "lib/mystdio.h"

int main(int argc, char **argv) {
    char* msg = (char*)malloc(64);
    if (msg) {
        printf_x("malloc1 succeeded! addr=%x\n", (unsigned int)msg);
        free(msg);
    }
    
    /*
    char* msg2 = (char*)malloc(64);
    if (msg2) {
        printf_x("malloc2 succeeded! addr=%x\n", (unsigned int)msg2);
        free(msg2);
    }*/
    
    return 0;
}

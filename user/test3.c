#include "lib/mystdio.h"

int main(int argc, char **argv){   

    printf_d("test3 argc = %d\n", argc);
    
    int i=0;
    for(;i<argc;i++){
        write(argv[i]);
    }
    
    return 0;
}


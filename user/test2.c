#include "lib/os.h"
#include "lib/mystdio.h"

int main(void) {
    write("hello from user!\n");
    
    int num=3;
    printf_d("num = %d\n", num);
    return 0;
}

#include "user_exec.h"

int user_exec(void* entry)
{
    int ret;

    asm volatile (
        "call *%1"
        : "=a"(ret)   // EAXに入った返り値を受け取りretへ格納
        : "r"(entry)
        : "memory", "cc"
    );

    return ret;
}

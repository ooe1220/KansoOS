#include "x86/cpu.h"

static int starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str++ != *prefix++) return 0;
    }
    return 1;
}

int is_bare_metal(void) {
    char cpuname[48];
    get_cpu_name(cpuname);

    // 先頭の空白を飛ばす
    char *p = cpuname;
    while (*p == ' ') p++;

    if (starts_with(p, "Intel")) {
        return 1;
    }
    return 0;
}

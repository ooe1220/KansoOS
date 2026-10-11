#pragma once
#include "stdint.h"

#define USER_PROG_MEM 0x10000 // ユーザプログラムを置くアドレス
#define USER_ARG_MEM  0x20000  // コマンドライン引数を置くアドレス、ユーザプログラムには実体でなくアドレスを渡す

//int user_exec(void* entry);
int user_exec(void* entry, int argc, char **argv);

int run_file(const char *line);

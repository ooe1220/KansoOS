#include "user_exec.h"

void user_exec(void* entry)
{
    asm volatile (
        "pushf\n"        /* EFLAGS 保存 */
        "call *%0\n"     /* ユーザコード実行 */
        "popf\n"         /* EFLAGS 復元 */
        :
        : "r"(entry)
        : "memory", "cc"
    );
}

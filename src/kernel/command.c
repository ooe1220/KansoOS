#include "command.h"
#include "x86/console.h"
#include "x86/io.h"
#include "lib/string.h"
#include "lib/stdint.h"
#include "lib/stddef.h" // NULLの定義
#include "user_exec.h"
#include "drivers/ata.h"
#include "fs/fat16.h"
#include "x86/pic.h"
#include "fs/fs_file.h"



// 内部コマンド実行
int run_builtin_command(const char *line){

    if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help     - Show this message\n");
        kputs("  clear    - Clear screen\n");
        kputs("  reboot   - Reboot CPU\n");
        kputs("  shutdown - Shutdown system (QEMU only)\n");
        kputs("  ls       - List files in current directory\n");
        kputs("  dir      - Same as 'ls'");
        return 0;
    }
    
    if (strcmp(line, "clear") == 0) {
        console_clear();
        return 0;
    }
    
    
    if (strcmp(line, "reboot") == 0) {
        kputs("Rebooting...\n");
        outb(0x64, 0xFC);
        kputs("This should not print\n");
        return 0;
    }
    
    
    if (strcmp(line, "shutdown") == 0) {
        outw(0x604, 0x2000);//QEMU専用
        return 0;
    }
    
    if (strcmp(line, "test") == 0) {
        kputs("test\n");
        return 0;
    }
    
    if (strcmp(line, "ls") == 0 || strcmp(line, "dir") == 0) {
        kputs("\n");
        fs_dir_list(); // fs/fat16.h
        return 0;
    }
    
    return -1;
}


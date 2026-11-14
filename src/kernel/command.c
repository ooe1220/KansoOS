#include "command.h"
#include "x86/console.h"
#include "x86/io.h"
#include "lib/string.h"
#include "lib/stdint.h"
#include "lib/stddef.h" // NULLの定義
#include "user_exec.h"
#include "drivers/ata.h"
#include "drivers/pci.h"
#include "fs/fs_ops.h"

#include "x86/pic.h"
#include "fs/fs_file.h"
#include "debug/serial.h"
#include "debug/debug_print.h"

extern fs_ops_t *g_fs_ops;

// 内部コマンド実行
int run_builtin_command(const char *line){

    if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help     - Show this message\n");
        kputs("  clear    - Clear screen\n");
        kputs("  reboot   - Reboot CPU (QEMU only)\n");
        kputs("  shutdown - Shutdown system (QEMU only)\n");
        kputs("  lspci    - List PCI devices\n");
        kputs("  ls       - List files in current directory\n");
        kputs("  dir      - Same as 'ls'");
        return 0;
    }
    
    if (strcmp(line, "clear") == 0) {
        console_clear();
        return 0;
    }
   
    if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
        return 0;
    }
    
    if (strcmp(line, "shutdown") == 0) {
        outw(0x604, 0x2000);//QEMU専用
        return 0;
    }
    
    if (strcmp(line, "test") == 0) {
        kputs("\ntest\n");
        return 0;
    }

    if (strcmp(line, "lspci") == 0) {
        kputs("\n");
        pci_scan();
        return 0;
    }

    if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
        return 0;
    }
    
    if (strcmp(line, "ls") == 0 || strcmp(line, "dir") == 0) {
        kputs("\n");

        if (g_fs_ops && g_fs_ops->dir)
            g_fs_ops->dir();
        else
            kputs("dir: not supported\n");
        return 0;
    }
    
    return -1;
}


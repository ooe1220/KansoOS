#include "command.h"
#include "arch/x86/console.h"
#include "arch/x86/io.h"
#include "lib/string.h"
#include "lib/string.h"
#include "lib/stdint.h"
#include "fs/dir.h"

void execute_command(const char *line) {
    if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help   - Show this message\n");
        kputs("  clear  - Clear screen\n");
        kputs("  reboot - Reboot CPU\n");
    } else if (strcmp(line, "clear") == 0) {
        console_clear();
    } else if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
    } else if (strcmp(line, "ls") == 0 || strcmp(line, "dir") == 0) {
        kputs("\n");
        fs_dir_list();
    } else if (line[0] != 0) {
        kputs("\nUnknown command: ");
        kputs(line);
    }
}


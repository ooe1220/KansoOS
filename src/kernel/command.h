#ifndef COMMAND_H
#define COMMAND_H

void init_cursor_from_hardware();
int do_builtin(const char *line);
void run_file(const char *line);

#endif


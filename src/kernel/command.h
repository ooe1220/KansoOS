#ifndef COMMAND_H
#define COMMAND_H

void init_cursor_from_hardware();
int run_builtin_command(const char *line);
void run_file(const char *line);

#endif


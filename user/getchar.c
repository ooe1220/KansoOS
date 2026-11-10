#include "lib/mystdio.h"

int main(int argc, char **argv) {

    write("Type one letter (a-Z): ");

    char c = getchar();

    write("You typed: ");
    char line[4];
    line[0] = c;
    line[1] = '\n';
    line[2] = '\0';
    write(line);

    return 0;
}
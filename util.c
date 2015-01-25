#include "util.h"

void print_error(char *error) {
    printf("[Error %d]: %s (%s)\n", errno, strerror(errno), error);
}

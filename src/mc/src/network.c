#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "adt.h"

static void error_handler(void);

char *serialise_packet(const char *s) {
    char *result = malloc(strlen(s) + 1);
    if (result == NULL) {
        error_handler();
    }
    strcpy(result, s);

    return result;
}

void error_handler(void) {
    // do stuff
}
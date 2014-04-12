#include "main.h"

char * to_hex(const char *data, size_t length) {
    char *out = malloc(length * 4);

    if (out != NULL) {
        int i;
        char *p = out;

        for (i = 0; i < length; i++) {
            sprintf(p, "%02x ", data[i]);
            p += 3;
        }
    }

    return out;
}

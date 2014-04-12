#ifndef MAIN_H
#define MAIN_H

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>

/**
 * Config
 */

#define DEBUG

#define USE_HEX 0
#define BAUDRATE B115200

extern char * to_hex(const char *data, size_t length);
 
#endif /* MAIN_H */

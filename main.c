#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <strings.h>

#define BAUDRATE B57600

/**
 * Functions
 */

extern void init(int argc, char **argv);

extern void device_open();
extern void device_loop();
extern void device_process_input(char *data, int received_count);
extern void device_close();

extern void handler_sigio (int status);
extern void handler_sigint (int status);

extern void exit_failure();

/**
 * Variables
 */

static char            *g_device_path = NULL;
static int              g_device_fd;
static volatile int     g_loop = false;
static volatile int     g_received = false;

void main(int argc, char **argv) {
    init(argc, argv);
    device_open();
    device_loop();
    device_close();
}

void exit_failure() {
    exit(EXIT_FAILURE);
}

void init(int argc, char **argv) {
    int opt;

    char *device_path = NULL;

    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
        case 'd':
            device_path = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -d <device_path>\n", argv[0]);
            exit_failure();
        }
    }

    if (device_path == NULL) {
        exit_failure();
    }

    g_device_path = device_path;

    /* set segint handler */

    struct sigaction saction;
    bzero(&saction, sizeof(saction));
    saction.sa_handler = handler_sigint;
    sigaction(SIGINT, &saction, NULL);
}

void device_open() {
    g_device_fd = open(g_device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (g_device_fd < 0) {
        perror(g_device_path);
        exit_failure();
    }

    /* set sigio handler */

    struct sigaction saction;
    bzero(&saction, sizeof(saction));
    saction.sa_handler = handler_sigio;
    sigaction(SIGIO, &saction, NULL);

    /* enable sigio callbacks */

    fcntl(g_device_fd, F_SETOWN, getpid());
    fcntl(g_device_fd, F_SETFL, FASYNC);
}

void device_loop() {
    char buf[256];
    struct termios old_termios, new_termios;

    tcgetattr(g_device_fd, &old_termios);

    new_termios.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    new_termios.c_iflag = IGNPAR | ICRNL;
    new_termios.c_oflag = 0;
    new_termios.c_lflag = 0;
    new_termios.c_cc[VMIN]=1;
    new_termios.c_cc[VTIME]=0;

    tcflush(g_device_fd, TCIFLUSH);
    tcsetattr(g_device_fd,TCSANOW,&new_termios);

    while (g_loop == false) {
        usleep(10000);

        if (g_received == false) {
            device_process_input(buf, read(g_device_fd, buf, 255));
            g_received = true;      /* wait for new input */
        }

        fflush(stdout);
    }

    /* restore old port settings */
    tcsetattr(g_device_fd, TCSANOW, &old_termios);
}

char * to_hex(const char *data, size_t length) {
    char *out = malloc(length * 4);

    if (out != NULL) {
        int i;
        char *p = out;
        for (i = 0; i < length; i++) {
            sprintf(p, "%02x ", data[i]);
            p += 4;
        }
    }

    return out;
}

void device_process_input(char *data, int received_count) {
    char *dump = to_hex(data, received_count);
    fprintf(stderr, "received %d count: %s\n", received_count, dump);
    free(dump);
}

void device_close() {
    if (close(g_device_fd) < 0) {
        perror(g_device_path);
        exit_failure();
    }
}

/**
 * Callback
 */

void handler_sigint(int status)
{
    g_loop = true;
}

void handler_sigio (int status)
{
    g_received = false;
}

#include "main.h"

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

#ifdef DEBUG
    fprintf(stderr, "Waiting for data:\n");
#endif

    while (g_loop == false) {
        usleep(10000);

        if (g_received == false) {
            int count;

            count = read(g_device_fd, buf, 255);
            if (count >= 1) {
                device_process_input(buf, count);
            }

            g_received = true;
        }

        fflush(stdout);
    }

    tcsetattr(g_device_fd, TCSANOW, &old_termios);
}

void device_process_input(char *data, int received_count) {
#ifdef DEBUG

#if USE_HEX == 1
    char *dump = to_hex(data, received_count);
    fprintf(stderr, "  received %d count: %s\n", received_count, dump);
    free(dump);
#else
    fprintf(stderr, "  received %d count: %.*s\n", received_count, received_count - 1, data + 1);
#endif

#endif
}

void device_close() {
    if (close(g_device_fd) < 0) {
        perror(g_device_path);
        exit_failure();
    }

#ifdef DEBUG
    fprintf(stderr, "Bye bye\n");
#endif    
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

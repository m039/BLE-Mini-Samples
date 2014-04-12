CC=gcc
CFLAGS=
DEPS=main.h

a.out : main.o misc.o
	$(CC) $< -o $@ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

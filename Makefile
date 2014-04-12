CC=gcc
CFLAGS=
DEPS=main.h
RM=rm
PROGRAM=a.out

$(PROGRAM): main.o misc.o
	$(CC) $< -o $@ $(CFLAGS)

clean:
	$(RM) -f *.o $(PROGRAM)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

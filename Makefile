CFLAGS=-O
 
CC=gcc
 
.c.o:
	$(CC) -c $(CFLAGS) $*.c
 
all: stv 
 
OBJS = iSamsungTV.o
 
stv: $(OBJS) iSamsungTV.o
	$(CC) $(CFLAGS) -o iSamsungTV $(OBJS) 
 
clean:
	/bin/rm -f iSamsungTV *.o *~
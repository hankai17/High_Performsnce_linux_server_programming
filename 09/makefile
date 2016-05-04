.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN= 9-1use_select
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
#cli:cli.o sckutil.o
#	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(BIN)





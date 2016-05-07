.PHONY:clean all
CC=gcc
CFLAGS= -Wall -g
BIN= 9-3mtlt
all:$(BIN)
%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

9-3mtlt:9-3mtlt.o 
	$(CC) $(CFLAGS) $^ -o $@ 
	
#9-1use_select_thread:9-1use_select_thread.o 
#	$(CC) $(CFLAGS) $^ -o $@ -lpthread

clean:
	rm -f *.o $(BIN)





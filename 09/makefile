.PHONY:clean all
CC=gcc
CFLAGS= -Wall -g
BIN= use_poll poll_client poll_client2
all:$(BIN)
%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

poll_client2:poll_client2.o 
	$(CC) $(CFLAGS) $^ -o $@ 
	
#9-1use_select_thread:9-1use_select_thread.o 
#	$(CC) $(CFLAGS) $^ -o $@ -lpthread

clean:
	rm -f *.o $(BIN)





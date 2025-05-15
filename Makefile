EXEC = motors
OBJS = motors.o rotary_encoder.o tcp_server.o
CC=gcc
LDLIBS += -lpigpio

CFLAGS = -Wall -g -O2 -pthread
LDFLAGS += -pthread

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	
clean:
	-rm -f  *.elf *.gdb *.o

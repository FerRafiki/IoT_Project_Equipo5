CC ?= gcc

EXECS = playback
SRCS  = $(EXECS:%=%.c)
OBJS  = $(SRCS:.c=.o)

CFLAGS += -g -Wall
LIBS   = -lasound

all: $(EXECS)

# Make sure executables are linked from .o, not directly from .c
$(EXECS): $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXECS)

fresh: clean all

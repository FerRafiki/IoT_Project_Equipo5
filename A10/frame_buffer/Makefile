CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME = frame_buffer
OBJFILES = frame_buffer.o
LIB_DIRS = .

all: $(APP_NAME)

$(APP_NAME): $(OBJFILES)
	$(CC) $^ -o $@ -L$(LIB_DIRS)

%.o: %.c
	$(CC) -c $^ -o $@ -g

clean:
	rm *.o  $(APP_NAME)

fresh:
	make clean
	make all

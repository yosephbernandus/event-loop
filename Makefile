.PHONY: all clean 

CC=gcc

# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -Werror: Treat warnings as errors
# -Wpedantic: Enforce the standard
# -std=c11: Use the C11 standard
CFLAGS=-Wall -Wextra -Werror -Wpedantic -std=c11 

DEBUG=1

ifeq ($(DEBUG), 0)
	CFLAGS+=-O3 
else
	CFLAGS+=-O0 -g -DDEBUG
endif 

SRCS=main.c 
OBJS=$(SRCS:.c=.o)

all: main 

main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning up..."
	rm -f main $(OBJS)

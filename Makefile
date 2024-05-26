CFLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb
GCC = gcc
heap: main.c
	$(GCC) $(CFLAGS) -o heap main.c

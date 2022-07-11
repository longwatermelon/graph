CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Isrc -O3
LIBS=-L. -lm -lcglm -lSDL2 -lSDL2_image

SRC=$(wildcard src/*.c src/*/*.c)
OBJS=$(addprefix obj/, $(SRC:.c=.o))

all:
	mkdir -p obj/src/shaderlang
	$(MAKE) lib
	$(CC) $(CFLAGS) example.c $(LIBS) -lgraph

lib: $(OBJS)
	$(AR) $(ARFLAGS) libgraph.a $^

obj/src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

clean:
	-rm -rf obj/ libgraph.a a.out


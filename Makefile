# Project: yuki
# Author: Huan Du (huan.du.work@gmail.com)

CC = gcc
AR = ar

PROJECT_NAME = yuki
LIB = lib$(PROJECT_NAME).a

OBJS  = $(patsubst %.c,%.o,$(wildcard *.c))
LIBS = -lpthread -lz
INCS =  
CFLAGS = $(INCS) -std=c99 -Wall -Werror -g -O1
RM = rm -f

.PHONY: all lib test clean

all: lib

test: lib
	cd test/ && make -f Makefile
	cd ..

lib: $(OBJS) $(LIB)

$(LIB): 
	$(AR) rc $@ $(OBJS)
	ranlib $@
	mkdir -p output/lib output/include
	cp $@ output/lib
	cp *.h output/include

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	${RM} $(OBJS) $(LIB)
	${RM}r output/
	cd test/ && make -f Makefile clean

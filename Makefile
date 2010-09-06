# Project: yuki
# Author: Huan Du (huan.du.work@gmail.com)

CC = gcc
AR = ar

PROJECT_NAME = yuki
LIB = lib$(PROJECT_NAME).a

OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
LIBS = \
    -L/usr/local/webserver/mysql/lib/mysql/ -lmysqlclient_r \
    -L../libconfig/lib -lconfig \
    -lpthread -lz
INCS = \
    -I/usr/local/webserver/mysql/include/mysql/ \
    -I../libconfig/include
DFLAGS =
CFLAGS = $(INCS) -std=c99 -Wall -Werror -g
RM = rm -f

.PHONY: all lib test clean debug samples

all : lib

debug : DFLAGS += -DDEBUG
debug : lib

test : lib
	cd test/ && make && cd ..

lib : $(OBJS) $(LIB)

$(LIB) : 
	$(AR) rc $@ $(OBJS)
	ranlib $@
	mkdir -p output/lib output/include
	cp $@ output/lib
	cp *.h output/include

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(DFLAGS)

clean :
	${RM} $(OBJS) $(LIB)
	${RM}r output/
	cd test/ && make clean && cd ..
	cd samples/ && make clean && cd ..

samples :
	cd samples/ && make && cd ..

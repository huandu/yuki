# Project: yuki
# Author: Huan Du (huan.du.work@gmail.com)

CC = gcc

PROJECT_NAME = simple_sidu
LINKOBJ = $(PROJECT_NAME).o
OBJS  = $(filter-out $(LINKOBJ),$(patsubst %.cpp,%.o,$(wildcard *.cpp)))

YUKI_INCLUDE_PATH = ../../output/include
YUKI_LIB_PATH = ../../output/lib
MYSQL_LIB_PATH = /usr/local/webserver/mysql/lib/mysql
CONFIG_LIB_PATH = $(shell cd ../../../libconfig/lib && pwd)

LIB_DIRS = -L$(YUKI_LIB_PATH) -L$(MYSQL_LIB_PATH) -L$(CONFIG_LIB_PATH)
LIBS = -lyuki -lmysqlclient_r -lconfig -lpthread -lz
INCS = -I$(YUKI_INCLUDE_PATH)
BIN  = $(PROJECT_NAME)

DFLAGS =
CFLAGS = $(INCS) $(DFLAGS) -g -Wall -Werror
LDFLAGS = $(LIB_DIRS) $(LIBS)
LNKFLAGS = -Wl,-rpath,$(MYSQL_LIB_PATH) -Wl,-rpath,$(CONFIG_LIB_PATH)
RM = rm -f

.PHONY: all bin clean debug

all : bin

debug : DFLAGS += -DDEBUG

clean :
	${RM} $(OBJS) $(BIN) $(LINKOBJ)

bin : $(OBJS) $(BIN)

$(BIN) : $(LINKOBJ)
	$(CC) $< $(OBJS) -o $@ $(LDFLAGS) $(LNKFLAGS)

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

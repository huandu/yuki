# Project: yuki
# Author: Huan Du (huan.du.work@gmail.com)

CC = g++

PROJECT_NAME = yuki_test
LINKOBJ = $(PROJECT_NAME).o
OBJS  = $(filter-out $(LINKOBJ),$(patsubst %.cpp,%.o,$(wildcard *.cpp)))

GTEST_INCLUDE_PATH = /usr/local/include
GTEST_LIB_PATH = /usr/local/lib
YUKI_INCLUDE_PATH = ../output/include
YUKI_LIB_PATH = ../output/lib
MYSQL_LIB_PATH = /usr/local/webserver/mysql/lib/mysql
CONFIG_LIB_PATH = $(shell cd ../../libconfig/lib && pwd)

LIB_DIRS = -L$(YUKI_LIB_PATH) -L$(GTEST_LIB_PATH) -L$(MYSQL_LIB_PATH) -L$(CONFIG_LIB_PATH)
LIBS = -lyuki -lmysqlclient_r -lconfig -lgtest -lpthread -lz
INCS = -I$(GTEST_INCLUDE_PATH) -I$(YUKI_INCLUDE_PATH)
BIN  = $(PROJECT_NAME)

DFLAGS =
CFLAGS = $(INCS) $(DFLAGS) -g -Wall -Werror
LDFLAGS = $(LIB_DIRS) $(LIBS)
LNKFLAGS = -Wl,-rpath,$(MYSQL_LIB_PATH) -Wl,-rpath,$(CONFIG_LIB_PATH) -Wl,-rpath,$(GTEST_LIB_PATH)
RM = rm -f

.PHONY: all bin clean debug

all : bin

debug : DFLAGS += -DDEBUG

clean :
	${RM} $(OBJS) $(BIN) $(LINKOBJ)

bin : $(OBJS) $(BIN)

$(BIN) : $(LINKOBJ)
	$(CC) $< $(OBJS) -o $@ $(LDFLAGS) $(LNKFLAGS)

%.o : %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

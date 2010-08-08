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

LIB_DIRS = -L$(YUKI_LIB_PATH) -L$(GTEST_LIB_PATH)
LIBS =  -lyuki -lgtest -lpthread -lz
INCS =  -I$(GTEST_INCLUDE_PATH) -I$(YUKI_INCLUDE_PATH)
BIN  = $(PROJECT_NAME)

DFLAGS =
CFLAGS = $(INCS) $(DFLAGS) -g -O1 -Wall -Werror
LDFLAGS = $(LIB_DIRS) $(LIBS)
RM = rm -f

.PHONY: all bin clean debug

all: bin

debug: DFLAGS += -DDEBUG
debug: bin

clean:
	${RM} $(OBJS) $(BIN) $(LINKOBJ)

bin: $(OBJS) $(BIN)

$(BIN): $(LINKOBJ)
	$(CC) $< $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

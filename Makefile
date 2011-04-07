# Project: yuki
# Author: Huan Du (huan.du.work@gmail.com)


PROJECT_NAME = yuki
LIB = lib$(PROJECT_NAME).a

include ../Makefile

all : lib

debug : DFLAGS += -DDEBUG
debug : lib

test : lib
	cd test/ && make && cd ..

lib : $(OBJS) $(LIB)

$(LIB) :
	$(AR) rc $@ $(OBJS)
	ranlib $@
	mkdir -p $(YUKI_LIB_PATH) $(YUKI_INCLUDE_PATH)
	cp $@ $(YUKI_LIB_PATH)
	cp *.h $(YUKI_INCLUDE_PATH)

%.o : %.c
	$(CC) -c $< -o $@ $(INCS_3RD) $(CFLAGS) $(DFLAGS)

clean :
	${RM} $(OBJS) $(LIB)
#	${RM}r output/
	cd test/ && make clean && cd ..
	cd samples/ && make clean && cd ..

samples :
	cd samples/ && make && cd ..

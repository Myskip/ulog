ULIST_DIR?=~/codes/application/ulist
SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
LIB:=ulog.a

.PHONY:all clean lib

all:lib

$(OBJS):
	$(CC) -o $@ -c $(patsubst %.o,%.c,$@) $(CFLAGS) -I $(ULIST_DIR)

lib:$(OBJS)
	$(AR) r $(LIB) $(OBJS)


clean:
	$(RM) r $(OBJS) $(LIB)
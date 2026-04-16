CC = gcc
CFLAGS = -Wall -g

TARGET = k0

SRCS = main.c tree.c symtab.c type.c k0gram.tab.c lex.yy.c token.c
OBJS = $(SRCS:.c=.o)

TAC_TARGET = tacprog
TAC_SRCS = tacmain.c tac.c
TAC_OBJS = $(TAC_SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c tree.h token.h k0gram.tab.h
	$(CC) $(CFLAGS) -c $<

k0gram.tab.c k0gram.tab.h: k0gram.y
	bison -d k0gram.y

lex.yy.c: k0lex.l k0gram.tab.h
	flex k0lex.l

tacrun: $(TAC_TARGET)

$(TAC_TARGET): $(TAC_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TAC_OBJS)

clean:
	rm -f $(OBJS) $(TAC_OBJS) lex.yy.c k0gram.tab.c k0gram.tab.h $(TARGET) $(TAC_TARGET)
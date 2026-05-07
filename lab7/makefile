CC = gcc
CFLAGS = -Wall -g

TARGET = k0
TAC_TARGET = tacprog

SRCS = main.c tree.c symtab.c type.c k0gram.tab.c lex.yy.c token.c
OBJS = $(SRCS:.c=.o)

TAC_SRCS = tacmain.c tac.c
TAC_OBJS = $(TAC_SRCS:.c=.o)

all: $(TARGET)

#main compiler
$(TARGET): $(OBJS) tac.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) tac.o

#tac program
$(TAC_TARGET): $(TAC_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TAC_OBJS)

#object rules
%.o: %.c tree.h token.h k0gram.tab.h
	$(CC) $(CFLAGS) -c $<

tac.o: tac.c tac.h
	$(CC) $(CFLAGS) -c tac.c

# lex/yacc
k0gram.tab.c k0gram.tab.h: k0gram.y
	bison -d k0gram.y

lex.yy.c: k0lex.l k0gram.tab.h
	flex k0lex.l

#clean
clean:
	rm -f $(OBJS) $(TAC_OBJS) \
	lex.yy.c k0gram.tab.c k0gram.tab.h \
	$(TARGET) $(TAC_TARGET)
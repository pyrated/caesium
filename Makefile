CC = gcc

LIB_DIR = lib
BIN_DIR = bin
INC_DIR = include

CFLAGS = -Iinclude/ -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -std=gnu99
LDFLAGS = -L$(LIB_DIR)/

CS_DIR = caesium
CS_SRCS = $(wildcard caesium/*.c)
CS_OBJS = $(CS_SRCS:.c=.o)

TINYC_DIR = $(LIB_DIR)/tinycthread
TINYC_SRCS = $(wildcard $(TINYC_DIR)/*.c)
TINYC_OBJS = $(TINYC_SRCS:.c=.o)

all: caesium

caesium: tinycthread $(CS_OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/caesium $(CS_OBJS) -ltinycthread -lpthread -lrt

tinycthread: $(LIB_DIR)/libtinycthread.a
$(LIB_DIR)/libtinycthread.a: $(TINYC_OBJS)
	$(AR) rcsv $(LIB_DIR)/libtinycthread.a $(TINYC_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/* $(LIB_DIR)/*.a *.o
	rm -f $(addprefix $(CS_DIR)/,*.o)
	rm -f $(addprefix $(TINYC_DIR)/,*.o)

test: libcs
	@$(MAKE) -C $(LIBCS_DIR)/tests test
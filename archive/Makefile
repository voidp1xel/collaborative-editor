CC = gcc

# Detect Homebrew prefix (macOS with Apple Silicon or Intel)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)

CFLAGS = -g -Wall -Wextra -std=c99
LDLIBS_COMMON = -lncurses

ifdef BREW_PREFIX
    CFLAGS += -I$(BREW_PREFIX)/include
    LDLIBS_COMMON := -L$(BREW_PREFIX)/lib $(LDLIBS_COMMON)
endif

# Client needs curl and jansson for AI chat
LDLIBS_CLIENT = $(LDLIBS_COMMON) -lcurl -ljansson

# Server only needs ncurses
LDLIBS_SERVER = $(LDLIBS_COMMON)


# --- CLIENT ---
CLIENT_TARGET = collaborative_editor
CLIENT_SRCS = main.c \
              network_client.c \
              input.c \
              display.c \
              ai_chat.c \
              document.c \
              text_buffer.c

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)


# --- SERVER ---
SERVER_TARGET = server
SERVER_SRCS = server.c \
              auth.c \
              document.c \
              text_buffer.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)


# --- BUILD RULES ---
.PHONY: all clean client build_server

all: client build_server

client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	@echo "==> Linking Client..."
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS) $(LDLIBS_CLIENT)
	@echo "==> Build complete: ./$@"

build_server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	@echo "==> Linking Server..."
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDLIBS_SERVER)
	@echo "==> Build complete: ./$@"

%.o: %.c
	@echo "==> Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "==> Cleaning up..."
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET) *.o

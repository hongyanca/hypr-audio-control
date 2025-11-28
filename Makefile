CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags gtk4 gtk4-layer-shell-0)
LDFLAGS = $(shell pkg-config --libs gtk4 gtk4-layer-shell-0)

TARGET = hypr-audio-control
SRCDIR = src
SRCS = $(SRCDIR)/main.c $(SRCDIR)/audio.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

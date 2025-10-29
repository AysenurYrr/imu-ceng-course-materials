CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk4)
LIBS = $(shell pkg-config --libs gtk4)
TARGET = msweeper
SRCS = main.c model.c view.c controller.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Ubuntu/Debian için GTK4 kurulumu
install-deps:
	sudo apt-get update
	sudo apt-get install -y libgtk-4-dev pkg-config 
# Compiler ve flag'ler
CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread -lSDL2

# Kaynak dosyaları
SRCS = list.c view.c survivor.c drone.c map.c ai.c controller.c
OBJS = $(SRCS:.c=.o)

# Hedef program
TARGET = drone_simulator

# Varsayılan hedef
all: $(TARGET)

# Program oluşturma
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Nesne dosyalarını oluşturma
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Temizleme
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
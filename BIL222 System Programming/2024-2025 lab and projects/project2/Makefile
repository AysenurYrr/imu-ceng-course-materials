# Compiler ve flag'ler
CC = gcc
CFLAGS = -Wall -Werror -g -pthread
LDFLAGS = -lm -ljson-c -luuid -lSDL2 -lwebsockets -pthread

# Build klasörü
BUILD_DIR = build

# Kaynak dosyaları
SERVER_SOURCES = server.c drone_manager.c json_helper.c network.c server_ai.c survivor.c map.c list.c view.c globals.c websocket_server.c http_server.c
CLIENT_SOURCES = basic_drone_client.c drone_client.c json_helper.c network.c list.c globals.c

SERVER_OBJECTS = $(addprefix $(BUILD_DIR)/, $(SERVER_SOURCES:.c=.o))
CLIENT_OBJECTS = $(addprefix $(BUILD_DIR)/, $(CLIENT_SOURCES:.c=.o))

# Varsayılan hedef
all: $(BUILD_DIR) drone_server drone_client
	@echo "Drone Server ve Drone Client oluşturuldu."

# Build klasörünü oluştur
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Program oluşturma
drone_server: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJECTS) $(LDFLAGS)

drone_client: $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJECTS) $(LDFLAGS)

# Nesne dosyalarını oluşturma
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Temizleme
clean:
	rm -rf $(BUILD_DIR) drone_server drone_client view

.PHONY: all clean

install-deps:
	apt-get update && apt-get install -y libjson-c-dev uuid-dev libsdl2-dev libwebsockets-dev

# Bağımlılıkları yönet
$(BUILD_DIR)/server.o: server.c headers/drone_manager.h headers/server_ai.h headers/survivor.h headers/globals.h headers/protocol.h headers/map.h headers/websocket_server.h headers/http_server.h
$(BUILD_DIR)/drone_manager.o: drone_manager.c headers/drone_manager.h headers/network.h headers/json_helper.h headers/protocol.h headers/globals.h
$(BUILD_DIR)/json_helper.o: json_helper.c headers/json_helper.h headers/drone.h headers/survivor.h headers/protocol.h
$(BUILD_DIR)/network.o: network.c headers/network.h
$(BUILD_DIR)/server_ai.o: server_ai.c headers/drone_manager.h headers/survivor.h headers/globals.h headers/protocol.h
$(BUILD_DIR)/survivor.o: survivor.c headers/survivor.h headers/map.h headers/globals.h headers/list.h
$(BUILD_DIR)/map.o: map.c headers/map.h headers/list.h headers/globals.h
$(BUILD_DIR)/list.o: list.c headers/list.h
$(BUILD_DIR)/view.o: view.c headers/drone.h headers/map.h headers/survivor.h headers/globals.h
$(BUILD_DIR)/basic_drone_client.o: basic_drone_client.c headers/drone_client.h headers/protocol.h
$(BUILD_DIR)/drone_client.o: drone_client.c headers/drone_client.h headers/network.h headers/json_helper.h headers/protocol.h
$(BUILD_DIR)/globals.o: globals.c headers/globals.h headers/map.h
$(BUILD_DIR)/websocket_server.o: websocket_server.c headers/websocket_server.h headers/drone_client.h headers/json_helper.h
$(BUILD_DIR)/http_server.o: http_server.c headers/http_server.h
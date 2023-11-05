# Compiler and its settings
GXX=g++
GXXFLAGS=-std=c++17 -Wall -Wpedantic -Wextra -Wconversion -Werror

# Source and object directories
SRC_DIR=./src
OBJ_DIR=./obj

# compiles client/room
default:
	mkdir -p $(OBJ_DIR)
	make all

all: obj/main.o obj/CLInput.o obj/clientClient.o obj/Message.o obj/Music.o obj/MusicStorage.o obj/Player.o obj/serverClient.o obj/Room.o obj/BaseSocket.o  obj/ThreadSafeSocket.o obj/TrackerAPI.o obj/IP.o obj/RoomEntry.o
	$(GXX) $(GXXFLAGS) $^ -o main -lout123 -lmpg123 -lpthread

static: obj/main.o obj/CLInput.o obj/clientClient.o obj/Message.o obj/Music.o obj/MusicStorage.o obj/Player.o obj/serverClient.o obj/Room.o obj/BaseSocket.o  obj/ThreadSafeSocket.o obj/TrackerAPI.o obj/IP.o obj/RoomEntry.o
	$(GXX) $(GXXFLAGS) $^ -o main /usr/local/lib/libout123.a /usr/local/lib/libmpg123.a

# src/main.cpp
obj/main.o: src/main.cpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/CLInput.cpp
obj/CLInput.o: src/CLInput.cpp src/CLInput.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/client
obj/clientClient.o: src/client/Client.cpp src/client/Client.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/messaging
obj/Message.o: src/messaging/Message.cpp src/messaging/Message.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/music
obj/Music.o: src/music/Music.cpp src/music/Music.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/MusicStorage.o: src/music/MusicStorage.cpp src/music/MusicStorage.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/Player.o: src/music/Player.cpp src/music/Player.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/room
obj/serverClient.o: src/room/Client.cpp src/room/Client.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/Room.o: src/room/Room.cpp src/room/Room.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/socket
obj/BaseSocket.o: src/socket/BaseSocket.cpp src/socket/BaseSocket.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/socket
obj/ThreadSafeSocket.o: src/socket/ThreadSafeSocket.cpp src/socket/ThreadSafeSocket.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/IP.o: src/socket/IP.cpp src/socket/IP.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

clean:
	cd src/tracker/ && make clean
	rm -rf $(OBJ_DIR) main

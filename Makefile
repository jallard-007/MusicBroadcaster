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

all: obj/main.o obj/CLInput.o obj/clientClient.o obj/Message.o obj/Music.o obj/MusicStorage.o obj/Player.o obj/serverClient.o obj/Room.o obj/BaseSocket.o obj/ThreadSafeSocket.o
	$(GXX) $(GXXFLAGS) $^ -o main -lao -lmpg123

# src/main.cpp
obj/main.o: src/main.cpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/CLInput.cpp
obj/CLInput.o: src/CLInput.cpp src/CLInput.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/client
obj/clientClient.o: src/client/Client.cpp src/client/Client.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/messaging
obj/Message.o: src/messaging/Message.cpp src/messaging/Message.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/music
obj/Music.o: src/music/Music.cpp src/music/Music.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/MusicStorage.o: src/music/MusicStorage.cpp src/music/MusicStorage.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/Player.o: src/music/Player.cpp src/music/Player.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/room
obj/serverClient.o: src/room/Client.cpp src/room/Client.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

obj/Room.o: src/room/Room.cpp src/room/Room.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/socket
obj/BaseSocket.o: src/socket/BaseSocket.cpp src/socket/BaseSocket.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

# src/socket
obj/ThreadSafeSocket.o: src/socket/ThreadSafeSocket.cpp src/socket/ThreadSafeSocket.hpp src/debug.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

clean:
	rm -r -f $(OBJ_DIR) main

# OBJ location
OBJ_DIR=../../obj

GXX=g++
GXXFLAGS=-std=c++17 -Wall -Wpedantic -Wextra -Wconversion

export tracker

$(shell mkdir -p $(OBJ_DIR))

tracker: tracker.cpp $(OBJ_DIR)/Message.o $(OBJ_DIR)/RoomEntry.o $(OBJ_DIR)/ListenSocket.o $(OBJ_DIR)/BaseSocket.o $(OBJ_DIR)/IP.o
	$(GXX) $(GXXFLAGS) $^ -o $@ -lpthread

$(OBJ_DIR)/Message.o: ../messaging/Message.cpp
	$(GXX) $(GXXFLAGS) -c $^ -o $@

$(OBJ_DIR)/RoomEntry.o: RoomEntry.cpp
	$(GXX) $(GXXFLAGS) -c $^ -o $@

$(OBJ_DIR)/ListenSocket.o: ListenSocket.cpp $(OBJ_DIR)/BaseSocket.o
	$(GXX) $(GXXFLAGS) -c $^ -o $@

$(OBJ_DIR)/BaseSocket.o: ../socket/BaseSocket.cpp ../socket/BaseSocket.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

$(OBJ_DIR)/IP.o: ../socket/IP.cpp ../socket/IP.hpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

clean:
	# Copied the rm list from the files that tracker depends on
	rm -rf ./tracker $(OBJ_DIR)/Message.o $(OBJ_DIR)/RoomEntry.o $(OBJ_DIR)/ListenSocket.o $(OBJ_DIR)/BaseSocket.o
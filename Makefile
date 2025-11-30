# Autor: Petr Špác (xspacpe00)
CC=g++
CXXFLAGS=-I$(IDIR) -std=c++20 -MMD -MP
DFLAGS=
LDFLAGS=

IDIR=./include
SRC_DIR=./src
BUILD_DIR=./build

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))


.PHONY: all run debug clean clear

all: ca

# $@ = target
# $^ = all prerequisites
# $< = first prerequisite

ca: $(OBJECTS)
	$(CC) $(CXXFLAGS) -o $@ $^ $(DFLAGS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@ $(DFLAGS) $(LDFLAGS)

-include $(BUILD_DIR)/*.d

run: ca
	./ca -d 0.09 -a 0 -w 1000 -s 5000

debug: DFLAGS += -g -DDEBUG_PRINT -Wconversion -Wall -Wextra -Werror
debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
debug: ca

clean:
clear:
	rm -f ca $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d

zip:
	zip -r T8_xspacpe00.zip dokumentace.pdf ./Makefile ./src ./include ./scripts
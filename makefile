# compiler
CC = g++

#compiler flags
CFLAGS = -Wall 
LIBS = -lncurses -lSDL2 

#build target
TARGET_NAME = game

#directorys
BUILD_DIR = build
SRC_DIR = src

#Build target
TARGET = $(BUILD_DIR)/$(TARGET_NAME)


#Source files
SRC =$(shell find $(SRC_DIR) -name '*.cpp')

#Build commands
REMOVE_BUILD = $(shell rm -r $(BUILD_DIR))
BUILD = $(shell mkdir -p $(BUILD_DIR)) $(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
BUILD_DEBUG = $(shell mkdir -p $(BUILD_DIR)) $(CC) $(CFLAGS) -g3 -o $(TARGET) $(SRC) $(LIBS)


default:
	$(BUILD)

debug: 
	$(BUILD_DEBUG)

new: 
	$(REMOVE_BUILD)
	$(BUILD)

.PHONY: clean

clean: 
	$(REMOVE_BUILD)

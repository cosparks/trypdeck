CC = g++
CFLAGS = -g -Wall
VERSION = -std=c++17
EXTRA_CFLAGS = -pthread
PIGPIO = -lpigpio -lrt
CLIBS =  $(pkg-config --cflags libvlc)
LIBS = $(PIGPIO) $(pkg-config --libs libvlc) -lvlc
INCLUDE = -I
PARENT_DIRECTORY = ..
SRC_DIR = $(PARENT_DIRECTORY)/src/
BUILD_DIR = $(PARENT_DIRECTORY)/obj/
LIB_DIR = $(PARENT_DIRECTORY)/lib/

# Get all directories from lib
INCLUDE_DIRS_LIB = $(wildcard $(LIB_DIR)*/src/)
INCLUDE_DIRS += $(INCLUDE_DIRS_LIB)
# Add lib cpp/c files and get their .h dependencies
CPPSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.cpp)))
CSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.h)))

# Get all directories in src
INCLUDE_DIRS_SRC = $(sort $(dir $(wildcard $(SRC_DIR)*/)))
INCLUDE_DIRS += $(INCLUDE_DIRS_SRC)
# Add sub-src directory files and get their .h dependencies
CPPSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.cpp)))
CSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.h)))

# Create flags to include all directories (so we don't have to use paths in #include)
INCLUDE_FLAGS := $(foreach %,$(INCLUDE_DIRS),$(INCLUDE)$(wildcard $(%)))

# Add all src-level files
CPPSRC += $(patsubst /%,%,$(foreach %,$(SRC_DIR),$(wildcard $(%)*.cpp)))
CSRC += $(patsubst /%,%,$(foreach %,$(SRC_DIR),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(SRC_DIR),$(wildcard $(%)*.h)))

_OBJ = $(CPPSRC:.cpp=.o) $(CSRC:.c=.o)

# try to get object files into separate folder
# OBJ = $(patsubst ../%,$(BUILD_DIR)%,$(patsubst ,%.cpp,$(_OBJ)))
# $(TARGET)%.o: %.cpp $(DEPENDENCIES)
# 	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDE_FLAGS)

%.o: %.cpp $(DEPENDENCIES)
	$(CC) -c -o $@ $< $(CFLAGS) $(CLIBS) $(INCLUDE_FLAGS)

tripdeck: $(_OBJ)
	$(CC) $(VERSION) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS) 

.PHONY: clean

clean : $(_OBJ)
	rm -r $(_OBJ)
CC = g++
VERSION = -std=c++17
CFLAGS = -g -Wall -Wno-psabi
LFLAGS = -pthread
LIBS_CARGS =  $(pkg-config --cflags libvlc)
LIBS = -lpigpio -lrt $(pkg-config --libs libvlc) -lvlc -lavcodec -lavformat -lavutil -lswscale -lx264
INCLUDE = -I
PARENT_DIRECTORY = ..
SRC_DIR = $(PARENT_DIRECTORY)/src/
TEST_DIR = $(PARENT_DIRECTORY)/test/
BUILD_DIR = $(PARENT_DIRECTORY)/obj/
LIB_DIR = $(PARENT_DIRECTORY)/lib/

# Get all directories from lib
INCLUDE_DIRS_LIB += $(wildcard $(LIB_DIR)**/src/)
INCLUDE_DIRS += $(INCLUDE_DIRS_LIB)
# Add lib cpp/c files and get their .h dependencies
CPPSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.cpp)))
CSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.h)))

# Get all directories in src
INCLUDE_DIRS_SRC = $(sort $(dir $(wildcard $(SRC_DIR)*/*)))
INCLUDE_DIRS += $(INCLUDE_DIRS_SRC)
# Add sub-src directory files and get their .h dependencies
CPPSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.cpp)))
CSRC += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.h)))

# add header files directly contained in src directory (workaround so we can exclude main.cpp in src)
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(SRC_DIR),$(wildcard $(%)*.h)))
INCLUDE_DIRS += $(SRC_DIR) $(TEST_DIR)

# Create flags to include all directories (so we don't have to use paths in #include)
INCLUDE_FLAGS := $(foreach %,$(INCLUDE_DIRS),$(INCLUDE)$(wildcard $(%)))

# Add all test-level files
CPPTEST := $(patsubst /%,%,$(foreach %,$(TEST_DIR),$(wildcard $(%)*.cpp)))
CTEST += $(patsubst /%,%,$(foreach %,$(TEST_DIR),$(wildcard $(%)*.c)))
DEPENDENCIES += $(patsubst /%,%,$(foreach %,$(TEST_DIR) ,$(wildcard $(%)*.h)))

OBJ_SRC := $(patsubst $(SRC_DIR)%,$(BUILD_DIR)%, $(CPPSRC:.cpp=.o) $(CSRC:.c=.o))
OBJ := $(patsubst $(LIB_DIR)%,$(BUILD_DIR)%, $(OBJ_SRC))

# pattern rules for src, test and lib files
$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp $(DEPENDENCIES)
	@mkdir -p $(@D)
	$(CC) $(LIBS_CARGS) -c -o $@ $< $(CFLAGS) $(INCLUDE_FLAGS)

$(BUILD_DIR)%.o: $(TEST_DIR)%.cpp $(DEPENDENCIES)
	@mkdir -p $(@D)
	$(CC) $(LIBS_CARGS) -c -o $@ $< $(CFLAGS) $(INCLUDE_FLAGS)

$(BUILD_DIR)%.o: $(LIB_DIR)%.cpp $(DEPENDENCIES)
	@mkdir -p $(@D)
	$(CC) $(LIBS_CARGS) -c -o $@ $< $(CFLAGS) $(INCLUDE_FLAGS)

# build with debug info
sandbox: $(OBJ) $(BUILD_DIR)sandbox.o
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

test-libvlc: $(OBJ) $(BUILD_DIR)test-libvlc.o
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

all: sandbox test-libvlc

.PHONY: clean

clean:
	rm -r $(OBJ) sandbox test-libvlc
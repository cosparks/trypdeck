CC = g++
VERSION = -std=c++17
CFLAGS = -Wall -Wno-psabi
LFLAGS = -pthread
LIBS_CARGS =  $(pkg-config --cflags libvlc)
LIBS = -lpigpio -lrt $(pkg-config --libs libvlc) -lvlc -lavcodec -lavformat -lavutil -lswscale -lx264
INCLUDE_PREFIX = -I
PARENT_DIRECTORY = ..
BIN_DIR = $(PARENT_DIRECTORY)/bin/
SRC_DIR = $(PARENT_DIRECTORY)/src/
BUILD_DIR = $(PARENT_DIRECTORY)/obj/
LIB_DIR = $(PARENT_DIRECTORY)/lib/
DEP_DIR = $(PARENT_DIRECTORY)/dep/
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)$*.d

# Get all directories from lib and add lib files
INCLUDE_DIRS_LIB += $(wildcard $(LIB_DIR)**/src/)
INCLUDE_DIRS += $(INCLUDE_DIRS_LIB)
CPPSRC += $(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.cpp))

# Get all directories in src
INCLUDE_DIRS_SRC = $(sort $(dir $(wildcard $(SRC_DIR)*/)))
ifneq ($(filter $(SRC_DIR),$(INCLUDE_DIRS_SRC)),)
else
    INCLUDE_DIRS_SRC += $(SRC_DIR)
endif

# Add src folders and directory files
INCLUDE_DIRS += $(INCLUDE_DIRS_SRC)
CPPSRC += $(foreach %,$(INCLUDE_DIRS_SRC),$(wildcard $(%)*.cpp))

# Create flags to include all directories (so we don't have to use paths in #include)
INCLUDE_FLAGS := $(foreach %,$(INCLUDE_DIRS),$(INCLUDE_PREFIX)$(wildcard $(%)))

# generate object / dependency file paths for source and library files
OBJ_SRC := $(patsubst $(SRC_DIR)%,$(BUILD_DIR)%, $(CPPSRC:.cpp=.o))
OBJ = $(patsubst $(LIB_DIR)%,$(BUILD_DIR)%, $(OBJ_SRC))
DEPENDENCIES := $(patsubst $(BUILD_DIR)%.o,$(DEP_DIR)%.d,$(OBJ))

# rules for generating object / dependency files
$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp $(DEP_DIR)%.d | $(DEP_DIR)
	@mkdir -p $(@D)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(LIBS_CARGS) -c -o $@ $< $(INCLUDE_FLAGS)

$(BUILD_DIR)%.o: $(LIB_DIR)%.cpp $(DEP_DIR)%.d | $(DEP_DIR)
	@mkdir -p $(@D)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(LIBS_CARGS) -c -o $@ $< $(INCLUDE_FLAGS)

$(DEP_DIR)%.d:
	@mkdir -p $(@D)

tripdeck: $(OBJ)
	$(CC) $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

debug: $(OBJ)
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR)$@ $^ $(LIBS) $(INCLUDE_FLAGS)

.PHONY: clean
 
clean:
	rm -r $(OBJ) $(DEPENDENCIES) tripdeck

-include $(DEPENDENCIES)
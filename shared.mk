CC = g++
VERSION = -std=c++17
CFLAGS = -Wall -Wno-psabi
LFLAGS = -pthread
LIBS_CARGS =  $(pkg-config --cflags libvlc)
LIBS = -lpigpio -lrt $(pkg-config --libs libvlc) -lvlc -lavcodec -lavformat -lavutil -lswscale -lx264
INCLUDE_PREFIX = -I
PARENT_DIRECTORY = ../
BIN_DIR = $(PARENT_DIRECTORY)bin/
SRC_DIR = $(PARENT_DIRECTORY)src/
BUILD_DIR = $(PARENT_DIRECTORY)obj/
LIB_DIR = $(PARENT_DIRECTORY)lib/
DEP_DIR = $(PARENT_DIRECTORY)dep/
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)$*.d

# Get all directories from lib and add lib files
INCLUDE_DIRS_LIB += $(wildcard $(LIB_DIR)**/src/)
INCLUDE_DIRS += $(INCLUDE_DIRS_LIB)
CPPSRC += $(foreach %,$(INCLUDE_DIRS_LIB),$(wildcard $(%)*.cpp))

# Get all directories in src and add to includes
SRC_DIRS = $(sort $(dir $(wildcard $(SRC_DIR)*/)))
ifeq ($(filter $(SRC_DIR),$(SRC_DIRS)),)
	INCLUDE_DIRS += $(SRC_DIRS)
	INCLUDE_DIRS += $(SRC_DIR)
else
	INCLUDE_DIRS += $(SRC_DIRS)
endif

SRC_SUB_DIRS = $(filter-out $(SRC_DIR),$(SRC_DIRS))

# Add src folders and directory files
CPPSRC += $(foreach %,$(SRC_SUB_DIRS),$(wildcard $(%)*.cpp))

# Create flags to include all directories (so we don't have to use paths in #include)
INCLUDE_FLAGS := $(foreach %,$(INCLUDE_DIRS),$(INCLUDE_PREFIX)$(wildcard $(%)))

# generate object / dependency file paths for source and library files
OBJ_SRC := $(patsubst $(PARENT_DIRECTORY)%,$(BUILD_DIR)%,$(CPPSRC:.cpp=.o))
OBJ = $(patsubst $(LIB_DIR)%,$(BUILD_DIR)%,$(OBJ_SRC))
DEPENDENCIES := $(patsubst $(BUILD_DIR)%.o,$(DEP_DIR)%.d,$(OBJ))

# rules for generating object / dependency files
$(BUILD_DIR)%.o: $(PARENT_DIRECTORY)%.cpp $(DEP_DIR)%.d | $(DEP_DIR)
	@echo ' *** compiling $< *** '
	@mkdir -p $(@D)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(LIBS_CARGS) -c -o $@ $< $(INCLUDE_FLAGS)

$(DEP_DIR)%.d:
	@mkdir -p $(@D)

-include $(DEPENDENCIES)
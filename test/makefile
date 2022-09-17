include ../shared.mk

# pattern rule for test obj files
$(BUILD_DIR)%.o: $(TEST_DIR)%.cpp $(DEP_DIR)%.d | $(DEP_DIR)
	@mkdir -p $(@D)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(LIBS_CARGS) -c -o $@ $<  $(INCLUDE_FLAGS)

# build with debug info
sandbox: $(OBJ) $(BUILD_DIR) sandbox.o
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

test-libvlc: $(OBJ) $(BUILD_DIR) test-libvlc.o
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

all: sandbox test-libvlc

.PHONY: clean

clean:
	rm -r $(OBJ) $(DEPENDENCIES) sandbox test-libvlc
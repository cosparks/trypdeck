include ../shared.mk

LED_CONFIG_TEST_OBJ = $(BUILD_DIR)test/led-config-test.o
SANDBOX_OBJ = $(BUILD_DIR)test/sandbox.o
TEST_LIBVLC_OBJ = $(BUILD_DIR)test/test-libvlc.o

# build with debug info
led-config-test: $(OBJ) $(LED_CONFIG_TEST_OBJ)
	@echo ' *** building $@ *** '
	@$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

sandbox: $(OBJ) $(SANDBOX_OBJ)
	@echo ' *** building $@ *** '
	@$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

test-libvlc: $(OBJ) $(TEST_LIBVLC_OBJ)
	@echo ' *** building $@ *** '
	@$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

all: sandbox test-libvlc led-config-test

.PHONY: clean

clean:
	rm -r $(OBJ) $(DEPENDENCIES) sandbox test-libvlc led-config-test
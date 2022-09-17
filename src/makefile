include ../shared.mk

OBJ += $(BUILD_DIR)src/tripdeck.o

tripdeck: $(OBJ) 
	@echo ' *** building $@ *** '
	@$(CC) $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

debug: $(OBJ)
	@echo ' *** building $@ *** '
	@$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR) $@ $^ $(LIBS) $(INCLUDE_FLAGS)

.PHONY: clean
 
clean:
	@rm -r $(OBJ) $(DEPENDENCIES) tripdeck
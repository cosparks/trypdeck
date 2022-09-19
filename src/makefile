include ../shared.mk

OBJ += $(BUILD_DIR)src/trypdeck.o

trypdeck: $(OBJ) 
	@echo ' *** building $@ *** '
	@$(CC) $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

debug: $(OBJ)
	@echo ' *** building $@ *** '
	@$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR) $@ $^ $(LIBS) $(INCLUDE_FLAGS)

.PHONY: clean
 
clean:
	@rm -r $(OBJ) $(DEPENDENCIES) trypdeck
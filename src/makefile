include ../shared.mk

tripdeck: $(OBJ) tripdeck.o
	$(CC) $(VERSION) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LIBS) $(INCLUDE_FLAGS)

debug: $(OBJ)
	$(CC) -g $(VERSION) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR) $@ $^ $(LIBS) $(INCLUDE_FLAGS)

.PHONY: clean
 
clean:
	rm -r $(OBJ) $(DEPENDENCIES) tripdeck
COMPILER := g++
CFLAGS := --std=c++17 -g -Wall
EMCCFLAGS := -sUSE_SDL=2 --emrun -lembind

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
DIST_DIR := dist

BIN := app
DIST := app

DIST_EXTENSION := html

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY := all clean

all: app

$(OBJ_DIR)/%.o: $(SRC_FILES)
	@mkdir -p $(@D)
	$(Q)$(COMPILER) $(CFLAGS) -c $< -o $@

app: $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(Q)$(COMPILER) $(CFLAGS) -o $(BIN_DIR)/$(BIN) $(OBJ_FILES)

run: app
	$(BIN_DIR)/$(BIN)

wasm: COMPILER := emcc
wasm:
	@mkdir -p $(DIST_DIR)
	$(Q)$(COMPILER) $(EMCCFLAGS) $(SRC_FILES) -o $(DIST_DIR)/$(DIST).$(DIST_EXTENSION)

wasm_run: wasm
	emrun $(DIST_DIR)/$(DIST).html

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DIST_DIR)

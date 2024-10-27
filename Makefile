COMPILER := g++
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 sdl2_image sdl2_ttf)
SDL_LIBS := $(shell pkg-config --libs sdl2 sdl2_image sdl2_ttf)

LDFLAGS = $(SDL_LIBS)

CFLAGS := --std=c++17 -g -Wall $(SDL_CFLAGS) -I$(ROOT_DIR)src
EMCCFLAGS := -sUSE_SDL=2 --emrun -lembind

SRC_DIR := $(ROOT_DIR)src
OBJ_DIR := $(ROOT_DIR)obj
BIN_DIR := $(ROOT_DIR)bin
DIST_DIR := $(ROOT_DIR)dist

BIN := app
DIST := app

DIST_EXTENSION := html

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY := all clean bear

all: app wasm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -c $< -o $@

app: $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(COMPILER) $(CFLAGS) -o $(BIN_DIR)/$(BIN) $(OBJ_FILES) $(LDFLAGS)

run: app
	$(BIN_DIR)/$(BIN)

wasm: COMPILER := emcc
wasm: $(SRC_FILES)
	@mkdir -p $(DIST_DIR)
	$(COMPILER) $(EMCCFLAGS) -I$(ROOT_DIR)src $(SRC_FILES) -o $(DIST_DIR)/$(DIST).$(DIST_EXTENSION)

wasm_run: wasm
	emrun $(DIST_DIR)/$(DIST).html

bear: clean
	bear -- make all

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DIST_DIR)

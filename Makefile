.PHONY: all clean build rebuild protoc run-svr run-cli test

# Build configuration
BUILD_DIR := build
DEBUG ?= false

# Determine build type
ifeq ($(DEBUG),true)
	CMAKE_BUILD_TYPE := Debug
else
	CMAKE_BUILD_TYPE := Release
endif

# CMake configuration
CMAKE_OPTIONS := -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=1

all: build

clean:
	rm -rf $(BUILD_DIR)

protoc:
	./protoc.sh

configure:
	mkdir -p $(BUILD_DIR)
	cmake $(CMAKE_OPTIONS) -B $(BUILD_DIR) .

build: configure
	cmake --build $(BUILD_DIR)

rebuild: clean protoc build

run-svr:
	./run-svr.sh

run-cli:
	./run-cli.sh

test:
	./test.sh

# Debug build target
debug:
	$(MAKE) DEBUG=true build

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build the project (default target)"
	@echo "  clean      - Remove build directory"
	@echo "  build      - Build the project"
	@echo "  rebuild    - Clean, generate protos, and rebuild"
	@echo "  protoc     - Generate protobuf files"
	@echo "  run-svr    - Run the server"
	@echo "  run-cli    - Run the client"
	@echo "  test       - Run tests"
	@echo "  debug      - Build in debug mode"
	@echo "  help       - Show this help message"



.PHONY: all clean build rebuild run test

# Build configuration
BUILD_DIR := ../build/rsp-cli
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

configure:
	mkdir -p $(BUILD_DIR)
	cmake $(CMAKE_OPTIONS) -B $(BUILD_DIR) .

build: configure
	cmake --build $(BUILD_DIR)

rebuild: clean build

run:
	./run.sh

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
	@echo "  rebuild    - Clean and rebuild"
	@echo "  run        - Run the client"
	@echo "  test       - Run tests"
	@echo "  debug      - Build in debug mode"
	@echo "  help       - Show this help message"
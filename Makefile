# Wrapper Makefile di atas CMake.
# Semua logika build sebenarnya berada di CMakeLists.txt.

BUILD_DIR ?= build
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)

.PHONY: all configure build test debug release clean format rebuild

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

test:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DTINYNES_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR) --parallel $(JOBS)
	ctest --test-dir $(BUILD_DIR) --output-on-failure

debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DTINYNES_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

release:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		find include src tests -name '*.c' -o -name '*.h' | xargs clang-format -i; \
	else \
		echo "clang-format tidak ditemukan. Pasang paket clang-format terlebih dahulu."; \
		exit 1; \
	fi

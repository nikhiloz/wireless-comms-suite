# Makefile — Wireless Communication Systems Tutorial Suite
# Requirements: gcc/clang, make
# Mirrors dsp-tutorial-suite conventions

CC ?= gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -Iinclude -fPIC
CFLAGS_DEBUG := $(CFLAGS) -g -O0 -DDEBUG
CFLAGS_RELEASE := $(CFLAGS) -O3 -DNDEBUG
LDFLAGS := -lm

# Build directories
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
LIB_DIR := $(BUILD_DIR)/lib
OBJ_DIR := $(BUILD_DIR)/obj

# Source files (all 9 modules)
SOURCES := src/comms_utils.c \
	src/modulation.c \
	src/coding.c \
	src/channel.c \
	src/sync.c \
	src/ofdm.c \
	src/spread_spectrum.c \
	src/equaliser.c \
	src/phy.c \
	src/analog_demod.c
OBJECTS := $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Tests
TESTS := tests/test_modulation.c \
	tests/test_coding.c \
	tests/test_channel.c \
	tests/test_sync.c \
	tests/test_ofdm.c \
	tests/test_spread.c \
	tests/test_equaliser.c \
	tests/test_phy.c

# Chapter demos
CHAPTER_DEMOS := chapters/01-system-overview/demo.c \
	chapters/02-source-coding/demo.c \
	chapters/03-channel-coding/demo.c \
	chapters/04-pulse-shaping/demo.c \
	chapters/05-modulation/demo.c \
	chapters/06-awgn-channel/demo.c \
	chapters/07-fading-channels/demo.c \
	chapters/08-timing-recovery/demo.c \
	chapters/09-carrier-sync/demo.c \
	chapters/10-frame-sync/demo.c \
	chapters/11-convolutional-viterbi/demo.c \
	chapters/12-interleaving/demo.c \
	chapters/13-equalisation/demo.c \
	chapters/14-ofdm/demo.c \
	chapters/15-spread-spectrum/demo.c \
	chapters/16-wifi-phy/demo.c \
	chapters/17-bluetooth-baseband/demo.c \
	chapters/18-zigbee-phy/demo.c \
	chapters/19-lora-phy/demo.c \
	chapters/20-adsb-phy/demo.c \
	chapters/21-link-budget/demo.c \
	chapters/22-ber-simulation/demo.c \
	chapters/23-mimo/demo.c \
	chapters/24-transceiver/demo.c \
	chapters/25-fm-broadcast/demo.c

# ── Default target ────────────────────────────────────────────────
all: release

# ── Directory creation ────────────────────────────────────────────
$(BUILD_DIR) $(BIN_DIR) $(LIB_DIR) $(OBJ_DIR):
	mkdir -p $@

# ── Object compilation ───────────────────────────────────────────
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

# ── Debug build ──────────────────────────────────────────────────
debug: CFLAGS_RELEASE = $(CFLAGS_DEBUG)
debug: lib chapters_build tests_build

# ── Release build ────────────────────────────────────────────────
release: lib chapters_build tests_build

# ── Static library ───────────────────────────────────────────────
lib: $(LIB_DIR)/libwireless_comms.a

$(LIB_DIR)/libwireless_comms.a: $(OBJECTS) | $(LIB_DIR)
	ar rcs $@ $^

# ── Shared library ───────────────────────────────────────────────
$(LIB_DIR)/libwireless_comms.so: $(OBJECTS) | $(LIB_DIR)
	$(CC) -shared -fPIC $(OBJECTS) $(LDFLAGS) -o $@

# ── Chapter demos ────────────────────────────────────────────────
chapters_build: $(patsubst chapters/%/demo.c, $(BIN_DIR)/%, $(CHAPTER_DEMOS))

$(BIN_DIR)/01-system-overview: chapters/01-system-overview/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/02-source-coding: chapters/02-source-coding/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/03-channel-coding: chapters/03-channel-coding/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/04-pulse-shaping: chapters/04-pulse-shaping/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/05-modulation: chapters/05-modulation/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/06-awgn-channel: chapters/06-awgn-channel/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/07-fading-channels: chapters/07-fading-channels/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/08-timing-recovery: chapters/08-timing-recovery/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/09-carrier-sync: chapters/09-carrier-sync/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/10-frame-sync: chapters/10-frame-sync/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/11-convolutional-viterbi: chapters/11-convolutional-viterbi/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/12-interleaving: chapters/12-interleaving/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/13-equalisation: chapters/13-equalisation/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/14-ofdm: chapters/14-ofdm/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/15-spread-spectrum: chapters/15-spread-spectrum/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/16-wifi-phy: chapters/16-wifi-phy/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/17-bluetooth-baseband: chapters/17-bluetooth-baseband/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/18-zigbee-phy: chapters/18-zigbee-phy/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/19-lora-phy: chapters/19-lora-phy/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/20-adsb-phy: chapters/20-adsb-phy/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/21-link-budget: chapters/21-link-budget/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/22-ber-simulation: chapters/22-ber-simulation/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/23-mimo: chapters/23-mimo/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/24-transceiver: chapters/24-transceiver/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/25-fm-broadcast: chapters/25-fm-broadcast/demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@


# ── Test binaries ────────────────────────────────────────────────
tests_build: $(BIN_DIR)/test_modulation $(BIN_DIR)/test_coding \
	$(BIN_DIR)/test_channel $(BIN_DIR)/test_sync \
	$(BIN_DIR)/test_ofdm $(BIN_DIR)/test_spread \
	$(BIN_DIR)/test_equaliser $(BIN_DIR)/test_phy \
	$(BIN_DIR)/test_analog_demod

$(BIN_DIR)/test_modulation: tests/test_modulation.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_coding: tests/test_coding.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_channel: tests/test_channel.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_sync: tests/test_sync.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_ofdm: tests/test_ofdm.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_spread: tests/test_spread.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_equaliser: tests/test_equaliser.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_phy: tests/test_phy.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_analog_demod: tests/test_analog_demod.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

# ── Run all tests ────────────────────────────────────────────────
test: tests_build
	@echo "=== Running Modulation tests ==="
	$(BIN_DIR)/test_modulation
	@echo "\n=== Running Coding tests ==="
	$(BIN_DIR)/test_coding
	@echo "\n=== Running Channel tests ==="
	$(BIN_DIR)/test_channel
	@echo "\n=== Running Sync tests ==="
	$(BIN_DIR)/test_sync
	@echo "\n=== Running OFDM tests ==="
	$(BIN_DIR)/test_ofdm
	@echo "\n=== Running Spread Spectrum tests ==="
	$(BIN_DIR)/test_spread
	@echo "\n=== Running Equaliser tests ==="
	$(BIN_DIR)/test_equaliser
	@echo "\n=== Running PHY tests ==="
	$(BIN_DIR)/test_phy
	@echo "\n=== Running Analog Demod tests ==="
	$(BIN_DIR)/test_analog_demod

# ── Run all chapter demos ────────────────────────────────────────
run: chapters_build
	@for demo in $(BIN_DIR)/[0-2]*; do \
		echo "\n=== $$(basename $$demo) ==="; \
		$$demo; \
	done

# ── Code formatting ──────────────────────────────────────────────
format:
	clang-format -i src/*.c include/*.h chapters/*/demo.c tests/test_*.c

# ── Static analysis ──────────────────────────────────────────────
lint:
	clang-tidy src/*.c -- -Iinclude

# ── Memory checking ──────────────────────────────────────────────
memcheck: debug
	@echo "=== Valgrind: test_modulation ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_modulation
	@echo "\n=== Valgrind: test_coding ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_coding
	@echo "\n=== Valgrind: test_channel ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_channel
	@echo "\n=== Valgrind: test_sync ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_sync
	@echo "\n=== Valgrind: test_ofdm ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_ofdm
	@echo "\n=== Valgrind: test_spread ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_spread
	@echo "\n=== Valgrind: test_equaliser ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_equaliser
	@echo "\n=== Valgrind: test_phy ==="
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_phy
	@echo "\n=== All 8 test suites passed memcheck ==="

# ── Clean ────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILD_DIR)

# ── Deep clean ───────────────────────────────────────────────────
distclean: clean
	find . -name "*.o" -o -name "*.a" -o -name "*.so" -o -name "perf.data*" | xargs rm -f

# ── Install ──────────────────────────────────────────────────────
install: release
	@echo "Installing to /usr/local..."
	mkdir -p /usr/local/include/wireless_comms /usr/local/lib
	cp include/*.h /usr/local/include/wireless_comms/
	cp $(LIB_DIR)/* /usr/local/lib/
	ldconfig

# ── Help ─────────────────────────────────────────────────────────
help:
	@echo "Wireless Comms Tutorial Suite Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make release     - Build release version (default)"
	@echo "  make debug       - Build debug version with symbols"
	@echo "  make lib         - Build static library only"
	@echo "  make test        - Run all unit tests"
	@echo "  make run         - Run all chapter demos"
	@echo "  make chapters_build - Build chapter demos only"
	@echo "  make memcheck    - Run tests with valgrind"
	@echo "  make format      - Format code with clang-format"
	@echo "  make lint        - Static analysis with clang-tidy"
	@echo "  make install     - Install headers & libraries to /usr/local"
	@echo "  make clean       - Remove build directory"
	@echo "  make distclean   - Remove all generated files"
	@echo "  make help        - Show this help message"

.PHONY: all debug release lib test run chapters_build tests_build \
	format lint memcheck clean distclean install help

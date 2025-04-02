# compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g

# directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
BUILD_DIR = build

################ EEG APP #################
EEG_SRC = $(SRC_DIR)/read_serial_data.c $(SRC_DIR)/ring_buffer.c $(SRC_DIR)/dsp.c 
EEG_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(EEG_SRC)))
EEG_BIN = $(BUILD_DIR)/eeg_app

################ TESTING #################
UNIT_TEST_SRC = $(TEST_DIR)/unit_test_ring_buffer.c $(SRC_DIR)/ring_buffer.c
EDGE_TEST_SRC = $(TEST_DIR)/edge_test_ring_buffer.c $(SRC_DIR)/ring_buffer.c
STRESS_TEST_SRC = $(TEST_DIR)/stress_test_ring_buffer.c $(SRC_DIR)/ring_buffer.c
UNIT_TEST_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(UNIT_TEST_SRC)))
EDGE_TEST_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(EDGE_TEST_SRC)))
STRESS_TEST_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(STRESS_TEST_SRC)))
TEST_BINS = \
 $(BUILD_DIR)/unit_test_ring_buffer \
 $(BUILD_DIR)/edge_test_ring_buffer \
 $(BUILD_DIR)/stress_test_ring_buffer 

############## BUILD RULES ###############
all: test-all memcheck eeg

test-all: $(TEST_BINS)
	@echo "========== Running all tests ==========="
	@for bin in $(TEST_BINS); do \
	  echo "🧪 Running $$bin..."; \
	  ./$$bin || exit 1; \
	  echo ""; \
	done

$(BUILD_DIR)/unit_test_ring_buffer: $(UNIT_TEST_OBJS)
	$(CC) $(CFLAGS) $(UNIT_TEST_OBJS) -o $@

$(BUILD_DIR)/edge_test_ring_buffer: $(EDGE_TEST_OBJS)
	$(CC) $(CFLAGS) $(EDGE_TEST_OBJS) -o $@

$(BUILD_DIR)/stress_test_ring_buffer: $(STRESS_TEST_OBJS)
	$(CC) $(CFLAGS) $(STRESS_TEST_OBJS) -o $@

memcheck: $(TEST_BINS)
	@for bin in $(TEST_BINS); do \
	echo "🔍 Running memory leak checks with macOS 'leaks' tool for $$bin ..."; \
	leaks --atExit -- $$bin || true; \
	echo "";\
	done

eeg: $(EEG_BIN)

$(EEG_BIN): $(EEG_OBJS)
	$(CC) $(CFLAGS) $(EEG_OBJS) -o $@

# compile each .c file to a corresponding .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# compile each .c file to a corresponding .o file
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# clean build artifacts
clean:
	@rm -rf $(BUILD_DIR)/*



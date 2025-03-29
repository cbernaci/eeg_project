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
UNIT_TEST_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(UNIT_TEST_SRC)))
UNIT_TEST_BIN = $(BUILD_DIR)/unit_test_ring_buffer

############## BUILD RULES ###############
all: test eeg

tests: $(UNIT_TEST_BIN)

$(UNIT_TEST_BIN): $(UNIT_TEST_OBJS)
	$(CC) $(CFLAGS) $(UNIT_TEST_OBJS) -o $@

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



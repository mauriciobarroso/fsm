UNITY_DIR = vendor/unity/src
UNITY_SRC = $(UNITY_DIR)/unity.c
FSM_SRC = fsm.c
CFLAGS += -I$(UNITY_DIR)
CFLAGS += -Iinclude
TEST_SRCS = test/test_fsm.c
TEST_OBJS = $(patsubst %.c,%.o,$(TEST_SRCS),$(UNITY_SRC),$(FSM_SRC))
TEST_BIN = fsm_test

test: $(TEST_OBJS)
	$(CC) $^ -o $(TEST_BIN) 
	./$(TEST_BIN)

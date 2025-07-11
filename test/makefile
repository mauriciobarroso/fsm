UNITY_DIR = vendor/unity/src
CFLAGS += -I$(UNITY_DIR)
TEST_SRCS = test/test_fsm.c
TEST_OBJS = $(patsubst %.c,%.o,$(TEST_SRCS))

test: $(TEST_OBJS)
	$(CC) $^ -o $@ 
	./test

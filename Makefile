BIN = hashids test
HAHSIDS_OBJS = main.o hashids.o
TEST_OBJS = test.o hashids.o

CFLAGS ?= -O3 -Wall
LIBS = -lm

all: $(BIN)

hashids: $(HAHSIDS_OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

test: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) -f $(BIN) $(HAHSIDS_OBJS) $(TEST_OBJS)

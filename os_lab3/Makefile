CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lrt
TARGET_PARENT = parent
TARGET_CHILD = child

all: $(TARGET_PARENT) $(TARGET_CHILD)

$(TARGET_PARENT): parent.c common.h
	$(CC) $(CFLAGS) -o $(TARGET_PARENT) parent.c $(LDFLAGS)

$(TARGET_CHILD): child.c common.h
	$(CC) $(CFLAGS) -o $(TARGET_CHILD) child.c $(LDFLAGS)

clean:
	rm -f $(TARGET_PARENT) $(TARGET_CHILD)

.PHONY: all clean




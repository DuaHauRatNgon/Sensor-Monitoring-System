CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -Iinc
LDFLAGS = -pthread -lsqlite3

SRCS = src/main.c src/sbuffer.c src/connmgr.c src/datamgr.c src/sensor_db.c src/logger.c src/log_process.c
OBJS = $(SRCS:.c=.o)
TARGET = sensor_gateway

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run server trên port 5678
run: $(TARGET)
	./$(TARGET) 5678

# valgrind: $(TARGET)
# 	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) 5678

# tsan: CFLAGS += -fsanitize=thread
# tsan: LDFLAGS += -fsanitize=thread
# tsan: clean $(TARGET)
# 	./$(TARGET) 5678

clean:
	rm -f $(OBJS) $(TARGET) logFifo gateway.log Sensor.db

.PHONY: all run valgrind tsan clean
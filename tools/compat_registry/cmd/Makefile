CC = gcc
CFLAGS = -Wall -O2 -I../lib
LDFLAGS = -L../lib -lcompatreg

# 假设源码已经复制到本目录的 src/ 下（后续手动复制）
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = cmd

all: $(TARGET)

$(TARGET): $(OBJ) ../lib/libcompatreg.a
	$(CC) -o $@ $^ $(LDFLAGS)

../lib/libcompatreg.a:
	$(MAKE) -C ../lib

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
CC = gcc
CFLAGS = -Wall -O2 -I../lib
LDFLAGS = -L../lib -lcompatreg

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = uac

all: $(TARGET)

$(TARGET): $(OBJ) ../lib/libcompatreg.a
	$(CC) -o $@ $^ $(LDFLAGS)

../lib/libcompatreg.a:
	$(MAKE) -C ../lib

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
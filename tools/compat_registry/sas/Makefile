CXX = g++
CXXFLAGS = -Wall -O2 -I../lib `pkg-config --cflags Qt5Core Qt5Widgets`
LDFLAGS = -L../lib -lcompatreg `pkg-config --libs Qt5Core Qt5Widgets`

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = sas

all: $(TARGET)

$(TARGET): $(OBJ) ../lib/libcompatreg.a
	$(CXX) -o $@ $^ $(LDFLAGS)

../lib/libcompatreg.a:
	$(MAKE) -C ../lib

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
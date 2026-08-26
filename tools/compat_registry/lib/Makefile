# tools/compat_registry/lib/Makefile

LIBNAME = libcompatreg.a
OBJS = compat_registry.o

CFLAGS += -Wall -Wextra -O2 -fPIC

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJS) $(LIBNAME)

.PHONY: all clean
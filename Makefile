##########################################################
##########################################################

TARGET := bin/rfid-executor

SRCS := src/exec.c src/main.c

OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.dep)
XDEPS := $(wildcard $(DEPS))

CC = gcc

CCFLAGS = -std=gnu99

CCFLAGS += -Wall -Wextra \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wnested-externs -Wmissing-include-dirs

LDFLAGS =

INCLUDES = -Iinclude

LIBS = -lphidget22

all: dirs $(TARGET)

ifneq ($(XDEPS),)
include $(XDEPS)
endif

dirs::
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): %.o: %.c %.dep
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ -c $<

$(DEPS): %.dep: %.c Makefile
	$(CC) $(CCFLAGS) $(INCLUDES) -MM $< > $@

install:
	cp $(TARGET) /usr/local/bin/

clean:
	-rm -f src/*.o
	-rm -f src/*.dep
	-rm -f $(TARGET)

TOPDIR = .
PAGERDIR = $(TOPDIR)/pagers
LSHDIR = $(TOPDIR)/loader_shared
INDIR = $(TOPDIR)/input
#TARGET = apager
#INPUTS = rmap
TARGET = readmap hello
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall --std=c99
SCFLAGS = -static -static-libgcc -Bstatic -fPIE
PCFLAGS = -Wl,-Ttext-segment=0x400000
ICFLAGS = -Wl,-Ttext-segment=0x1000000 

.PHONY: default all clean #inputs pagers

default: $(TARGET)
#default: $(INPUTS)
all: default

SOURCES = $(wildcard $(PAGERDIR)/*.c) $(wildcard $(LSHDIR)/*.c) $(wildcard $(INDIR)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
SHARED_SRC = $(wildcard $(LSHDIR)/*.c)
SHARED_OBJS = $(patsubst %.c, %.o, $(SHARED_SRC))
HEADERS = $(wildcard $(PAGERDIR)/*.h) $(wildcard $(LSHDIR)/*.h) $(wildcard $(INDIR)/*.c)
INCLUDES = -I$(LSHDIR) -I$(PAGERDIR) -I$(TOPDIR) -I$(INDIR) 

$(SHARED_OBJS): $(SHARED_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

.PRECIOUS: $(TARGET) $(OBJECTS)

#$(TARGET): $(OBJECTS)
#	$(CC) $(OBJECTS) $(PCFLAGS) -Wall $(LIBS) -o $@

$(TARGET):  $(SHARED_OBJS)
	$(CC) $(INDIR)/$@.c $(SHARED_OBJS) $(SCFLAGS) $(ICFLAGS) -Wall $(LIBS) -o $@ $(INCLUDES)

clean:
	-rm -f *.o
	-rm -f $(LSHDIR)/*.o
	-rm -f $(INDIR)/*.o
	-rm -f $(TARGET)

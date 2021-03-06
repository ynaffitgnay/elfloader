TOPDIR = .
PAGERDIR = $(TOPDIR)/pagers
LSHDIR = $(TOPDIR)/loader_shared
INDIR = $(TOPDIR)/input
TARGET = apager dpager hpager
INPROGS = readmap hello stack_examiner nullprog lessmem fastmem
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall --std=gnu99
SCFLAGS = -static -static-libgcc -Bstatic -fPIE
PCFLAGS = -Wl,-Ttext-segment=0x400000
ICFLAGS = -Wl,-Ttext-segment=0x1000000 

.PHONY: default all clean inprogs pagers

default: $(TARGET) $(INPROGS)
pagers: $(TARGET)
inprogs: $(INPROGS)
all: default

SOURCES = $(wildcard $(PAGERDIR)/*.c) $(wildcard $(LSHDIR)/*.c) $(wildcard $(INDIR)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
IN_SRC = $(wildcard $(INDIR)/*.c)
IN_OBJS = $(INDIR)/utils.o
SHARED_SRC = $(wildcard $(LSHDIR)/*.c)
SHARED_OBJS = $(patsubst %.c, %.o, $(SHARED_SRC))
HEADERS = $(wildcard $(PAGERDIR)/*.h) $(wildcard $(LSHDIR)/*.h) $(wildcard $(INDIR)/*.h)
INCLUDES = -I$(LSHDIR) -I$(PAGERDIR) -I$(TOPDIR) -I$(INDIR) 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(PAGERDIR)/$@.o $(SHARED_OBJS) $(SCFLAGS) $(PCFLAGS) -Wall $(LIBS) -o $@ $(INCLUDES)

$(INPROGS):  $(OBJECTS)
	$(CC) $(INDIR)/$@.o $(IN_OBJS) $(SCFLAGS) $(ICFLAGS) -Wall $(LIBS) -o $@ $(INCLUDES)

clean:
	-rm -f *.o
	-rm -f $(LSHDIR)/*.o
	-rm -f $(INDIR)/*.o
	-rm -f $(TARGET) $(INPROGS)

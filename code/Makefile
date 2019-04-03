TOPDIR = .
PAGERDIR = $(TOPDIR)/pagers
LSHDIR = $(TOPDIR)/loader_shared
INDIR = $(TOPDIR)/input
TARGET = apager
INPROGS = readmap hello
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall --std=c99 -static -static-libgcc -Bstatic
MFLAGS = -fPIE
LFLAGS = -fPIC
PCFLAGS = -Wl,-Ttext-segment=0x400000
ICFLAGS = -Wl,-Ttext-segment=0x1000000 

.PHONY: default all clean inprogs pagers

default: $(TARGET)
pagers: default
inprogs: $(INPROGS)
all: default inprogs

SOURCES = $(wildcard $(PAGERDIR)/*.c) $(wildcard $(LSHDIR)/*.c) $(wildcard $(INDIR)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
MAIN_SRC = $(wildcard $(INDIR)/*.c) $(wildcard $(PAGERDIR)/*.c)
MAIN_OBJS = $(patsubst %.c, %.o, $(MAIN_SRC))
LIB_SRC = $(wildcard $(LSHDIR)/*.c)
LIB_OBJS = $(patsubst %.c, %.o, $(LIB_SRC))
HEADERS = $(wildcard $(PAGERDIR)/*.h) $(wildcard $(LSHDIR)/*.h) $(wildcard $(INDIR)/*.c)
INCLUDES = -I$(LSHDIR) -I$(PAGERDIR) -I$(TOPDIR) -I$(INDIR) 

$(MAIN_OBJS): $(MAIN_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(MFLAGS) -c $< -o $@ $(INCLUDES)

$(LIB_OBJS): $(LIB_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(LFLAGS) -c $< -o $@ $(INCLUDES)

.PRECIOUS: $(TARGET) $(OBJECTS)



$(TARGET): $(OBJECTS)
	$(CC) $(PAGERDIR)/$@.o $(LIB_OBJS) $(PCFLAGS) -Wall $(LIBS) -o $@ $(INCLUDES)

$(INPROGS):  $(OBJECTSS)
	$(CC) $(INDIR)/$@.o $(LIB_OBJS) $(ICFLAGS) -Wall $(LIBS) -o $@ $(INCLUDES)

clean:
	-rm -f *.o
	-rm -f $(LSHDIR)/*.o
	-rm -f $(INDIR)/*.o
	-rm -f $(TARGET) $(INPROGS)

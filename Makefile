CC	= /opt/gbdk/bin/lcc
LCCFLAGS = -Wa-l -Wl-m -Wl-j -Wm-yoA -Wm-yt0x1A -Wf-bo1 -Wf-bo2

SRCDIR = src
BUILDDIR = build
BINS	= $(BUILDDIR)/snake.gb
CSOURCES    = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.c)))
OBJS  = $(CSOURCES:%.c=$(BUILDDIR)/%.o)

all:	$(BINS)

# Compile and link single file in one pass
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $<

$(BINS): $(OBJS)
	$(CC) $(LCCFLAGS) -o $(BUILDDIR)/snake.gb $(OBJS) 

clean:
	rm -f build/*

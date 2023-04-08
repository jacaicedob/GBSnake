CC	= /opt/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j --debug

BINS	= snake.gb

all:	$(BINS)

# Compile and link single file in one pass
%.gb:	%.c
	$(CC) -o $@ $<

clean:
	rm -f *.o *.lst *.map *.gb *~ *.rel *.cdb *.ihx *.lnk *.sym *.asm *.noi
NAME = rapid

GAME_C_FILES = rapid.c rapid1.c rapid2.c lib/blitter.c lib/blitter_tmap.c lib/blitter_sprites.c lib/chiptune.c
GAME_BINARY_FILES = platform.spr ball.spr heart.spr bg.tmap bg.tset
GAME_C_OPTS = -DVGAMODE_400 


include $(BITBOX)/kernel/bitbox.mk
rapid.c: bg.h

%.c: %.song
	python $(BITBOX)/scripts/song2C.py $^ > $@

bg.h bg.tmap: bg.tmx 
	python $(BITBOX)/scripts/tmx.py  $< > $*.h

%.spr : %.png
	python $(BITBOX)/scripts/couples_encode.py $@ $^ 

platform.spr : platform0.png platform1.png platform2.png platform3.png 
	python $(BITBOX)/scripts/couples_encode.py $@ $^ 

clean::
	rm -f *.spr.png *.spr *.tmap *.tset rapid1.c rapid2.c
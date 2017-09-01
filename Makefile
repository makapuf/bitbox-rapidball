NAME = rapid

GAME_C_FILES = rapid.c rapid1.c rapid2.c \
  $(BITBOX)/lib/blitter/blitter.c\
  $(BITBOX)/lib/blitter/blitter_tmap.c\
  $(BITBOX)/lib/blitter/blitter_sprites.c \
  $(BITBOX)/lib/chiptune/chiptune.c \
  $(BITBOX)/lib/chiptune/player.c

GAME_BINARY_FILES = platform.spr ball.spr heart.spr bg.tmap bg.tset
VGA_MODE=400


include $(BITBOX)/kernel/bitbox.mk
rapid.c: bg.h

%.c: %.song
	python $(BITBOX)/lib/chiptune/song2C.py $^ > $@

bg.h bg.tmap: bg.tmx 
	python $(BITBOX)/lib/blitter/scripts/tmx.py  $< > $*.h

%.spr : %.png
	python $(BITBOX)/lib/blitter/scripts/sprite_encode2.py $@ $^ 

platform.spr : platform0.png platform1.png platform2.png platform3.png 
	python $(BITBOX)/lib/blitter/scripts/sprite_encode2.py $@ $^ 

clean::
	rm -f *.spr.png *.spr *.tmap *.tset rapid1.c rapid2.c bg.h
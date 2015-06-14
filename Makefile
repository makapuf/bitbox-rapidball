NAME = rapid

GAME_C_FILES = rapid.c build/bg.c rapid1.c rapid2.c
GAME_BINARY_FILES = platform.spr ball.spr heart.spr
GAME_C_OPTS = -DVGAMODE_400 


USE_ENGINE=1
USE_CHIPTUNE=1

include $(BITBOX)/lib/bitbox.mk
rapid.c: build/bg.h

build/bg.c build/bg.h: bg.tmx 
	mkdir -p build
	python $(BITBOX)/scripts/tmx.py -o build -c $< > $*.h

%.spr : %.png
	python $(BITBOX)/scripts/couples_encode.py $@ $^ 

platform.spr : platform0.png platform1.png platform2.png platform3.png 
	python $(BITBOX)/scripts/couples_encode.py $@ $^ 

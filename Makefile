all: heatpid.bin

heatpid.bin:build/heatpid.ihx
	makebin -p build/heatpid.ihx heatpid.bin


build/heatpid.ihx: heatpid.c heatpid.h
	sdcc -o build/ heatpid.c

clean:
	rm *.bin
	rm build/*

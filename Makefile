all: heatpid.bin

heatpid.bin:build/heatpid.ihx
	makebin -p build/heatpid.ihx heatpid.bin


build/irtest.ihx:
	sdcc -o build/ heatpid.c

clean:
	rm *.bin
	rm build/*

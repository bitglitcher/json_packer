
CC=gcc
SRC= *.c
HEADERS= *.h
CFLAGS= -Wall -lm -g

all: json_packer

json_packer: $(SRC) $(HEADERS)
	$(CC) $(SRC) $(CFLAGS) -o json_packer

clean: json_packer
	rm json_packer
	rm output.bin
	rm input.json

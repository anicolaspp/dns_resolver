CC=cc
CFLAGS=-Wall

SRC  =  main.c 
OBJ  =  $(SRC:.c=.o)

all:	nperez

nperez:	$(OBJ)
	@echo LINK $(OBJ) into $@
	$(CC) $(CFLAGS) $(OBJ) -o $@ 



clean:
	rm -f *.o nperez


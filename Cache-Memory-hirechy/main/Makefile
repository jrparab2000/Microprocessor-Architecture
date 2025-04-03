CC = gcc
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

SIM_SRC = sim.c
SIM_OBJ = sim.o

all: sim
    @echo "my work is done here..."

sim: $(SIM_OBJ)
    $(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
    @echo "-----------DONE WITH sim-----------"

.c.o:
    $(CC) $(CFLAGS) -c $*.c

clean:
    rm -f *.o sim
CC = gcc
IDIR = ../include
CFLAGS = -I$(IDIR) -Wall -pedantic -g

ODIR = ../obj
LDIR = ../lib

LIBS = -lm

_DEPS = bin_tree.h charger_uptime.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = bin_tree.o charger_uptime.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -o $@ -c $< $(CFLAGS)

charger_uptime: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f ./charger_uptime
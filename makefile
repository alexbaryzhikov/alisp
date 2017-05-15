CC = gcc
CFLAGS = -Wall -I.
LIBS = -lm
DEPS = alisp.h
ODIR = obj
OFILES = main.o eval.o parser.o types.o globenv.o edict.o utils.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

alisp: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/%.o: %.c $(DEPS) $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR):
	mkdir $@


.PHONY: clean

clean:
	rm -r $(ODIR)

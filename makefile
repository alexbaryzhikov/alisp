CC = gcc
CFLAGS = -Wall -I.
LIBS = -lm
DEPS = alisp.h
ODIR = obj
OFILES = main.o parser.o eval.o apply.o atom.o list.o dict.o globenv.o operators.o utils.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

alisp: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: clean

clean:
	rm -r $(ODIR)

IDIR = ./include
OUTDIR = ./out
BINDIR = ./bin
OUTOBJDIR = $(OUTDIR)/obj
SRCDIR = ./src

CC = g++
DBGFLAG = -g
CFLAGS = -Wall -I$(IDIR)

PROGRAM = emulator

SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OUTOBJDIR)/%.o,$(SRC))

$(BINDIR)/$(PROGRAM): $(OBJ) | $(BINDIR)
	$(CC) -pthread -o $@ $^ $(DBGFLAG)

$(OUTOBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OUTOBJDIR)
	$(CC) -pthread -MMD -o $@ -c $< $(CFLAGS) $(DBGFLAG)

$(BINDIR):
	mkdir -p $@

$(OUTOBJDIR):
	mkdir -p $@

-include $(OUTOBJDIR)/*.d

clean:
	rm -r $(OUTDIR)
	rm -r $(BINDIR)

.PHONY: clean

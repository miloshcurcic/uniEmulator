IDIR = ./include
OUTDIR = ./out
OUTOBJDIR = $(OUTDIR)/obj
SRCDIR = ./src

CC = g++
CFLAGS = -Wall -I$(IDIR)

PROGRAM = emulator

SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OUTOBJDIR)/%.o,$(SRC))

$(OUTDIR)/$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^

$(OUTOBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -MMD -o $@ -c $< $(CFLAGS)

-include $(OUTOBJDIR)/*.d

clean:
	rm -f $(OUTOBJDIR)/*.o
	rm -f $(OUTOBJDIR)/*.d
	rm -f $(OUTDIR)/$(PROGRAM)
	rm -f $(OUTDIR)/*~

.PHONY: clean

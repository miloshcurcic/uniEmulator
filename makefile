IDIR = ./include
OUTDIR = ./out
OUTOBJDIR = $(OUTDIR)/obj
SRCDIR = ./src

CC = g++
DBGFLAG = -g
CFLAGS = -Wall -I$(IDIR)

PROGRAM = emulator

SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OUTOBJDIR)/%.o,$(SRC))

$(OUTDIR)/$(PROGRAM): $(OBJ)
	$(CC) -pthread -o $@ $^ $(DBGFLAG)

$(OUTOBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -pthread -MMD -o $@ -c $< $(CFLAGS) $(DBGFLAG)

-include $(OUTOBJDIR)/*.d

clean:
	rm -f $(OUTOBJDIR)/*.o
	rm -f $(OUTOBJDIR)/*.d
	rm -f $(OUTDIR)/$(PROGRAM)
	rm -f $(OUTDIR)/*~

.PHONY: clean

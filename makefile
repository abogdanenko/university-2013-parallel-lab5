CC=mpiCC
CFLAGS=-c -Wall -Wextra -pedantic -Wno-long-long
BUILDDIR=build
HFILES=*.h
CPPFILES=$(wildcard *.cpp)
OBASENAMES=$(CPPFILES:.cpp=.o)
OFILES=$(addprefix $(BUILDDIR)/,$(OBASENAMES))

.PHONY: all
all: $(BUILDDIR)/transform-1-qubit-mpi

.PHONY: debug
debug: $(BUILDDIR)/transform-1-qubit-mpi
debug: CFLAGS += -DDEBUG -g

$(BUILDDIR)/transform-1-qubit-mpi: $(OFILES)
	$(CC) -o $@ $(OFILES)

$(BUILDDIR)/%.o: %.cpp *.h
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/transform-1-qubit-mpi 

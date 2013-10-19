CC=mpiCC
CFLAGS=-c -Wall -Wextra -pedantic -Wno-long-long
DEBUGDIR=debug
RELEASEDIR=release
HFILES=*.h
CPPFILES=$(wildcard *.cpp)
OBASENAMES=$(CPPFILES:.cpp=.o)
DEBUGOFILES=$(addprefix $(DEBUGDIR)/,$(OBASENAMES))
RELEASEOFILES=$(addprefix $(RELEASEDIR)/,$(OBASENAMES))

.PHONY: all
all: debug release

.PHONY: debug
debug: $(DEBUGDIR)/transform-1-qubit-mpi
debug: CFLAGS += -DDEBUG -g

$(DEBUGDIR)/transform-1-qubit-mpi: $(DEBUGOFILES)
	$(CC) -o $@ $(DEBUGDIR)/*.o

$(DEBUGDIR)/%.o: %.cpp *.h
	test -d $(DEBUGDIR) || mkdir $(DEBUGDIR)
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: release
release: $(RELEASEDIR)/transform-1-qubit-mpi

$(RELEASEDIR)/transform-1-qubit-mpi: $(RELEASEOFILES)
	$(CC) -o $@ $(RELEASEDIR)/*.o

$(RELEASEDIR)/%.o: %.cpp *.h
	test -d $(RELEASEDIR) || mkdir $(RELEASEDIR)
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(DEBUGDIR)/ $(RELEASEDIR)/

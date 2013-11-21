# usage: make [release | debug [EXTRADEBUGFLAGS='-DNORANDOM -DWAITFORGDB']]

CC=mpiCC
CFLAGS=-c -std=c++0x -Wall -Wextra -pedantic -Wno-long-long -Werror
EXTRADEBUGFLAGS= # should be overriden by command line arguments to make
DEBUGDIR=debug
RELEASEDIR=release
HFILES=$(wildcard *.h)
CPPFILES=$(wildcard *.cpp)
OBASENAMES=$(CPPFILES:.cpp=.o)
DEBUGOFILES=$(addprefix $(DEBUGDIR)/,$(OBASENAMES))
RELEASEOFILES=$(addprefix $(RELEASEDIR)/,$(OBASENAMES))

.PHONY: all
all: debug release

.PHONY: debug
debug: create_dir_debug
debug: $(DEBUGDIR)/fidelity-mpi
debug: CFLAGS += -g -DDEBUG $(EXTRADEBUGFLAGS)

.PHONY: create_dir_debug
create_dir_debug:
	test -d $(DEBUGDIR) || mkdir $(DEBUGDIR)

$(DEBUGDIR)/fidelity-mpi: $(DEBUGOFILES)
	$(CC) -o $@ $(DEBUGDIR)/*.o

$(DEBUGDIR)/%.o: %.cpp $(HFILES)
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: release
release: create_dir_release
release: $(RELEASEDIR)/fidelity-mpi

.PHONY: create_dir_release
create_dir_release:
	test -d $(RELEASEDIR) || mkdir $(RELEASEDIR)

$(RELEASEDIR)/fidelity-mpi: $(RELEASEOFILES)
	$(CC) -o $@ $(RELEASEDIR)/*.o

$(RELEASEDIR)/%.o: %.cpp $(HFILES)
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(DEBUGDIR)/ $(RELEASEDIR)/

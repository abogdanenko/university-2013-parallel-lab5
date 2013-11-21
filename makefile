# usage: make [release | debug [EXTRADEBUGFLAGS='-DNORANDOM -DWAITFORGDB']]

CC=mpiCC
CXXFLAGS=-std=c++0x -Wall -Wextra -pedantic -Wno-long-long -Werror
EXTRADEBUGFLAGS= # should be overriden by command line arguments to make
DEBUGDIR=debug
RELEASEDIR=release
HFILES=$(wildcard *.h)
CPPFILES=$(wildcard *.cpp)
OBASENAMES=$(CPPFILES:.cpp=.o)
DEBUGOFILES=$(addprefix $(DEBUGDIR)/,$(OBASENAMES))
RELEASEOFILES=$(addprefix $(RELEASEDIR)/,$(OBASENAMES))
EXECUTABLE=fidelity-mpi

.PHONY: all
all: debug release

.PHONY: debug
debug: create_dir_debug
debug: $(DEBUGDIR)/$(EXECUTABLE)
debug: CXXFLAGS += -g -DDEBUG $(EXTRADEBUGFLAGS)

.PHONY: create_dir_debug
create_dir_debug:
	test -d $(DEBUGDIR) || mkdir $(DEBUGDIR)

$(DEBUGDIR)/$(EXECUTABLE): $(DEBUGOFILES)
	$(CC) -o $@ $(DEBUGDIR)/*.o

$(DEBUGDIR)/%.o: %.cpp $(HFILES)
	$(CC) -c -o $@ $(CXXFLAGS) $<

.PHONY: release
release: create_dir_release
release: $(RELEASEDIR)/$(EXECUTABLE)

.PHONY: create_dir_release
create_dir_release:
	test -d $(RELEASEDIR) || mkdir $(RELEASEDIR)

$(RELEASEDIR)/$(EXECUTABLE): $(RELEASEOFILES)
	$(CC) -o $@ $(RELEASEDIR)/*.o

$(RELEASEDIR)/%.o: %.cpp $(HFILES)
	$(CC) -c -o $@ $(CXXFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(DEBUGDIR)/ $(RELEASEDIR)/

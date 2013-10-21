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
debug: create_dir_debug
debug: $(DEBUGDIR)/transform-1-qubit-mpi
debug: CFLAGS += -DDEBUG -g

.PHONY: create_dir_debug 
create_dir_debug: 
	test -d $(DEBUGDIR) || mkdir $(DEBUGDIR)

$(DEBUGDIR)/transform-1-qubit-mpi: $(DEBUGOFILES)
	$(CC) -o $@ $(DEBUGDIR)/*.o

$(DEBUGDIR)/%.o: %.cpp *.h
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: release
release: create_dir_release
release: $(RELEASEDIR)/transform-1-qubit-mpi

.PHONY: create_dir_release
create_dir_release:
	test -d $(RELEASEDIR) || mkdir $(RELEASEDIR)

$(RELEASEDIR)/transform-1-qubit-mpi: $(RELEASEOFILES)
	$(CC) -o $@ $(RELEASEDIR)/*.o

$(RELEASEDIR)/%.o: %.cpp *.h
	$(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -rf $(DEBUGDIR)/ $(RELEASEDIR)/

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <iostream>
#endif

#ifdef DEBUG
#define INDENT_WIDTH 4
#define INDENT_CHAR ' '
#define INDENT(level) string((level) * INDENT_WIDTH, INDENT_CHAR)
#endif

#ifdef DEBUG
using std::string;
using std::cout;
using std::endl;
#endif

#ifdef WAITFORGDB
void WaitForGdb();
#endif
#endif

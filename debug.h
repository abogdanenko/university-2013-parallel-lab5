#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <iostream>

#define INDENT_WIDTH 4
#define INDENT_CHAR ' '
#define INDENT(level) string((level) * INDENT_WIDTH, INDENT_CHAR)

using std::string;
using std::cout;
using std::endl;

#ifdef WAITFORGDB
void WaitForGdb();
#endif

void PrintBarrierCount();

#endif // DEBUG
#endif // DEBUG_H

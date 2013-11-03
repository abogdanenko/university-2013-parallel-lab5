#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <iostream>
#endif

#ifdef DEBUG
#define IDENT_WIDTH 4
#define IDENT_CHAR ' '
#define IDENT(level) string((level) * IDENT_WIDTH, IDENT_CHAR)
#endif

#ifdef DEBUG
using std::string;
using std::cout;
using std::endl;
#endif

#endif

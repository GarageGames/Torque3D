#ifndef _CONSOLFUNCTIONS_H_
#define _CONSOLFUNCTIONS_H_

#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"
#endif

bool isInt(const char* str);

bool isFloat(const char* str);

bool isValidIP(const char* ip);

bool isValidPort(U16 port);

#endif
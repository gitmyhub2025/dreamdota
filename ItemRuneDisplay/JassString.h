/*
 * JassString.h - JASS string wrapper for TextTag text
 * Based on working implementation from 1.27 reference code
 */

#ifndef JASS_STRING_H
#define JASS_STRING_H

#include <cstring>

// Simple structure matching War3's expected string format
struct CJassStringData {
    char* data;
};

struct CJassString {
    CJassStringData* data;
};

// C++ wrapper for managing CJassString lifecycle
class JassString {
private:
    CJassString* str;
    size_t len;

public:
    JassString(const char* text);
    ~JassString();

    bool ChangeStr(const char* text);
    CJassString* GetJassStr();
    const char* GetCStr();
};

#endif // JASS_STRING_H

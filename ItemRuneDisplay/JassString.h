/*
 * JassString.h - JASS string wrapper for TextTag text
 * Based on working implementation from 1.27 reference code
 */

#ifndef JASS_STRING_H
#define JASS_STRING_H

#include <cstring>

// Complete structure matching War3's expected string format
// Based on working reference code - includes vtable and internal fields
struct CJassStringData {
    unsigned int vtable;
    unsigned int refCount;
    unsigned int dwUnk1;
    unsigned int pUnk2;
    unsigned int pUnk3;
    unsigned int pUnk4;
    unsigned int pUnk5;
    char* data;
};

struct CJassString {
    unsigned int vtable;
    unsigned int dw0;
    CJassStringData* data;
    unsigned int dw1;
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

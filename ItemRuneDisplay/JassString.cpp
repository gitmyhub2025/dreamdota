/*
 * JassString.cpp - JASS string wrapper implementation
 * Based on working 1.27 reference code
 */

#include "JassString.h"
#include <cstring>

JassString::JassString(const char* text) {
    len = strlen(text);
    str = new CJassString;
    str->data = new CJassStringData;
    str->data->data = new char[len + 1];
    memcpy(str->data->data, text, len);
    str->data->data[len] = 0;
}

JassString::~JassString() {
    delete[] str->data->data;
    delete str->data;
    delete str;
}

bool JassString::ChangeStr(const char* text) {
    if (!text) return false;

    bool ret = false;
    size_t newLen = strlen(text);

    if (this->len < newLen) {
        delete[] str->data->data;
        str->data->data = new char[newLen + 1];
        this->len = newLen;
        ret = true;
    }

    memcpy(str->data->data, text, newLen);
    str->data->data[newLen] = 0;
    return ret;
}

CJassString* JassString::GetJassStr() {
    return this->str;
}

const char* JassString::GetCStr() {
    return this->str->data->data;
}

#include "ParamRegistry.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ParamEntry ParamRegistry::entries_[ParamRegistry::MAX_PARAMS];
int ParamRegistry::count_ = 0;

const char* param_short_name(const char* full_expr) {
    const char* last_dot = strrchr(full_expr, '.');
    return last_dot ? last_dot + 1 : full_expr;
}

void ParamRegistry::add_(const char* name, ParamType type, void* ptr) {
    if (count_ >= MAX_PARAMS) {
        // Registry full. Raise MAX_PARAMS in ParamRegistry.h if you hit this.
        return;
    }
    entries_[count_].name = name;
    entries_[count_].type = type;
    entries_[count_].ptr = ptr;
    count_++;
}

void ParamRegistry::reg(const char* name, float& v)    { add_(name, ParamType::FLOAT, &v); }
void ParamRegistry::reg(const char* name, uint8_t& v)  { add_(name, ParamType::U8,    &v); }
void ParamRegistry::reg(const char* name, uint16_t& v) { add_(name, ParamType::U16,   &v); }
void ParamRegistry::reg(const char* name, int16_t& v)  { add_(name, ParamType::I16,   &v); }
void ParamRegistry::reg(const char* name, uint32_t& v) { add_(name, ParamType::U32,   &v); }
void ParamRegistry::reg(const char* name, bool& v)     { add_(name, ParamType::BOOL,  &v); }

int ParamRegistry::count() { return count_; }

const ParamEntry* ParamRegistry::at(int index) {
    if (index < 0 || index >= MAX_PARAMS) return nullptr;
    return &entries_[index];
}

const ParamEntry* ParamRegistry::find(const char* name) {
    for (int i = 0; i < count_; i++) {
        if (strcmp(entries_[i].name, name) == 0) return &entries_[i];
    }
    return nullptr;
}

bool ParamRegistry::get_as_string(const char* name, char* out, size_t out_len) {
    const ParamEntry* e = find(name);
    if (!e) return false;

    switch (e->type) {
        case ParamType::FLOAT: snprintf(out, out_len, "%.4f", *static_cast<float*>(e->ptr)); break;
        case ParamType::U8:    snprintf(out, out_len, "%u",   *static_cast<uint8_t*>(e->ptr)); break;
        case ParamType::U16:   snprintf(out, out_len, "%u",   *static_cast<uint16_t*>(e->ptr)); break;
        case ParamType::I16:   snprintf(out, out_len, "%d",   *static_cast<int16_t*>(e->ptr)); break;
        case ParamType::U32:   snprintf(out, out_len, "%lu",  static_cast<unsigned long>(*static_cast<uint32_t*>(e->ptr))); break;
        case ParamType::BOOL:  snprintf(out, out_len, "%d",   *static_cast<bool*>(e->ptr) ? 1 : 0); break;
    }
    return true;
}

bool ParamRegistry::set_from_string(const char* name, const char* value_str) {
    const ParamEntry* e = find(name);
    if (!e || value_str == nullptr) return false;

    switch (e->type) {
        case ParamType::FLOAT: *static_cast<float*>(e->ptr)    = atof(value_str); break;
        case ParamType::U8:    *static_cast<uint8_t*>(e->ptr)  = static_cast<uint8_t>(atol(value_str)); break;
        case ParamType::U16:   *static_cast<uint16_t*>(e->ptr) = static_cast<uint16_t>(atol(value_str)); break;
        case ParamType::I16:   *static_cast<int16_t*>(e->ptr)  = static_cast<int16_t>(atol(value_str)); break;
        case ParamType::U32:   *static_cast<uint32_t*>(e->ptr) = static_cast<uint32_t>(atol(value_str)); break;
        case ParamType::BOOL:  *static_cast<bool*>(e->ptr)     = (atol(value_str) != 0); break;
    }
    return true;
}

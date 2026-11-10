/*
 Helpers shared by the command code: UTF-16 <-> UTF-8 conversion that does not depend on the
 platform's wchar_t, and small wrappers around the 4D object and collection API.

 Values read with PA_GetObjectProperty / PA_GetCollectionElement are borrowed. Values stored
 with PA_SetObjectProperty are copied by the runtime, so the plugin clears its own variable
 afterwards.
*/

#ifndef PLUGIN_4D_HELPERS_H
#define PLUGIN_4D_HELPERS_H

#include "4DPluginAPI.h"

#include <cstdint>
#include <cstring>
#include <string>

typedef std::u16string ustring;

static std::string to_utf8(const PA_Unichar *s, size_t len) {
    std::string out;
    out.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (s[i + 1] - 0xDC00);
            ++i;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
            c = 0xFFFD;
        }
        if (c < 0x80) {
            out += char(c);
        } else if (c < 0x800) {
            out += char(0xC0 | (c >> 6));
            out += char(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            out += char(0xE0 | (c >> 12));
            out += char(0x80 | ((c >> 6) & 0x3F));
            out += char(0x80 | (c & 0x3F));
        } else {
            out += char(0xF0 | (c >> 18));
            out += char(0x80 | ((c >> 12) & 0x3F));
            out += char(0x80 | ((c >> 6) & 0x3F));
            out += char(0x80 | (c & 0x3F));
        }
    }
    return out;
}

// Invalid sequences become U+FFFD; the renderers only ever produce valid UTF-8, but input to
// this function is not trusted.
static ustring to_utf16(const char *p_, size_t n) {
    const unsigned char *p = (const unsigned char *)p_;
    ustring out;
    out.reserve(n);
    size_t i = 0;
    while (i < n) {
        uint32_t c = p[i];
        size_t extra;
        uint32_t min;
        if (c < 0x80) { extra = 0; min = 0; }
        else if ((c & 0xE0) == 0xC0) { c &= 0x1F; extra = 1; min = 0x80; }
        else if ((c & 0xF0) == 0xE0) { c &= 0x0F; extra = 2; min = 0x800; }
        else if ((c & 0xF8) == 0xF0) { c &= 0x07; extra = 3; min = 0x10000; }
        else { out += char16_t(0xFFFD); ++i; continue; }
        bool ok = i + extra < n || extra == 0;
        for (size_t k = 1; ok && k <= extra; ++k) {
            if (i + k >= n || (p[i + k] & 0xC0) != 0x80) ok = false;
            else c = (c << 6) | (p[i + k] & 0x3F);
        }
        if (!ok || c < min || c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF)) {
            out += char16_t(0xFFFD);
            ++i;
            continue;
        }
        i += extra + 1;
        if (c >= 0x10000) {
            c -= 0x10000;
            out += char16_t(0xD800 + (c >> 10));
            out += char16_t(0xDC00 + (c & 0x3FF));
        } else {
            out += char16_t(c);
        }
    }
    return out;
}

static ustring to_utf16(const std::string &s) { return to_utf16(s.data(), s.size()); }

static std::string unistring_utf8(const PA_Unistring *u) {
    if (!u || !u->fString || u->fLength <= 0) return std::string();
    return to_utf8(u->fString, size_t(u->fLength));
}

// --- objects ------------------------------------------------------------------------------

static PA_Unistring make_key(const char *key) {
    ustring k = to_utf16(key, std::strlen(key));
    return PA_CreateUnistring((PA_Unichar *)k.c_str());
}

static void ob_set_var(PA_ObjectRef o, const char *key, PA_Variable *v) {
    PA_Unistring k = make_key(key);
    PA_SetObjectProperty(o, &k, *v);
    PA_DisposeUnistring(&k);
    PA_ClearVariable(v);
}

static void ob_set_s(PA_ObjectRef o, const char *key, const char *value, size_t len) {
    ustring u = to_utf16(value, len);
    PA_Variable v = PA_CreateVariable(eVK_Unistring);
    PA_Unistring us = PA_CreateUnistring((PA_Unichar *)u.c_str());
    PA_SetStringVariable(&v, &us);
    ob_set_var(o, key, &v);
}

static void ob_set_s(PA_ObjectRef o, const char *key, const std::string &value) {
    ob_set_s(o, key, value.data(), value.size());
}

static void ob_set_n(PA_ObjectRef o, const char *key, PA_long32 value) {
    PA_Variable v = PA_CreateVariable(eVK_Longint);
    PA_SetLongintVariable(&v, value);
    ob_set_var(o, key, &v);
}

static void ob_set_b(PA_ObjectRef o, const char *key, bool value) {
    PA_Variable v = PA_CreateVariable(eVK_Boolean);
    PA_SetBooleanVariable(&v, value ? 1 : 0);
    ob_set_var(o, key, &v);
}

static bool ob_get_var(PA_ObjectRef o, const char *key, PA_Variable &out) {
    if (!o) return false;
    PA_Unistring k = make_key(key);
    bool found = PA_HasObjectProperty(o, &k) != 0;
    if (found) out = PA_GetObjectProperty(o, &k);
    PA_DisposeUnistring(&k);
    return found;
}

static bool ob_get_s(PA_ObjectRef o, const char *key, std::string &out) {
    PA_Variable v;
    if (!ob_get_var(o, key, v) || PA_GetVariableKind(v) != eVK_Unistring) return false;
    PA_Unistring u = PA_GetStringVariable(v);
    out = unistring_utf8(&u);
    return true;
}

static bool ob_get_n(PA_ObjectRef o, const char *key, double &out) {
    PA_Variable v;
    if (!ob_get_var(o, key, v)) return false;
    switch (PA_GetVariableKind(v)) {
        case eVK_Real: out = PA_GetRealVariable(v); return true;
        case eVK_Longint: out = PA_GetLongintVariable(v); return true;
        default: return false;
    }
}

static bool ob_get_b(PA_ObjectRef o, const char *key, bool &out) {
    PA_Variable v;
    if (!ob_get_var(o, key, v) || PA_GetVariableKind(v) != eVK_Boolean) return false;
    out = PA_GetBooleanVariable(v) != 0;
    return true;
}

// Sets or clears `flag` in `flags` when the option is a Boolean; other types are ignored.
static void ob_get_flag(PA_ObjectRef o, const char *key, unsigned flag, unsigned &flags) {
    bool b;
    if (ob_get_b(o, key, b)) flags = b ? (flags | flag) : (flags & ~flag);
}

// --- the command's result object ----------------------------------------------------------

static void return_success(PA_PluginParameters params, const char *result, size_t len) {
    PA_ObjectRef o = PA_CreateObject();
    ob_set_b(o, "success", true);
    ob_set_n(o, "status", 0);
    ob_set_s(o, "result", result, len);
    PA_ReturnObject(params, o);
}

static void return_failure(PA_PluginParameters params, PA_long32 status, const std::string &error) {
    PA_ObjectRef o = PA_CreateObject();
    ob_set_b(o, "success", false);
    ob_set_n(o, "status", status);
    ob_set_s(o, "result", "", 0);
    ob_set_s(o, "error", error);
    PA_ReturnObject(params, o);
}

#endif

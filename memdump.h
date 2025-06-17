// memdump.h --- C++ memory dumper
// License: MIT

#ifndef MEMDUMP_H_INCLUDED
#define MEMDUMP_H_INCLUDED 100

#pragma once

#ifndef __cplusplus
    #error This file requires C++ compiler.
#endif

#include <ctype.h>
#include <assert.h>

namespace md {

template <typename T_STRING>
inline void add_char(T_STRING& str, char ch)
{
    str += ch;
}

template <typename T_STRING>
inline void add_string(T_STRING& str, const char *ptr)
{
    while (*ptr)
        add_char(str, *ptr++);
}

template <typename T_STRING>
inline void add_byte(T_STRING& str, unsigned char byte, const char *hex)
{
    add_char(str, hex[(byte) >> 4]);
    add_char(str, hex[(byte) & 0xF]);
}

template <typename T_STRING>
inline void add_addr(T_STRING& str, size_t value, const char *hex, bool enable_64bit = true)
{
#if defined(_WIN64) || defined(_M_X64) || defined(M_AMD64) || defined(_M_ARM64) // 64-bit?
    if (enable_64bit)
    {
        add_char(str, hex[(value >> 60) & 0xF]);
        add_char(str, hex[(value >> 56) & 0xF]);
        add_char(str, hex[(value >> 52) & 0xF]);
        add_char(str, hex[(value >> 48) & 0xF]);
        add_char(str, hex[(value >> 44) & 0xF]);
        add_char(str, hex[(value >> 40) & 0xF]);
        add_char(str, hex[(value >> 36) & 0xF]);
        add_char(str, hex[(value >> 32) & 0xF]);
    }
#endif
    add_char(str, hex[(value >> 28) & 0xF]);
    add_char(str, hex[(value >> 24) & 0xF]);
    add_char(str, hex[(value >> 20) & 0xF]);
    add_char(str, hex[(value >> 16) & 0xF]);
    add_char(str, hex[(value >> 12) & 0xF]);
    add_char(str, hex[(value >> 8) & 0xF]);
    add_char(str, hex[(value >> 4) & 0xF]);
    add_char(str, hex[(value >> 0) & 0xF]);
}

struct settings_t
{
    size_t addr_base;
    size_t column;
    bool has_addr;
    bool has_header;
    bool enable_64bit;
    const char *hex;

    settings_t()
    {
        addr_base = 0;
        column = 16;
        has_addr = true;
        has_header = true;
        enable_64bit = true;
        hex = "0123456789ABCDEF";
    }
};

template <typename T_STRING>
void memdump(T_STRING& str, const void *ptr, size_t size, const settings_t& settings)
{
    size_t addr_base = settings.addr_base, column = settings.column;
    bool has_addr = settings.has_addr, has_header = settings.has_header;
    bool enable_64bit = settings.enable_64bit;
    const char *hex = settings.hex;
    const unsigned char *pb = reinterpret_cast<const unsigned char *>(ptr);

    assert(1 < column);
    assert(column <= 0x100);

    size_t i, j, count;

    // header
    if (has_header)
    {
        // addr
        if (has_addr)
        {
            add_string(str, "+ADDRESS  ");
#if defined(_WIN64) || defined(_M_X64) || defined(M_AMD64) || defined(_M_ARM64) // 64-bit?
            if (enable_64bit)
                add_string(str, "******  ");
#endif
        }

        // column offsets
        for (i = 0; i < column; ++i)
        {
            if (i < 16)
            {
                add_char(str, '+');
                add_char(str, hex[i & 0xF]);
            }
            else
            {
                add_char(str, hex[i >> 4]);
                add_char(str, hex[i & 0xF]);
            }
            add_char(str, ' ');
        }
        add_char(str, ' ');

        // text offsets
        for (i = 0; i < column; ++i)
            add_char(str, hex[i & 0xF]);

        // new line
        add_char(str, '\n');
    }

    // body
    for (i = 0; i < size; i += column)
    {
        // addr
        if (has_addr)
        {
            add_addr(str, addr_base + i, hex, enable_64bit);
            add_string(str, "  ");
        }

        count = (i + column < size) ? column : (size - i);

        // hex
        for (j = 0; j < count; j++)
        {
            add_byte(str, pb[i + j], hex);
            add_char(str, ' ');
        }

        for (j = 0; j < 3 * (column - count) + 1; ++j)
            add_char(str, ' ');

        // text
        for (j = 0; j < count; j++)
            add_char(str, isprint(pb[i + j]) ? pb[i + j] : '.');

        // new line
        add_char(str, '\n');
    }
}

template <typename T_STRING>
inline void memdump(T_STRING& str, const void *ptr, size_t size)
{
    settings_t settings;
    memdump(str, ptr, size, settings);
}

} // namespace md

using md::memdump;

#endif /* ndef MEMDUMP_H_INCLUDED */

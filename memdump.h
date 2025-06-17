// memdump.h --- C++11 memory dumper

#pragma once

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cassert>

struct memdump_settings
{
    int column = 16;
    bool has_addr = true;
    bool has_header = true;
};

template <typename T_STR>
void memdump(T_STR& str, const void *ptr, size_t size, const memdump_settings& settings = {})
{
    // settings
    int column = settings.column;
    bool has_addr = settings.has_addr, has_header = settings.has_header;

    size_t i, j, next_count;
    const uint8_t *pb = reinterpret_cast<const uint8_t *>(ptr);

    assert(1 < column);
    assert(column <= 0x100);

    // helper function
    static const char *HEX = "0123456789ABCDEF";
    auto byte_to_hex = [&](char buf[3], uint8_t byte)
    {
        buf[0] = HEX[byte >> 4];
        buf[1] = HEX[byte & 0xF];
        buf[2] = 0;
    };

    // header
    if (has_header)
    {
        // addr
        if (has_addr)
            str += "+ADDRESS  ";

        // column offset
        for (i = 0; i < column; ++i)
        {
            if (i < 16)
            {
                str += '+';
                str += HEX[i & 0xF];
            }
            else
            {
                str += HEX[i >> 4];
                str += HEX[i & 0xF];
            }
            str += ' ';
        }
        str += ' ';

        // text offset
        for (i = 0; i < column; ++i)
        {
            str += HEX[i & 0xF];
        }

        // new line
        str += '\n';
    }

    // body
    for (i = 0; i < size; i += column)
    {
        // addr
        if (has_addr)
        {
            char buf1[16];
            std::sprintf(buf1, "%08lX  ", i);
            str += buf1;
        }

        next_count = (i + column < size) ? column : (size - i);

        // hex
        for (j = 0; j < next_count; j++)
        {
            char buf2[8];
            byte_to_hex(buf2, pb[i + j]);
            buf2[2] = ' ';
            buf2[3] = 0;
            str += buf2;
        }
        str += T_STR(3 * (column - next_count) + 1, ' ');

        // text
        for (j = 0; j < next_count; j++)
        {
            if (isprint(pb[i + j]))
                str += pb[i + j];
            else
                str += '.';
        }

        // new line
        str += "\n";
    }
}

template <typename T_STR>
void filedump(T_STR& str, FILE *fp, const memdump_settings& settings = {})
{
    T_STR image;

    char buf[1024];
    for (;;)
    {
        size_t count = fread(buf, 1, sizeof(buf), fp);
        if (!count)
            break;

        image.append(buf, count);
    }

    memdump(str, image.data(), image.size(), settings);
}

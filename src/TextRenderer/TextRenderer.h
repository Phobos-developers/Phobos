#pragma once
#include <Windows.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <vector>

class BitFont;
class Surface;

namespace TextRenderer
{
    struct Glyph
    {
        unsigned int id;
        int x_advance;
        int x_offset;
        int y_offset;
        bool isSpace;
        bool isDigit;
    };

    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int alignment);
    bool GetTextDimension(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth);
}

#pragma once
#include <Windows.h>
#include <wingdi.h>
#include <string>

class BitFont;
class Surface;

namespace TextRenderer
{
    bool IsInitialized();
    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int alignment);
    bool GetTextDimension(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth);
}

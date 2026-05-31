#include <Helpers/Macro.h>
#include "TextRenderer.h"
#include <BitFont.h>
#include <BitText.h>
#include <Surface.h>

DEFINE_HOOK(0x433CF0, BitFont_GetTextDimension, 8)
{
    GET(BitFont*, pFont, ECX);
    GET_STACK(const wchar_t*, pText, 0x4);
    GET_STACK(int*, pWidth, 0x8);
    GET_STACK(int*, pHeight, 0xC);
    GET_STACK(int, nMaxWidth, 0x10);

    if (TextRenderer::GetTextDimension(pFont, pText, pWidth, pHeight, nMaxWidth))
    { R->EAX(1); return 0x433EA2; }
    return 0;
}

DEFINE_HOOK(0x434CD0, BitText_DrawText, 10)
{
    GET_STACK(BitFont*, pFont, 0x4);
    GET_STACK(Surface*, pSurface, 0x8);
    GET_STACK(const wchar_t*, pWideString, 0xC);
    GET_STACK(int, X, 0x10); GET_STACK(int, Y, 0x14);
    GET_STACK(int, W, 0x18); GET_STACK(int, H, 0x1C);
    GET_STACK(int, alignment, 0x20); // Extracted as a8 in original code

    if (TextRenderer::DrawText(pFont, pSurface, pWideString, X, Y, W, H, alignment))
        return 0x435310;
    return 0;
}

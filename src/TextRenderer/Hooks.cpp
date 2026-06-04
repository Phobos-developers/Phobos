#include <Helpers/Macro.h>
#include "TextRenderer.h"
#include <BitFont.h>
#include <BitText.h>
#include <Surface.h>

// BitText::Print is designed for single-line text but the game passes a narrow
// width that causes GDI to word-wrap. We override with a large fixed width to
// force single-line rendering for CSF messages and other Print callers.
static constexpr int PRINT_SINGLELINE_WIDTH = 2000;

DEFINE_HOOK(0x433CF0, BitFont_GetTextDimension, 8)
{
    GET(BitFont*, pFont, ECX);
    GET_STACK(const wchar_t*, pText, 0x4);
    GET_STACK(int*, pWidth, 0x8);
    GET_STACK(int*, pHeight, 0xC);
    GET_STACK(int, nMaxWidth, 0x10);

    if (TextRenderer::GetTextDimension(pFont, pText, pWidth, pHeight, nMaxWidth))
    {
        R->EAX(1);
        return 0x433EA2;
    }
    return 0;
}

DEFINE_HOOK(0x434CD0, BitText_DrawText, 10)
{
    GET_STACK(BitFont*, pFont, 0x4);
    GET_STACK(Surface*, pSurface, 0x8);
    GET_STACK(const wchar_t*, pWideString, 0xC);
    GET_STACK(int, X, 0x10);
    GET_STACK(int, Y, 0x14);
    GET_STACK(int, W, 0x18);
    GET_STACK(int, H, 0x1C);
    GET_STACK(int, alignment, 0x20);

    if (TextRenderer::DrawText(pFont, pSurface, pWideString, X, Y, W, H, alignment))
        return 0x435310;
    return 0;
}

DEFINE_HOOK(0x434B90, BitText_Print, 6)
{
    GET_STACK(BitFont*, pFont, 0x4);
    GET_STACK(Surface*, pSurface, 0x8);
    GET_STACK(const wchar_t*, pWideString, 0xC);
    GET_STACK(int, X, 0x10);
    GET_STACK(int, Y, 0x14);
    GET_STACK(int, W, 0x18);
    GET_STACK(int, H, 0x1C);

    if (TextRenderer::DrawText(pFont, pSurface, pWideString, X, Y, PRINT_SINGLELINE_WIDTH, H, 0))
        return 0x434BDE;
    return 0;
}

DEFINE_HOOK(0x434120, BitFont_Blit, 6)
{
    GET(BitFont*, pFont, ECX);
    GET_STACK(wchar_t, wch, 0x4);
    GET_STACK(int, X, 0x8);
    GET_STACK(int, Y, 0xC);
    GET_STACK(int, nColor, 0x10);

    wchar_t pText[2] = { wch, L'\0' };
    const auto originalColor = pFont->Color;
    if (nColor != -1)
    {
        pFont->Color = static_cast<WORD>(nColor);
    }

    if (TextRenderer::DrawText(pFont, DSurface::Composite, pText, X, Y, 0, 0, 0))
    {
        int charWidth = 0;
        TextRenderer::GetTextDimension(pFont, pText, &charWidth, nullptr, 0);
        pFont->Color = originalColor;
        R->EAX(X + charWidth + 1);
        return 0x434155;
    }
    pFont->Color = originalColor;
    return 0;
}

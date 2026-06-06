// Hooks.cpp
// Engine hooks that intercept the game's text rendering functions and route
// them through the GDI-based TTF renderer. Four hooks cover all text paths:
// measurement, main drawing, single-line printing, and character blitting.
#include <Helpers/Macro.h>
#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>

// ============================================================================
// HOOK: BitFont::GetTextDimension (0x433CF0)
// Intercepts text measurement calls. The game uses this to determine how large
// a text box needs to be before allocating UI space.
// Returns TTF-measured dimensions instead of .FNT bitmap font dimensions.
// ============================================================================
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

// ============================================================================
// HOOK: BitText::DrawText (0x434CD0)
// Main text rendering hook. Catches all UI text: menu buttons, tooltips,
// sidebar text, loading screen, power bar, and general game messages.
// Routes directly to the TTF renderer with the game's original parameters.
// ============================================================================
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

// ============================================================================
// HOOK: BitText::Print (0x434B90)
// Handles single-line text: CSF messages ("Unit Lost", "Tech Building Captured"),
// player names on the loading screen, and other game notifications.
// The game passes a narrow width that would cause GDI to word-wrap mid-sentence.
// We override with MAX_TEXT_WIDTH (800px) to force single-line output.
// ============================================================================
DEFINE_HOOK(0x434B90, BitText_Print, 6)
{
    GET_STACK(BitFont*, pFont, 0x4);
    GET_STACK(Surface*, pSurface, 0x8);
    GET_STACK(const wchar_t*, pWideString, 0xC);
    GET_STACK(int, X, 0x10);
    GET_STACK(int, Y, 0x14);
    GET_STACK(int, H, 0x1C);

    if (TextRenderer::DrawText(pFont, pSurface, pWideString, X, Y, TextRenderer::MAX_TEXT_WIDTH, H, 0))
        return 0x434BDE;
    return 0;
}

// ============================================================================
// HOOK: BitFont::Blit (0x434120)
// Handles per-character rendering for chat input and typewriter text effects.
// Uses measure-only mode: returns the character width for cursor positioning
// without performing an expensive DIB draw operation.
// The full string is rendered at once by BitText::Print on the next frame,
// avoiding per-keystroke surface lock/unlock cycles.
// ============================================================================
DEFINE_HOOK(0x434120, BitFont_Blit, 6)
{
    GET(BitFont*, pFont, ECX);
    GET_STACK(wchar_t, wch, 0x4);
    GET_STACK(int, X, 0x8);

    // Measure character width for cursor advancement only - do not draw
    wchar_t pText[2] = { wch, L'\0' };
    int charWidth = 0;
    TextRenderer::GetTextDimension(pFont, pText, &charWidth, nullptr, 0);
    R->EAX(X + charWidth + 1);
    return 0x434155;
}

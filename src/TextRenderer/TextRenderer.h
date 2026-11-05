// TextRenderer.h
#pragma once
#include <Windows.h>

class BitFont;
class Surface;

namespace TextRenderer
{
    // Maximum pixel width for text drawing and single-line rendering.
    // Caps DIB buffer allocation to prevent memory exhaustion on large surfaces
    // and forces single-line rendering for CSF messages and
    // Print callers by overriding their narrow width parameter.
    static constexpr int MAX_TEXT_WIDTH = 800;

    // Renders text onto a game surface using GDI. Handles surface locking,
    // background preservation, color conversion, alignment, and word wrapping.
    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int alignment);

    // Measures text dimensions without drawing. Used by the game to allocate
    // appropriately sized text boxes for UI elements.
    bool GetTextDimension(BitFont* pFont, const wchar_t* pText,
        int* pWidth, int* pHeight, int nMaxWidth);
}

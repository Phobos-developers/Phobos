// TextRenderer.cpp
// GDI-based TrueType font rendering engine for Command & Conquer: Yuri's Revenge.
// Replaces the game's legacy .FNT bitmap fonts with modern .TTF/.OTF support
// using Windows DrawTextW. All text rendering is routed through a temporary
// 16-bit RGB565 DIB section that matches the game's surface format.
#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>

namespace TextRenderer
{
    // GDI objects created once and reused across all text rendering calls
    static HFONT g_FontHandle = nullptr;      // Handle to the selected TrueType font
    static HDC g_DeviceContext = nullptr;      // Memory device context for off-screen drawing
    static bool g_FontLoaded = false;          // Guards one-time font initialization

    // Cached DIB (Device Independent Bitmap) for performance.
    // Recreated only when draw dimensions change between frames.
    static void* g_BitmapData = nullptr;       // Raw pixel buffer of the DIB section
    static HBITMAP g_BitmapHandle = nullptr;   // GDI handle to the DIB section
    static int g_BitmapWidth = 0;              // Current cached bitmap width
    static int g_BitmapHeight = 0;             // Current cached bitmap height

    // ========================================================================
    // LoadFontOnce - One-time initialization of the GDI font and device context.
    // Reads [EnableTTF] section from UIMD.INI. Called automatically on the
    // first DrawText or GetTextDimension call. Fails silently if TTF is
    // disabled or the font file cannot be loaded.
    // ========================================================================
    static void LoadFontOnce()
    {
        if (g_FontLoaded) return;
        g_FontLoaded = true;

        CCINIClass config;
        config.LoadFromFile(GameStrings::UIMD_INI);

        // TTF rendering is opt-in; defaults to disabled
        if (!config.ReadBool("EnableTTF", "Enabled", false)) return;

        // Read font configuration from INI
        char fileName[MAX_PATH];
        config.ReadString("EnableTTF", "FontName", "arial.ttf", fileName);
        int fontSize = config.ReadInteger("EnableTTF", "FontSize", 14);

        // AntiAlias: true = ClearType (smoother and faster), false = no smoothing
        bool antiAlias = config.ReadBool("EnableTTF", "AntiAlias", true);
        DWORD quality = antiAlias ? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY;

        // CreateFontA accepts ANSI strings from the INI reader
        g_FontHandle = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            quality, FF_DONTCARE, fileName);

        if (g_FontHandle)
            g_DeviceContext = CreateCompatibleDC(nullptr);
    }

    // ========================================================================
    // DrawText - Renders text onto a game DirectDraw surface using Windows GDI.
    //
    // Process:
    //   1. Lock the surface region where text will be drawn
    //   2. Create or reuse a 16-bit RGB565 DIB matching the surface format
    //   3. Copy surface background into DIB (preserves existing graphics)
    //   4. Convert game's BGR565 color to RGB for GDI
    //   5. Draw text onto DIB using DrawTextW with appropriate flags
    //   6. Copy finished DIB back to the surface
    //   7. Unlock surface, restore GDI state
    //
    // Parameters:
    //   gameFont   - Source for text color (pFont->Color in BGR565 format)
    //   gameSurface - Target DirectDraw surface
    //   text       - Wide-character string to render
    //   posX, posY - Top-left position on the surface
    //   boxWidth, boxHeight - Bounding box (0 = use surface dimensions)
    //   alignment  - Bit flags: bit0=center, bit1=right, bit2=vertical center
    // ========================================================================
    bool DrawText(BitFont* gameFont, Surface* gameSurface, const wchar_t* text,
        int posX, int posY, int boxWidth, int boxHeight, int alignment)
    {
        if (!text || !*text || !gameSurface) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        DSurface* surface = static_cast<DSurface*>(gameSurface);
        int surfaceW = surface->GetWidth(), surfaceH = surface->GetHeight();

        // Micro-adjustment for button-sized text to visually center within capsules
        if (boxHeight > 0 && boxHeight < 30)
            posY = std::max(0, posY - 2);

        // Clamp drawing origin to visible surface area.
        // offsetX/Y represents how far into the off-screen area the origin was.
        int clampedX = std::max(0, posX), clampedY = std::max(0, posY);
        int offsetX = clampedX - posX, offsetY = clampedY - posY;

        // Calculate draw area, reduced by any off-screen portion
        int drawW = boxWidth > 0 ? boxWidth : surfaceW - posX;
        int drawH = boxHeight > 0 ? boxHeight : surfaceH - posY;
        drawW -= offsetX;
        drawH -= offsetY;

        // Safety caps to prevent excessive memory allocation
        if (drawW > MAX_TEXT_WIDTH) drawW = MAX_TEXT_WIDTH;
        if (drawW > surfaceW - clampedX) drawW = surfaceW - clampedX;
        drawW = (drawW + 1) & ~1; // Align to 16-bit word boundary
        if (drawW < 16) drawW = 16;

        if (drawW <= 0 || drawH <= 0 || clampedX >= surfaceW || clampedY >= surfaceH)
            return false;

        // Lock only the region being drawn, not the entire surface
        void* surfaceBuf = surface->Lock(clampedX, clampedY);
        if (!surfaceBuf) return false;

        // Recreate cached DIB only when draw dimensions change
        if (drawW != g_BitmapWidth || drawH != g_BitmapHeight)
        {
            if (g_BitmapHandle) DeleteObject(g_BitmapHandle);

            // 16-bit RGB565 format matching the game's surface
            BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), drawW, -drawH, 1, 16, BI_BITFIELDS } };
            ((DWORD*)&bi.bmiColors)[0] = 0xF800; // Red mask (5 bits)
            ((DWORD*)&bi.bmiColors)[1] = 0x07E0; // Green mask (6 bits)
            ((DWORD*)&bi.bmiColors)[2] = 0x001F; // Blue mask (5 bits)

            g_BitmapHandle = CreateDIBSection(g_DeviceContext, &bi, DIB_RGB_COLORS, &g_BitmapData, nullptr, 0);
            g_BitmapWidth = drawW; g_BitmapHeight = drawH;
        }
        if (!g_BitmapHandle) { surface->Unlock(); return false; }

        // Select our font and DIB into the memory device context
        HBITMAP oldBmp = (HBITMAP)SelectObject(g_DeviceContext, g_BitmapHandle);
        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);
        int pitch = surface->GetPitch(), copyW = std::min(drawW, surfaceW - clampedX);

        // Preserve surface background by copying it into the DIB before drawing
        for (int y = 0; y < drawH && (clampedY + y) < surfaceH; y++)
            memcpy((uint8_t*)g_BitmapData + y * drawW * 2, (uint8_t*)surfaceBuf + y * pitch, copyW * 2);

        // Convert game's BGR565 color to RGB for GDI.
        // BGR565 format: bits 0-4=Red, 5-10=Green, 11-15=Blue
        uint16_t color = gameFont ? gameFont->Color : 0x7FFF;
        SetTextColor(g_DeviceContext, RGB(((color >> 11) & 0x1F) << 3,
                                         ((color >> 5) & 0x3F) << 2,
                                         (color & 0x1F) << 3));
        SetBkMode(g_DeviceContext, TRANSPARENT);

        // Build formatting flags from the game's alignment parameter
        UINT flags = DT_NOPREFIX;
        if (alignment & 1)       flags |= DT_CENTER;
        else if (alignment & 2)  flags |= DT_RIGHT;
        else                     flags |= DT_LEFT;

        // Define the text rectangle, offset to account for clamped origin
        RECT rect = { offsetX, offsetY, offsetX + drawW, offsetY + drawH };

        // Select rendering mode based on text type
        if (wcslen(text) == 1 && boxWidth <= 0 && boxHeight <= 0)
            flags |= DT_SINGLELINE;                          // Single character
        else if (boxHeight > 0 && boxHeight < 30)
            flags |= DT_SINGLELINE | DT_VCENTER;             // Button text
        else
            flags |= DT_WORDBREAK | DT_NOCLIP;               // Multi-line with word wrap

        // Render text using Windows GDI
        DrawTextW(g_DeviceContext, text, -1, &rect, flags);
        GdiFlush(); // Ensure all GDI commands complete before reading back

        // Copy rendered pixels back to the game surface
        for (int y = 0; y < drawH && (clampedY + y) < surfaceH; y++)
            memcpy((uint8_t*)surfaceBuf + y * pitch, (uint8_t*)g_BitmapData + y * drawW * 2, copyW * 2);

        // Restore previous GDI state and unlock surface
        SelectObject(g_DeviceContext, oldFont); SelectObject(g_DeviceContext, oldBmp);
        surface->Unlock();
        return true;
    }

    // ========================================================================
    // GetTextDimension - Measures text without drawing.
    // Uses DT_CALCRECT to calculate the pixel dimensions the text would occupy.
    // Returns width and height through output parameters.
    // ========================================================================
    bool GetTextDimension(BitFont*, const wchar_t* text, int* outW, int* outH, int maxW)
    {
        if (!text || !*text) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);

        // DT_CALCRECT tells GDI to measure only, not draw
        RECT rect = { 0, 0, maxW > 0 ? maxW : 2000, 0 };
        DrawTextW(g_DeviceContext, text, -1, &rect, DT_CALCRECT | DT_NOCLIP | DT_WORDBREAK);

        SelectObject(g_DeviceContext, oldFont);

        if (outW) *outW = rect.right - rect.left;
        if (outH) *outH = rect.bottom - rect.top;
        return true;
    }
}

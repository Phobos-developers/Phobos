#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>
#include <algorithm>

namespace TextRenderer
{
    // GDI objects created once and reused for all text rendering
    static HFONT g_FontHandle = nullptr;
    static HDC g_DeviceContext = nullptr;
    static bool g_FontLoaded = false;

    // DIB (Device Independent Bitmap) cache - recreated only when draw dimensions change
    static void* g_BitmapData = nullptr;
    static HBITMAP g_BitmapHandle = nullptr;
    static int g_BitmapWidth = 0;
    static int g_BitmapHeight = 0;

    // Maximum width for a single text draw - prevents excessive memory allocation
    static constexpr int MAX_DRAWING_WIDTH = 1200;

    // Returns true if the font handle and device context are valid and ready for drawing
    bool IsInitialized()
    {
        return g_FontHandle != nullptr && g_DeviceContext != nullptr;
    }

    // Loads font configuration from UIMD.INI and creates the GDI font handle once.
    // Called automatically on first DrawText or GetTextDimension call.
    static void LoadFontOnce()
    {
        if (g_FontLoaded) return;
        g_FontLoaded = true;

        CCINIClass config;
        config.LoadFromFile(GameStrings::UIMD_INI);

        // TTF rendering must be explicitly enabled in the INI file
        if (!config.ReadBool("EnableTTF", "Enabled", false)) return;

        // Read font name and size from the [EnableTTF] section
        char fileName[MAX_PATH];
        config.ReadString("EnableTTF", "FontName", "arial.ttf", fileName);
        int fontSize = config.ReadInteger("EnableTTF", "FontSize", 14);

        // Anti-aliasing: true = ClearType (smoother, faster), false = no smoothing
        bool antiAlias = config.ReadBool("EnableTTF", "AntiAlias", true);
        DWORD quality = antiAlias ? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY;

        // Create the logical font for GDI text rendering
        g_FontHandle = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            quality, FF_DONTCARE, fileName);

        if (g_FontHandle)
            g_DeviceContext = CreateCompatibleDC(nullptr);
    }

    // Renders text onto a game surface using Windows GDI.
    // Handles surface locking, background preservation, color conversion, alignment,
    // single-line vs multi-line formatting, and word wrapping.
    bool DrawText(BitFont* gameFont, Surface* gameSurface, const wchar_t* text,
        int posX, int posY, int boxWidth, int boxHeight, int alignment)
    {
        if (!text || !*text || !gameSurface) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        DSurface* directDrawSurface = static_cast<DSurface*>(gameSurface);
        int surfaceWidth = directDrawSurface->GetWidth();
        int surfaceHeight = directDrawSurface->GetHeight();

        // Small vertical adjustment for button-sized text to visually center within capsules
        if (boxHeight > 0 && boxHeight < 30)
            posY = std::max(0, posY - 2);

        // Clamp drawing position to visible surface area and calculate offset for GDI rect
        int clampedX = std::max(0, posX);
        int clampedY = std::max(0, posY);
        int offsetX = clampedX - posX;
        int offsetY = clampedY - posY;

        // Calculate actual draw dimensions, accounting for off-screen clamping
        int drawingWidth = boxWidth > 0 ? boxWidth : surfaceWidth - posX;
        int drawingHeight = boxHeight > 0 ? boxHeight : surfaceHeight - posY;
        drawingWidth -= offsetX;
        drawingHeight -= offsetY;

        // Apply safety caps to prevent memory exhaustion on large surfaces
        if (drawingWidth > MAX_DRAWING_WIDTH) drawingWidth = MAX_DRAWING_WIDTH;
        if (drawingWidth > surfaceWidth - clampedX) drawingWidth = surfaceWidth - clampedX;
        drawingWidth = (drawingWidth + 1) & ~1; // Align to 16-bit word boundary
        if (drawingWidth < 16) drawingWidth = 16;

        if (drawingWidth <= 0 || drawingHeight <= 0 || clampedX >= surfaceWidth || clampedY >= surfaceHeight)
            return false;

        // Lock only the region we need to draw on (not the entire surface)
        void* surfaceBuffer = directDrawSurface->Lock(clampedX, clampedY);
        if (!surfaceBuffer) return false;

        // Recreate the DIB only when draw dimensions change (cached for performance)
        if (drawingWidth != g_BitmapWidth || drawingHeight != g_BitmapHeight)
        {
            if (g_BitmapHandle) DeleteObject(g_BitmapHandle);

            // Set up 16-bit RGB565 bitmap matching the game's surface format
            BITMAPINFO bitmapInfo = { { sizeof(BITMAPINFOHEADER), drawingWidth, -drawingHeight, 1, 16, BI_BITFIELDS } };
            ((DWORD*)&bitmapInfo.bmiColors)[0] = 0xF800; // Red mask (5 bits)
            ((DWORD*)&bitmapInfo.bmiColors)[1] = 0x07E0; // Green mask (6 bits)
            ((DWORD*)&bitmapInfo.bmiColors)[2] = 0x001F; // Blue mask (5 bits)

            g_BitmapHandle = CreateDIBSection(g_DeviceContext, &bitmapInfo, DIB_RGB_COLORS, &g_BitmapData, nullptr, 0);
            g_BitmapWidth = drawingWidth;
            g_BitmapHeight = drawingHeight;
        }
        if (!g_BitmapHandle) { directDrawSurface->Unlock(); return false; }

        // Select font and bitmap into the device context for drawing
        HBITMAP oldBitmap = (HBITMAP)SelectObject(g_DeviceContext, g_BitmapHandle);
        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);
        int surfacePitch = directDrawSurface->GetPitch();
        int copyWidth = std::min(drawingWidth, surfaceWidth - clampedX);

        // Copy the current surface background into the bitmap to preserve what's behind the text
        for (int y = 0; y < drawingHeight && (clampedY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)g_BitmapData + y * drawingWidth * 2,
                   (uint8_t*)surfaceBuffer + y * surfacePitch, copyWidth * 2);
        }

        // Convert the game's BGR565 color format to RGB for GDI
        uint16_t gameColor = gameFont ? gameFont->Color : 0x7FFF;
        SetTextColor(g_DeviceContext, RGB(((gameColor >> 11) & 0x1F) << 3,
                                         ((gameColor >> 5) & 0x3F) << 2,
                                         (gameColor & 0x1F) << 3));
        SetBkMode(g_DeviceContext, TRANSPARENT); // Text background is transparent

        // Build text formatting flags from the alignment parameter
        // Bits 0-1: horizontal alignment (0=left, 1=center, 2=right)
        // Bit 2: vertical center
        UINT drawingFlags = DT_NOPREFIX;
        if (alignment & 1)       drawingFlags |= DT_CENTER;
        else if (alignment & 2)  drawingFlags |= DT_RIGHT;
        else                     drawingFlags |= DT_LEFT;

        // Define the text drawing rectangle within the bitmap, offset for clamped position
        RECT textBoundaryBox = { offsetX, offsetY, offsetX + drawingWidth, offsetY + drawingHeight };

        // Single character from Blit hook - force horizontal layout
        if (wcslen(text) == 1 && boxWidth <= 0 && boxHeight <= 0)
            drawingFlags |= DT_SINGLELINE;
        // Button-sized text - single line with vertical centering
        else if (boxHeight > 0 && boxHeight < 30)
            drawingFlags |= DT_SINGLELINE | DT_VCENTER;
        // Multi-line text (tooltips, messages) - allow word wrapping
        else
            drawingFlags |= DT_WORDBREAK | DT_NOCLIP;

        // Render the text using Windows GDI
        DrawTextW(g_DeviceContext, text, -1, &textBoundaryBox, drawingFlags);
        GdiFlush(); // Ensure all drawing commands are processed

        // Copy the rendered text back to the game surface
        for (int y = 0; y < drawingHeight && (clampedY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)surfaceBuffer + y * surfacePitch,
                   (uint8_t*)g_BitmapData + y * drawingWidth * 2, copyWidth * 2);
        }

        // Restore previous GDI objects and release the surface lock
        SelectObject(g_DeviceContext, oldFont);
        SelectObject(g_DeviceContext, oldBitmap);
        directDrawSurface->Unlock();
        return true;
    }

    // Measures the pixel dimensions of a text string without drawing it.
    // Used by the game to allocate appropriate box sizes for text.
    bool GetTextDimension(BitFont*, const wchar_t* text, int* outWidth, int* outHeight, int maxWidth)
    {
        if (!text || !*text) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);

        // Set up measurement rectangle with word wrapping
        RECT boundaryCalculationBox = { 0, 0, maxWidth > 0 ? maxWidth : 2000, 0 };
        DrawTextW(g_DeviceContext, text, -1, &boundaryCalculationBox,
                  DT_CALCRECT | DT_NOCLIP | DT_WORDBREAK);

        SelectObject(g_DeviceContext, oldFont);

        // Return the calculated text dimensions
        if (outWidth)  *outWidth = boundaryCalculationBox.right - boundaryCalculationBox.left;
        if (outHeight) *outHeight = boundaryCalculationBox.bottom - boundaryCalculationBox.top;
        return true;
    }
}

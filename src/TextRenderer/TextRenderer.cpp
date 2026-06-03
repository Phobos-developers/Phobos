#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>
#include <algorithm>

namespace TextRenderer
{
    // Global Win32 GDI handles reused across draw calls to optimize rendering performance
    static HFONT g_FontHandle = nullptr;
    static HDC g_DeviceContext = nullptr;
    static bool g_FontLoaded = false;

    // Persistent Device Independent Bitmap (DIB) cache variables to avoid constant reallocations
    static void* g_BitmapData = nullptr;
    static HBITMAP g_BitmapHandle = nullptr;
    static int g_BitmapWidth = 0;
    static int g_BitmapHeight = 0;

    // Verifies that both the logical font and its device context are valid and ready
    bool IsInitialized()
    {
        return g_FontHandle != nullptr && g_DeviceContext != nullptr;
    }

    // Parses configuration from uimd.ini and creates the native TrueType font handle once
    static void LoadFontOnce()
    {
        if (g_FontLoaded) return;
        g_FontLoaded = true;

        CCINIClass config;
        config.LoadFromFile(GameStrings::UIMD_INI);
        if (!config.ReadBool("EnableTTF", "Enabled", false)) return;

        char fileName[MAX_PATH];
        config.ReadString("Font", "FileName", "arial.ttf", fileName);
        int fontSize = config.ReadInteger("FontSize", "LatinSize", 14);

        // Instantiates the font using Win32 ANSI signature matching
        g_FontHandle = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FF_DONTCARE, fileName);

        if (g_FontHandle)
            g_DeviceContext = CreateCompatibleDC(nullptr);
    }

    // Blends TrueType fonts onto the game's DirectDraw surfaces using a temporary GDI buffer
    bool DrawText(BitFont* gameFont, Surface* gameSurface, const wchar_t* text,
        int posX, int posY, int boxWidth, int boxHeight, int alignment)
    {
        if (!text || !*text || !gameSurface) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        DSurface* directDrawSurface = static_cast<DSurface*>(gameSurface);
        int surfaceWidth = directDrawSurface->GetWidth();
        int surfaceHeight = directDrawSurface->GetHeight();

        // Shorthand offset tweak to vertically realign tiny text layouts inside buttons
        if (boxHeight > 0 && boxHeight < 30)
            posY = std::max(0, posY - 2);

        // Determine drawing bounds. Fall back to remaining screen dimensions if none provided
        int drawingWidth = boxWidth > 0 ? boxWidth : surfaceWidth - posX;
        int drawingHeight = boxHeight > 0 ? boxHeight : surfaceHeight - posY;
        drawingWidth = (drawingWidth + 1) & ~1; // Force 16-bit word alignment step boundaries

        if (drawingWidth <= 0 || drawingHeight <= 0 || posX >= surfaceWidth || posY >= surfaceHeight)
            return false;

        // Obtain a direct memory address mapping relative to the target sub-coordinates lock location
        int lockX = std::max(0, posX);
        int lockY = std::max(0, posY);
        void* surfaceBuffer = directDrawSurface->Lock(lockX, lockY);
        if (!surfaceBuffer) return false;

        // Reallocate our temporary GDI backbuffer only when text frame dimension requirements scale
        if (drawingWidth != g_BitmapWidth || drawingHeight != g_BitmapHeight)
        {
            if (g_BitmapHandle) DeleteObject(g_BitmapHandle);

            BITMAPINFO bitmapInfo = { { sizeof(BITMAPINFOHEADER), drawingWidth, -drawingHeight, 1, 16, BI_BITFIELDS } };
            ((DWORD*)&bitmapInfo.bmiColors)[0] = 0xF800; // RGB565 Red Channel Mask
            ((DWORD*)&bitmapInfo.bmiColors)[1] = 0x07E0; // RGB565 Green Channel Mask
            ((DWORD*)&bitmapInfo.bmiColors)[2] = 0x001F; // RGB565 Blue Channel Mask

            g_BitmapHandle = CreateDIBSection(g_DeviceContext, &bitmapInfo, DIB_RGB_COLORS, &g_BitmapData, nullptr, 0);
            g_BitmapWidth = drawingWidth;
            g_BitmapHeight = drawingHeight;
        }
        if (!g_BitmapHandle) { directDrawSurface->Unlock(); return false; }

        HBITMAP oldBitmap = (HBITMAP)SelectObject(g_DeviceContext, g_BitmapHandle);
        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);
        int surfacePitch = directDrawSurface->GetPitch();
        int copyWidth = std::min(drawingWidth, surfaceWidth - lockX);

        // Wipe the temporary bitmap surface to prevent trailing artifacts and inject background pixels
        memset(g_BitmapData, 0, drawingWidth * drawingHeight * 2);
        for (int y = 0; y < drawingHeight && (lockY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)g_BitmapData + y * drawingWidth * 2,
                   (uint8_t*)surfaceBuffer + y * surfacePitch, copyWidth * 2);
        }

        // De-serialize the game engine's BGR565 coloring scheme to raw Win32 COLORREF channels
        uint16_t gameColor = gameFont ? gameFont->Color : 0x7FFF;
        SetTextColor(g_DeviceContext, RGB(((gameColor >> 11) & 0x1F) << 3,
                                         ((gameColor >> 5) & 0x3F) << 2,
                                         (gameColor & 0x1F) << 3));
        SetBkMode(g_DeviceContext, TRANSPARENT);

        // Map internal alignment flags to Windows standard layout formatting options
        UINT drawingFlags = DT_NOPREFIX;
        if (alignment & 1)       drawingFlags |= DT_CENTER;
        else if (alignment & 2)  drawingFlags |= DT_RIGHT;
        else                     drawingFlags |= DT_LEFT;

        RECT textBoundaryBox = { 0, 0, drawingWidth, drawingHeight };
        if (wcslen(text) == 1 && boxWidth <= 0 && boxHeight <= 0)
            drawingFlags |= DT_SINGLELINE;
        else if (boxHeight > 0 && boxHeight < 30)
            drawingFlags |= DT_SINGLELINE | DT_VCENTER;
        else
            drawingFlags |= DT_WORDBREAK | DT_NOCLIP;

        DrawTextW(g_DeviceContext, text, -1, &textBoundaryBox, drawingFlags);
        GdiFlush();

        // Copy the composited image back over onto the live display surface pipelines
        for (int y = 0; y < drawingHeight && (lockY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)surfaceBuffer + y * surfacePitch,
                   (uint8_t*)g_BitmapData + y * drawingWidth * 2, copyWidth * 2);
        }

        SelectObject(g_DeviceContext, oldFont);
        SelectObject(g_DeviceContext, oldBitmap);
        directDrawSurface->Unlock();
        return true;
    }

    // Calculates the required dimensions bounding box for a string without printing it to the screen
    bool GetTextDimension(BitFont*, const wchar_t* text, int* outWidth, int* outHeight, int maxWidth)
    {
        if (!text || !*text) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);
        RECT boundaryCalculationBox = { 0, 0, maxWidth > 0 ? maxWidth : 2000, 0 };
        DrawTextW(g_DeviceContext, text, -1, &boundaryCalculationBox, DT_CALCRECT | DT_NOCLIP | DT_WORDBREAK);
        SelectObject(g_DeviceContext, oldFont);

        if (outWidth)  *outWidth = boundaryCalculationBox.right - boundaryCalculationBox.left;
        if (outHeight) *outHeight = boundaryCalculationBox.bottom - boundaryCalculationBox.top;
        return true;
    }
}

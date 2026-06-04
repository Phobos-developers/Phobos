#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>
#include <algorithm>

namespace TextRenderer
{
    static HFONT g_FontHandle = nullptr;
    static HDC g_DeviceContext = nullptr;
    static bool g_FontLoaded = false;
    static void* g_BitmapData = nullptr;
    static HBITMAP g_BitmapHandle = nullptr;
    static int g_BitmapWidth = 0;
    static int g_BitmapHeight = 0;

    bool IsInitialized()
    {
        return g_FontHandle != nullptr && g_DeviceContext != nullptr;
    }

    static void LoadFontOnce()
    {
        if (g_FontLoaded) return;
        g_FontLoaded = true;

        CCINIClass config;
        config.LoadFromFile(GameStrings::UIMD_INI);
        if (!config.ReadBool("EnableTTF", "Enabled", false)) return;

        char fileName[MAX_PATH];
        config.ReadString("EnableTTF", "FontName", "arial.ttf", fileName);
        int fontSize = config.ReadInteger("EnableTTF", "FontSize", 14);

        // Anti-aliasing: false = no smoothing (default), true = ClearType
        bool antiAlias = config.ReadBool("EnableTTF", "AntiAlias", false);
        DWORD quality = antiAlias ? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY;

        g_FontHandle = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            quality, FF_DONTCARE, fileName);

        if (g_FontHandle)
            g_DeviceContext = CreateCompatibleDC(nullptr);
    }

    bool DrawText(BitFont* gameFont, Surface* gameSurface, const wchar_t* text,
        int posX, int posY, int boxWidth, int boxHeight, int alignment)
    {
        if (!text || !*text || !gameSurface) return false;
        LoadFontOnce();
        if (!g_FontHandle || !g_DeviceContext) return false;

        DSurface* directDrawSurface = static_cast<DSurface*>(gameSurface);
        int surfaceWidth = directDrawSurface->GetWidth();
        int surfaceHeight = directDrawSurface->GetHeight();

        if (boxHeight > 0 && boxHeight < 30)
            posY = std::max(0, posY - 2);

        int clampedX = std::max(0, posX);
        int clampedY = std::max(0, posY);
        int offsetX = posX - clampedX;
        int offsetY = posY - clampedY;

        int drawingWidth = boxWidth > 0 ? boxWidth : surfaceWidth - posX;
        int drawingHeight = boxHeight > 0 ? boxHeight : surfaceHeight - posY;
        drawingWidth -= offsetX;
        drawingHeight -= offsetY;
        drawingWidth = (drawingWidth + 1) & ~1;

        if (drawingWidth <= 0 || drawingHeight <= 0 || clampedX >= surfaceWidth || clampedY >= surfaceHeight)
            return false;

        void* surfaceBuffer = directDrawSurface->Lock(clampedX, clampedY);
        if (!surfaceBuffer) return false;

        if (drawingWidth != g_BitmapWidth || drawingHeight != g_BitmapHeight)
        {
            if (g_BitmapHandle) DeleteObject(g_BitmapHandle);

            BITMAPINFO bitmapInfo = { { sizeof(BITMAPINFOHEADER), drawingWidth, -drawingHeight, 1, 16, BI_BITFIELDS } };
            ((DWORD*)&bitmapInfo.bmiColors)[0] = 0xF800;
            ((DWORD*)&bitmapInfo.bmiColors)[1] = 0x07E0;
            ((DWORD*)&bitmapInfo.bmiColors)[2] = 0x001F;

            g_BitmapHandle = CreateDIBSection(g_DeviceContext, &bitmapInfo, DIB_RGB_COLORS, &g_BitmapData, nullptr, 0);
            g_BitmapWidth = drawingWidth;
            g_BitmapHeight = drawingHeight;
        }
        if (!g_BitmapHandle) { directDrawSurface->Unlock(); return false; }

        HBITMAP oldBitmap = (HBITMAP)SelectObject(g_DeviceContext, g_BitmapHandle);
        HFONT oldFont = (HFONT)SelectObject(g_DeviceContext, g_FontHandle);
        int surfacePitch = directDrawSurface->GetPitch();
        int copyWidth = std::min(drawingWidth, surfaceWidth - clampedX);

        memset(g_BitmapData, 0, drawingWidth * drawingHeight * 2);
        for (int y = 0; y < drawingHeight && (clampedY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)g_BitmapData + y * drawingWidth * 2,
                   (uint8_t*)surfaceBuffer + y * surfacePitch, copyWidth * 2);
        }

        uint16_t gameColor = gameFont ? gameFont->Color : 0x7FFF;
        SetTextColor(g_DeviceContext, RGB(((gameColor >> 11) & 0x1F) << 3,
                                         ((gameColor >> 5) & 0x3F) << 2,
                                         (gameColor & 0x1F) << 3));
        SetBkMode(g_DeviceContext, TRANSPARENT);

        UINT drawingFlags = DT_NOPREFIX;
        if (alignment & 1)       drawingFlags |= DT_CENTER;
        else if (alignment & 2)  drawingFlags |= DT_RIGHT;
        else                     drawingFlags |= DT_LEFT;

        RECT textBoundaryBox = { offsetX, offsetY, offsetX + drawingWidth, offsetY + drawingHeight };
        if (wcslen(text) == 1 && boxWidth <= 0 && boxHeight <= 0)
            drawingFlags |= DT_SINGLELINE;
        else if (boxHeight > 0 && boxHeight < 30)
            drawingFlags |= DT_SINGLELINE | DT_VCENTER;
        else
            drawingFlags |= DT_WORDBREAK | DT_NOCLIP;

        DrawTextW(g_DeviceContext, text, -1, &textBoundaryBox, drawingFlags);
        GdiFlush();

        for (int y = 0; y < drawingHeight && (clampedY + y) < surfaceHeight; y++)
        {
            memcpy((uint8_t*)surfaceBuffer + y * surfacePitch,
                   (uint8_t*)g_BitmapData + y * drawingWidth * 2, copyWidth * 2);
        }

        SelectObject(g_DeviceContext, oldFont);
        SelectObject(g_DeviceContext, oldBitmap);
        directDrawSurface->Unlock();
        return true;
    }

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

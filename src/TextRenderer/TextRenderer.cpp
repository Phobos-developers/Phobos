#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <Phobos.h>
#include <GameStrings.h>
#include <Utilities/Debug.h>
#include <map>

namespace TextRenderer
{
    static GDIFont g_Font;
    static bool g_Initialized = false;
    static std::map<int, HFONT> g_FontCache;

    bool Initialize()
    {
        if (g_Initialized) return true;

        CCINIClass ini;
        ini.LoadFromFile(GameStrings::UIMD_INI);

        if (!ini.ReadBool("EnableTTF", "Enabled", false))
        {
            Debug::Log("TextRenderer: TTF disabled in UIMD.INI\n");
            return false;
        }

        char fontFile[MAX_PATH];
        ini.ReadString("Font", "FileName", "arial.ttf", fontFile);
        g_Font.fontName = std::wstring(fontFile, fontFile + strlen(fontFile));
        g_Font.fontSize = ini.ReadInteger("FontSize", "LatinSize", 14);
        if (g_Font.fontSize <= 0) g_Font.fontSize = 14;

        Debug::Log("TextRenderer: Initializing GDI with font '%S' size=%d\n",
                   g_Font.fontName.c_str(), g_Font.fontSize);

        g_Initialized = true;
        return true;
    }

    HFONT GetOrCreateFont(int size)
    {
        auto it = g_FontCache.find(size);
        if (it != g_FontCache.end())
            return it->second;

        HFONT hFont = CreateFontW(
            size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FF_DONTCARE, g_Font.fontName.c_str()
        );

        if (hFont)
            g_FontCache[size] = hFont;

        return hFont;
    }

    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int alignment)
    {
        if (!pText || !*pText || !pSurface) return false;
        if (!Initialize()) return false;

        DSurface* pDSurface = static_cast<DSurface*>(pSurface);
        if (!pDSurface) return false;

        int sw = pDSurface->GetWidth();
        int sh = pDSurface->GetHeight();

        int drawX = X;
        int drawY = Y;
        int drawW = W > 0 ? W : sw - X;
        int drawH = H > 0 ? H : sh - Y;

        if (drawX < 0) drawX = 0;
        if (drawY < 0) drawY = 0;
        if (drawX + drawW > sw) drawW = sw - drawX;
        if (drawY + drawH > sh) drawH = sh - drawY;

        if (drawW <= 0 || drawH <= 0) return false;

        void* surfaceBuffer = pDSurface->Lock(drawX, drawY);
        if (!surfaceBuffer) return false;

        int pitch = pDSurface->GetPitch();

        // Create a 16-bit 565 DIB matching the surface format
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = drawW;
        bmi.bmiHeader.biHeight = -drawH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 16;
        bmi.bmiHeader.biCompression = BI_BITFIELDS;

        // Set the bitmasks for RGB565 format
        DWORD* masks = (DWORD*)(&bmi.bmiColors);
        masks[0] = 0xF800;  // Red mask (5 bits)
        masks[1] = 0x07E0;  // Green mask (6 bits)
        masks[2] = 0x001F;  // Blue mask (5 bits)

        HDC hdc = CreateCompatibleDC(nullptr);
        if (!hdc) { pDSurface->Unlock(); return false; }

        void* dibBits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &dibBits, nullptr, 0);
        if (!hBitmap) { DeleteDC(hdc); pDSurface->Unlock(); return false; }

        HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc, hBitmap);

        // Copy current surface contents to DIB
        for (int y = 0; y < drawH; y++)
            memcpy((uint8_t*)dibBits + y * drawW * 2,
                   (uint8_t*)surfaceBuffer + y * pitch,
                   drawW * 2);

        HFONT hFont = GetOrCreateFont(g_Font.fontSize);
        if (!hFont)
        {
            SelectObject(hdc, hOldBmp); DeleteObject(hBitmap);
            DeleteDC(hdc); pDSurface->Unlock();
            return false;
        }

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // Convert game color (BGR565) to RGB for GDI
        uint16_t color = pFont ? pFont->Color : 0x7FFF;
        int b = ((color >> 11) & 0x1F) * 255 / 31;
        int g = ((color >> 5) & 0x3F) * 255 / 63;
        int r = (color & 0x1F) * 255 / 31;
        SetTextColor(hdc, RGB(r, g, b));
        SetBkMode(hdc, TRANSPARENT);

        DWORD dwFlags = DT_NOCLIP | DT_NOPREFIX;
        if (alignment & 1) dwFlags |= DT_CENTER;
        else if (alignment & 2) dwFlags |= DT_RIGHT;

        bool hasArabic = false;
        int len = wcslen(pText);
        for (int i = 0; i < len; i++)
        {
            if ((pText[i] >= 0x0600 && pText[i] <= 0x06FF))
            { hasArabic = true; break; }
        }
        if (hasArabic) dwFlags |= DT_RTLREADING;

        RECT rect;
        rect.left = X - drawX;
        rect.top = Y - drawY;
        if (W > 0)
            rect.right = rect.left + W;
        else
            rect.right = drawW;
        if (H > 0)
            rect.bottom = rect.top + H;
        else
            rect.bottom = drawH;

        DrawTextW(hdc, pText, -1, &rect, dwFlags);
        GdiFlush();

        // Copy DIB back to surface
        for (int y = 0; y < drawH; y++)
            memcpy((uint8_t*)surfaceBuffer + y * pitch,
                   (uint8_t*)dibBits + y * drawW * 2,
                   drawW * 2);

        SelectObject(hdc, hOldFont);
        SelectObject(hdc, hOldBmp);
        DeleteObject(hBitmap);
        DeleteDC(hdc);
        pDSurface->Unlock();

        return true;
    }

    bool GetTextDimension(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth)
    {
        if (pWidth) *pWidth = 0;
        if (pHeight) *pHeight = 0;
        if (!pText || !*pText) return false;
        if (!Initialize()) return false;

        HDC hdc = GetDC(nullptr);
        if (!hdc) return false;

        HFONT hFont = GetOrCreateFont(g_Font.fontSize);
        if (!hFont)
        {
            ReleaseDC(nullptr, hdc);
            return false;
        }

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        RECT rect = { 0, 0, nMaxWidth > 0 ? nMaxWidth : 0, 0 };
        DWORD dwFlags = DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX;

        bool hasArabic = false;
        int len = wcslen(pText);
        for (int i = 0; i < len; i++)
        {
            if ((pText[i] >= 0x0600 && pText[i] <= 0x06FF))
            { hasArabic = true; break; }
        }
        if (hasArabic) dwFlags |= DT_RTLREADING;

        DrawTextW(hdc, pText, -1, &rect, dwFlags);

        if (pWidth) *pWidth = rect.right - rect.left;
        if (pHeight) *pHeight = rect.bottom - rect.top;

        SelectObject(hdc, hOldFont);
        ReleaseDC(nullptr, hdc);
        return true;
    }
}

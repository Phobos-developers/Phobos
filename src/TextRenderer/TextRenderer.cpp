#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>

namespace TextRenderer
{
    static HFONT g_hFont = nullptr;
    static bool g_Loaded = false;

    static void LoadFont()
    {
        if (g_Loaded) return;
        g_Loaded = true;

        CCINIClass ini;
        ini.LoadFromFile(GameStrings::UIMD_INI);
        if (!ini.ReadBool("EnableTTF", "Enabled", false)) return;

        char fontFile[MAX_PATH];
        ini.ReadString("Font", "FileName", "arial.ttf", fontFile);
        int fontSize = ini.ReadInteger("FontSize", "LatinSize", 14);

        g_hFont = CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FF_DONTCARE,
            std::wstring(fontFile, fontFile + strlen(fontFile)).c_str());
    }

    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int)
    {
        if (!pText || !*pText || !pSurface) return false;
        LoadFont();
        if (!g_hFont) return false;

        DSurface* pDSurface = static_cast<DSurface*>(pSurface);
        int sw = pDSurface->GetWidth(), sh = pDSurface->GetHeight();
        int dw = W > 0 ? W : sw - X, dh = H > 0 ? H : sh - Y;

        void* buf = pDSurface->Lock(std::max(0, X), std::max(0, Y));
        if (!buf) return false;

        HDC hdc = CreateCompatibleDC(nullptr);
        BITMAPINFO bmi = { { sizeof(BITMAPINFOHEADER), dw, -dh, 1, 16, BI_BITFIELDS } };
        DWORD* masks = (DWORD*)&bmi.bmiColors;
        masks[0] = 0xF800; masks[1] = 0x07E0; masks[2] = 0x001F;

        void* dib;
        HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &dib, nullptr, 0);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc, hBmp);

        for (int y = 0; y < dh; y++)
            memcpy((uint8_t*)dib + y * dw * 2, (uint8_t*)buf + y * pDSurface->GetPitch(), dw * 2);

        HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFont);
        uint16_t c = pFont ? pFont->Color : 0x7FFF;
        SetTextColor(hdc, RGB((c & 0x1F) * 8, ((c >> 5) & 0x3F) * 4, ((c >> 11) & 0x1F) * 8));
        SetBkMode(hdc, TRANSPARENT);

        RECT rect = { X - std::max(0, X), Y - std::max(0, Y), dw, dh };
        DrawTextW(hdc, pText, -1, &rect, DT_NOCLIP | DT_NOPREFIX);
        GdiFlush();

        for (int y = 0; y < dh; y++)
            memcpy((uint8_t*)buf + y * pDSurface->GetPitch(), (uint8_t*)dib + y * dw * 2, dw * 2);

        SelectObject(hdc, hOldFont); SelectObject(hdc, hOldBmp);
        DeleteObject(hBmp); DeleteDC(hdc);
        pDSurface->Unlock();
        return true;
    }

    bool GetTextDimension(BitFont*, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth)
    {
        if (!pText || !*pText) return false;
        LoadFont();
        if (!g_hFont) return false;

        HDC hdc = GetDC(nullptr);
        HFONT hOld = (HFONT)SelectObject(hdc, g_hFont);
        RECT rect = { 0, 0, nMaxWidth, 0 };
        DrawTextW(hdc, pText, -1, &rect, DT_CALCRECT | DT_NOCLIP);
        SelectObject(hdc, hOld);
        ReleaseDC(nullptr, hdc);

        if (pWidth) *pWidth = rect.right;
        if (pHeight) *pHeight = rect.bottom;
        return true;
    }
}

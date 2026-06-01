#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <GameStrings.h>
#include <algorithm>

namespace TextRenderer {
    static HFONT g_hFont = nullptr;
    static HDC g_hDC = nullptr;
    static bool g_Loaded = false;

    static void LoadFont() {
        if (g_Loaded) return;
        g_Loaded = true;
        CCINIClass ini; ini.LoadFromFile(GameStrings::UIMD_INI);
        if (!ini.ReadBool("EnableTTF", "Enabled", false)) return;
        char file[MAX_PATH]; ini.ReadString("Font", "FileName", "arial.ttf", file);
        int size = ini.ReadInteger("FontSize", "LatinSize", 14);
        g_hFont = CreateFontW(size, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE,
            std::wstring(file, file + strlen(file)).c_str());
        if (g_hFont) g_hDC = CreateCompatibleDC(nullptr);
    }

    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText, int X, int Y, int W, int H, int alignment) {
        if (!pText || !*pText || !pSurface) return false;
        LoadFont(); if (!g_hFont || !g_hDC) return false;
        DSurface* pDS = static_cast<DSurface*>(pSurface);
        int sw = pDS->GetWidth(), sh = pDS->GetHeight();
        if (H > 0 && H < 30) { Y -= 2; if (Y < 0) Y = 0; }
        int dw = W > 0 ? W : sw - X, dh = H > 0 ? H : sh - Y;
        dw = (dw + 1) & ~1;
        if (dw <= 0 || dh <= 0 || X >= sw || Y >= sh) return false;
        void* buf = pDS->Lock(0, 0); if (!buf) return false;
        BITMAPINFO bmi = { { sizeof(BITMAPINFOHEADER), dw, -dh, 1, 16, BI_BITFIELDS } };
        ((DWORD*)&bmi.bmiColors)[0] = 0xF800; ((DWORD*)&bmi.bmiColors)[1] = 0x07E0; ((DWORD*)&bmi.bmiColors)[2] = 0x001F;
        void* dib; HBITMAP hBmp = CreateDIBSection(g_hDC, &bmi, DIB_RGB_COLORS, &dib, nullptr, 0);
        if (!hBmp) { pDS->Unlock(); return false; }
        HBITMAP hOldBmp = (HBITMAP)SelectObject(g_hDC, hBmp);
        HFONT hOldFont = (HFONT)SelectObject(g_hDC, g_hFont);
        int pitch = pDS->GetPitch();
        for (int y = 0; y < dh && (Y + y) < sh; y++) {
            if (Y + y < 0) continue; // Crash protection: Prevents negative address multiplication
            memcpy((uint8_t*)dib + y * dw * 2, (uint8_t*)buf + (Y + y) * pitch + (X * 2), std::min(dw, sw - X) * 2);
        }
        uint16_t c = pFont ? pFont->Color : 0x7FFF;
        SetTextColor(g_hDC, RGB(((c >> 11) & 0x1F) << 3, ((c >> 5) & 0x3F) << 2, (c & 0x1F) << 3));
        SetBkMode(g_hDC, TRANSPARENT);
        UINT flags = DT_NOPREFIX;
        if (alignment & 1) flags |= DT_CENTER; else if (alignment & 2) flags |= DT_RIGHT; else flags |= DT_LEFT;
        RECT r = { 0, 0, W > 0 ? W : (dw - 8), dh };
        if (H > 0 && H < 30) flags |= DT_SINGLELINE | DT_VCENTER; else flags |= DT_WORDBREAK | DT_NOCLIP;
        DrawTextW(g_hDC, pText, -1, &r, flags); GdiFlush();
        for (int y = 0; y < dh && (Y + y) < sh; y++) {
            if (Y + y < 0) continue; // Crash protection: Mirror safety block for writeback pass
            memcpy((uint8_t*)buf + (Y + y) * pitch + (X * 2), (uint8_t*)dib + y * dw * 2, std::min(dw, sw - X) * 2);
        }
        SelectObject(g_hDC, hOldFont); SelectObject(g_hDC, hOldBmp); DeleteObject(hBmp); pDS->Unlock();
        return true;
    }

    bool GetTextDimension(BitFont*, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth) {
        if (!pText || !*pText) return false;
        LoadFont(); if (!g_hFont || !g_hDC) return false;
        HFONT hOld = (HFONT)SelectObject(g_hDC, g_hFont);
        RECT r = { 0, 0, nMaxWidth > 0 ? (nMaxWidth + 12) : 2000, 0 }; // Padding keeps RTL scripts from clipping
        DrawTextW(g_hDC, pText, -1, &r, DT_CALCRECT | DT_NOCLIP | DT_WORDBREAK);
        SelectObject(g_hDC, hOld);
        if (pWidth) *pWidth = (r.right - r.left);
        if (pHeight) *pHeight = r.bottom - r.top;
        return true;
    }
}

#include "TextRenderer.h"
#include <BitFont.h>
#include <Surface.h>
#include <CCINIClass.h>
#include <Phobos.h>
#include <GameStrings.h>
#include <Utilities/Debug.h>
#include <algorithm>

namespace TextRenderer
{
    static FT_Library gFTLibrary = nullptr;
    static FT_Face gFTFace = nullptr;
    static hb_font_t* gHbFont = nullptr;
    static bool gLoaded = false;
    static bool gEnabled = false;
    static int gFontSize = 14;

    static void LoadFontOnce()
    {
        if (gLoaded) return;
        gLoaded = true;

        CCINIClass ini;
        ini.LoadFromFile(GameStrings::UIMD_INI);

        gEnabled = ini.ReadBool("EnableTTF", "Enabled", false);
        if (!gEnabled)
        {
            Debug::Log("TextRenderer: TTF disabled in UIMD.INI\n");
            return;
        }

        char fontFile[MAX_PATH];
        ini.ReadString("Font", "FileName", "arial.ttf", fontFile);
        std::string fontPath = std::string("Fonts\\") + fontFile;

        gFontSize = ini.ReadInteger("FontSize", "LatinSize", 14);
        if (gFontSize <= 0) gFontSize = 14;

        Debug::Log("TextRenderer: Loading font '%s' size=%d\n", fontPath.c_str(), gFontSize);

        if (FT_Init_FreeType(&gFTLibrary) != 0)
        {
            Debug::Log("TextRenderer: FT_Init_FreeType failed\n");
            return;
        }
        if (FT_New_Face(gFTLibrary, fontPath.c_str(), 0, &gFTFace) != 0)
        {
            Debug::Log("TextRenderer: FT_New_Face failed\n");
            return;
        }
        FT_Set_Pixel_Sizes(gFTFace, 0, gFontSize);

        gHbFont = hb_ft_font_create_referenced(gFTFace);
        Debug::Log("TextRenderer: Font loaded successfully\n");
    }

    static bool IsArabicChar(wchar_t ch)
    {
        return (ch >= 0x0600 && ch <= 0x06FF) ||
               (ch >= 0x0750 && ch <= 0x077F) ||
               (ch >= 0xFB50 && ch <= 0xFDFF) ||
               (ch >= 0xFE70 && ch <= 0xFEFF);
    }

    static std::vector<Glyph> ShapeText(const wchar_t* text, int len)
    {
        std::vector<Glyph> result;
        if (!gHbFont || !text || len <= 0) return result;

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf16(buf, (const uint16_t*)text, len, 0, len);
        hb_buffer_guess_segment_properties(buf);

        bool hasArabic = false;
        for (int i = 0; i < len; i++)
        {
            if (IsArabicChar(text[i]))
            {
                hasArabic = true;
                break;
            }
        }

        if (hasArabic)
            hb_buffer_set_direction(buf, HB_DIRECTION_RTL);

        hb_shape(gHbFont, buf, nullptr, 0);

        unsigned int count = 0;
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &count);
        hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &count);

        for (unsigned j = 0; j < count; ++j)
        {
            Glyph g;
            g.id = info[j].codepoint;
            g.x_advance = pos[j].x_advance >> 6;
            g.x_offset = pos[j].x_offset >> 6;
            g.y_offset = pos[j].y_offset >> 6;

            wchar_t ch = (info[j].cluster < (uint32_t)len) ? text[info[j].cluster] : L'\0';
            g.isSpace = (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' ||
                         ch == 0x00A0);
            g.isDigit = (ch >= L'0' && ch <= L'9');
            result.push_back(g);
        }

        if (hasArabic)
        {
            int i = 0;
            while (i < (int)result.size())
            {
                if (result[i].isDigit)
                {
                    int start = i;
                    while (i < (int)result.size() && result[i].isDigit)
                        i++;
                    if (i - start > 1)
                        std::reverse(result.begin() + start, result.begin() + i);
                }
                else
                {
                    i++;
                }
            }
        }

        hb_buffer_destroy(buf);
        return result;
    }

    bool GetTextDimension(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth)
    {
        if (pWidth) *pWidth = 0;
        if (pHeight) *pHeight = 0;
        if (!pText || !*pText) return false;

        LoadFontOnce();
        if (!gEnabled || !gFTFace || !gHbFont) return false;

        int len = (int)wcslen(pText);
        auto glyphs = ShapeText(pText, len);
        if (glyphs.empty()) return false;

        int totalW = 0;
        for (const auto& g : glyphs)
            totalW += g.x_advance;

        if (pWidth) *pWidth = totalW;
        if (pHeight) *pHeight = gFTFace->size->metrics.height >> 6;
        return true;
    }

    bool DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pText,
        int X, int Y, int W, int H, int alignment)
    {
        if (!pText || !*pText || !pSurface) return false;

        LoadFontOnce();
        if (!gEnabled || !gFTFace || !gHbFont) return false;

        DSurface* pDSurface = static_cast<DSurface*>(pSurface);
        if (!pDSurface) return false;

        int len = (int)wcslen(pText);
        auto glyphs = ShapeText(pText, len);
        if (glyphs.empty()) return false;

        int totalWidth = 0;
        for (const auto& g : glyphs)
            totalWidth += g.x_advance;

        int startX = X;
        if (W > 0)
        {
            if (alignment & 1)
                startX = X + (W - totalWidth) / 2;
            else if (alignment & 2)
                startX = X + W - totalWidth;
        }

        int sw = pDSurface->GetWidth();
        int sh = pDSurface->GetHeight();

        int lockX = startX;
        int lockY = Y;
        int lockW = totalWidth + 10;
        int lockH = (gFTFace->size->metrics.height >> 6) + 10;

        if (lockX < 0) lockX = 0;
        if (lockY < 0) lockY = 0;
        if (lockX + lockW > sw) lockW = sw - lockX;
        if (lockY + lockH > sh) lockH = sh - lockY;

        void* buffer = pDSurface->Lock(lockX, lockY);
        if (!buffer) return false;

        int pitch = pDSurface->GetPitch();
        int bpp = pDSurface->GetBytesPerPixel();
        uint16_t color = pFont ? pFont->Color : 0xFFFF;

        int curX = startX;
        int baseline = Y + (gFTFace->size->metrics.ascender >> 6);

        for (const auto& g : glyphs)
        {
            if (g.isSpace)
            {
                curX += g.x_advance;
                continue;
            }

            if (FT_Load_Glyph(gFTFace, g.id, FT_LOAD_RENDER) != 0)
            {
                curX += g.x_advance;
                continue;
            }

            FT_Bitmap& bmp = gFTFace->glyph->bitmap;
            if (!bmp.buffer || bmp.rows == 0 || bmp.width == 0)
            {
                curX += g.x_advance;
                continue;
            }

            int dx = curX + gFTFace->glyph->bitmap_left + g.x_offset;
            int dy = baseline - gFTFace->glyph->bitmap_top + g.y_offset;

            for (unsigned int row = 0; row < bmp.rows; row++)
            {
                int py = dy + row;
                if (py < lockY || py >= lockY + lockH) continue;

                uint8_t* src = bmp.buffer + row * bmp.pitch;
                int localY = py - lockY;

                for (unsigned int col = 0; col < bmp.width; col++)
                {
                    int px = dx + col;
                    if (px < lockX || px >= lockX + lockW) continue;

                    uint8_t alpha = src[col];
                    if (alpha == 0) continue;

                    int localX = px - lockX;
                    uint16_t* dst = (uint16_t*)((uint8_t*)buffer + localY * pitch + localX * bpp);

                    if (alpha == 255)
                    {
                        *dst = color;
                    }
                    else
                    {
                        int r1 = ((color >> 11) & 0x1F);
                        int g1 = ((color >> 5) & 0x3F);
                        int b1 = (color & 0x1F);

                        int r2 = ((*dst >> 11) & 0x1F);
                        int g2 = ((*dst >> 5) & 0x3F);
                        int b2 = (*dst & 0x1F);

                        int r = r1 + (r2 - r1) * (255 - alpha) / 255;
                        int g = g1 + (g2 - g1) * (255 - alpha) / 255;
                        int b = b1 + (b2 - b1) * (255 - alpha) / 255;

                        *dst = (uint16_t)((r << 11) | (g << 5) | b);
                    }
                }
            }

            curX += g.x_advance;
        }

        pDSurface->Unlock();
        return true;
    }
}

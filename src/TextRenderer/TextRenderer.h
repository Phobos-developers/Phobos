#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>
#include <hb.h>
#include <hb-ft.h>
#include <fribidi.h>
#include <ft2build.h>
#include "BitFont.h"
#include "BitText.h"
#include <Surface.h>
#include <unordered_map>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using std::min;
using std::max;
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H 
namespace TextRenderer
{

	struct ShapedGlyph
	{
		hb_codepoint_t glyphId;
		int            x_advance;
		int y_advance;
		int            x_offset;
		int            y_offset;
		unsigned int   cluster;  // index into original text
		bool isSpace;
		wchar_t ch;
		bool isRTL;
	};
	struct FontLineMetrics
	{
		int ascender;
		int descender;
		int height;
		int baselineOffset; // extra: for alignment between fonts
		float leading;
	};
	struct Run
	{
		int  start;
		int  length;
		bool isRTL;
		bool isArabic;
		int  vi, vj; // visual span (needed for minimal fix)
	};
	FT_Face GetFTFace(BitFont* pFont);
	FT_Face GetFTFaceForText(BitFont* pFont, bool isRTL);
	hb_font_t* GetHbFontForText(BitFont* pFont, bool isRTL);
	bool IsRTLText(const wchar_t* text, int len);
	std::wstring FixUtf8InWchar(const wchar_t* ws);
	int MeasureRealAscender(FT_Face face, const wchar_t* testSet);
	int MeasureRealDescender(FT_Face face, const wchar_t* testSet);
	int GetCharacterWidth(BitFont* pFont, wchar_t ch);
	const wchar_t* FindLineEnd(BitFont* pFont, const wchar_t* start, int nMaxWidth);
	BitFont::InternalData* LoadTTFAsInternalData(const char* pFileName);
	int MeasureGlyphs(BitFont* pFont, const std::vector<ShapedGlyph>& glyphs);
	std::vector<ShapedGlyph> ShapeText(BitFont* pFont, const wchar_t* text, int len);
	void DrawText_FlushLine(BitFont* pFont, FT_Face face, const wchar_t* lineStart, const wchar_t* lineEnd, int X, int Y, int W, int a8, int a9, int nColorAdjust, int& shadowPos);
	void RenderGlyphs(BitFont* pFont, FT_Face face, const std::vector<ShapedGlyph>& glyphs, int startX, int startY, WORD color, int shadowStart, int fadeLen, bool isRTL, bool isDrawLine);
	BitFont* BitFont_CTOR_(BitFont* pFont, const char* pFileName);
	bool BitFont_GetTextDimension_(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth);
	bool BitText_DrawText_(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int X, int Y, int W, int H, int a8, int a9, int nColorAdjust);
	int BitFont_434500_(BitFont* pFont, wchar_t* pText, int xLeft, int yTop, int charCount, int nColorAdjust);
};

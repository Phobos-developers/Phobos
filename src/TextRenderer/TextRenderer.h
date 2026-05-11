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
#include <BitFont.h>
#include <BitText.h>
#include <Surface.h>
#include <unordered_map>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
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
	static FT_Library ftLib = nullptr;
	FT_Face GetFTFace(BitFont* pFont);
	FT_Face GetFTFaceForText(BitFont* pFont, bool isRTL);
	hb_font_t* GetHbFontForText(BitFont* pFont, bool isRTL);
	int MeasureRealAscender(FT_Face face, const wchar_t* testSet);
	int MeasureRealDescender(FT_Face face, const wchar_t* testSet);
	bool IsRTLText(const wchar_t* text, int len);
	BitFont::InternalData* LoadTTFAsInternalData(const char* pFileName);
	BitFont* BitFont_CTOR_(BitFont* pFont, const char* pFileName);
}

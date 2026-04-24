#include "TextRenderer.h"

namespace TextRenderer
{
	static const wchar_t LATIN_ASC_TEST[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890$";
	static const wchar_t LATIN_DESC_TEST[] = L"gjpqyQ";
	static const wchar_t ARABIC_ASC_TEST[] = L"أبتثجحخدذرزسشصضطظعغفقكلمنهوي";
	static const wchar_t ARABIC_DESC_TEST[] = L"جحخصضطظعغ";
	static const wchar_t ARABIC_DIACRITICS[] = L"ًٌٍَُِّْ";

	// FreeType faces
	// Latin/default rendering face
	std::unordered_map<BitFont*, FT_Face> gFTFaceMap;

	// Arabic rendering face (different metrics/fallback)
	std::unordered_map<BitFont*, FT_Face> gFTFaceArabicMap;

	// HarfBuzz fonts
	// Latin/default shaping font
	std::unordered_map<BitFont*, hb_font_t*> gHbFontMap;

	// Arabic shaping font (GSUB/GPOS for Arabic)
	std::unordered_map<BitFont*, hb_font_t*> gHbFontArabicMap;

	// Metrics (measured asc/desc)
	// Latin ascender / descender
	std::unordered_map<BitFont*, int> gAscenderMap;
	std::unordered_map<BitFont*, int> gDescenderMap;

	// Arabic ascender / descender
	std::unordered_map<BitFont*, int> gAscenderArabicMap;
	std::unordered_map<BitFont*, int> gDescenderArabicMap;

	hb_font_t* GetHbFont(BitFont* pFont)
	{
		auto it = gHbFontMap.find(pFont);
		return it != gHbFontMap.end() ? it->second : nullptr;
	}

	std::wstring FixUtf8InWchar(const wchar_t* ws)
	{
		if (!ws) return L"";

		bool isRTL = IsRTLText(ws, (int)wcslen(ws));
		if (isRTL)
			return std::wstring(ws);

		std::string utf8;
		while (*ws)
		{
			utf8.push_back(static_cast<char>(*ws));
			++ws;
		}
		if (utf8.empty())
			return L"";

		int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		if (len <= 0)
			return L"";

		std::wstring result(len - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, result.data(), len);

		return result;
	}

	int MeasureRealAscender(FT_Face face, const wchar_t* testSet)
	{
		if (!face || !testSet) return 0;

		int maxTop = 0;

		for (int i = 0; testSet[i]; i++)
		{
			FT_UInt idx = FT_Get_Char_Index(face, testSet[i]);
			if (!idx) continue;

			if (FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT) != 0)
				continue;

			int top = face->glyph->metrics.horiBearingY >> 6;

			if (top > maxTop)
				maxTop = top;
		}

		return maxTop > 0 ? maxTop : (int)(face->size->metrics.ascender >> 6);
	}

	int MeasureRealDescender(FT_Face face, const wchar_t* testSet)
	{
		if (!face || !testSet) return 0;

		int maxBelow = 0;

		for (int i = 0; testSet[i]; i++)
		{
			FT_UInt idx = FT_Get_Char_Index(face, testSet[i]);
			if (!idx) continue;

			if (FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT) != 0)
				continue;

			FT_GlyphSlot slot = face->glyph;

			int top = slot->metrics.horiBearingY >> 6;
			int height = slot->metrics.height >> 6;

			int below = height - top;

			if (below > maxBelow)
				maxBelow = below;
		}

		return maxBelow > 0 ? -maxBelow : (int)(face->size->metrics.descender >> 6);
	}

	// Select correct FreeType face for rendering (Arabic vs Latin)
	FT_Face GetFTFaceForText(BitFont* pFont, bool isRTL)
	{
		if (!pFont) return nullptr;

		if (isRTL)
		{
			auto it = gFTFaceArabicMap.find(pFont);
			if (it != gFTFaceArabicMap.end())
				return it->second;
		}

		return GetFTFace(pFont);
	}

	// Select correct HarfBuzz font for shaping (Arabic vs Latin)
	hb_font_t* GetHbFontForText(BitFont* pFont, bool isRTL)
	{
		if (isRTL)
		{
			auto it = gHbFontArabicMap.find(pFont);
			if (it != gHbFontArabicMap.end())
				return it->second;
		}

		return GetHbFont(pFont);
	}

	// Returns the primary FreeType face used for rendering Latin/default text.
	// This is the base FT_Face (non‑Arabic). Arabic uses GetFTFaceForText() instead.
	FT_Face GetFTFace(BitFont* pFont)
	{
		auto it = gFTFaceMap.find(pFont);
		return it != gFTFaceMap.end() ? it->second : nullptr;
	}
	// RTL Arabic Text 
	bool IsRTLText(const wchar_t* text, int len)
	{
		for (int i = 0; i < len; i++)
		{
			wchar_t ch = text[i];
			if ((ch >= 0x0600 && ch <= 0x06FF) || // Arabic
				(ch >= 0x0750 && ch <= 0x077F) || // Arabic Supplement
				(ch >= 0x08A0 && ch <= 0x08FF) || // Arabic Extended-A
				(ch >= 0xFB50 && ch <= 0xFDFF) || // Arabic Presentation Forms-A
				(ch >= 0xFE70 && ch <= 0xFEFF))  // Arabic Presentation Forms-B
				return true;
		}
		return false;
	}

	const wchar_t* FindLineEnd(BitFont* pFont, const wchar_t* start, int nMaxWidth)
	{
		if (!pFont || !start || !*start)
			return start;

		// No wrapping → stop only at newline
		if (nMaxWidth <= 0)
		{
			const wchar_t* p = start;
			while (*p && *p != L'\n' && *p != L'\r')
				++p;
			return p;
		}

		// Count logical characters until newline
		int len = 0;
		while (start[len] && start[len] != L'\n' && start[len] != L'\r')
			++len;

		if (len == 0)
			return start;

		// Shape entire logical segment once (cluster‑safe)
		auto shaped = ShapeText(pFont, start, len);
		if (shaped.empty())
			return start + len;

		// Per‑codepoint width map
		std::vector<int> charWidth(len, 0);
		std::vector<bool> isTab(len, false);

		const int tabSize = (pFont->Unknown_28 > 0) ? pFont->Unknown_28 : 64;

		// Accumulate cluster advances
		for (const auto& g : shaped)
		{
			int idx = (int)g.cluster;
			if (idx < 0 || idx >= len)
				continue;

			wchar_t ch = start[idx];

			if (ch == L'\t')
			{
				isTab[idx] = true;
			}
			else if (ch == 0x200C || ch == 0x200D || ch == 0x200B) // ZWNJ, ZWJ, ZWSP
			{
				charWidth[idx] = 0;
			}
			else
			{
				charWidth[idx] += g.x_advance;
			}
		}

		// Helpers
		auto isHigh = [](wchar_t c) { return (c >= 0xD800 && c <= 0xDBFF); };
		auto isLow = [](wchar_t c) { return (c >= 0xDC00 && c <= 0xDFFF); };

		auto isBreakable = [](wchar_t ch)
			{
				switch (ch)
				{
				case L' ': case L'\t': case L'-': case 0x00AD: case 0x200B:
					return true;
				case 0x00A0: case 0x2011: case 0x2060: case 0xFEFF:
					return false;
				}
				return (ch <= 0x20 || (ch >= 0x2000 && ch <= 0x200A));
			};

		auto script = [](wchar_t ch)
			{
				// Arabic family
				if ((ch >= 0x0600 && ch <= 0x06FF) || (ch >= 0x0750 && ch <= 0x077F) || (ch >= 0x08A0 && ch <= 0x08FF) ||
					(ch >= 0xFB50 && ch <= 0xFDFF) || (ch >= 0xFE70 && ch <= 0xFEFF))
					return 1;

				// Hebrew
				if (ch >= 0x0590 && ch <= 0x05FF)
					return 2;

				// Latin
				if ((ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z') ||
					(ch >= 0x00C0 && ch <= 0x00FF) || (ch >= 0x0100 && ch <= 0x017F))
					return 3;

				// Cyrillic
				if ((ch >= 0x0400 && ch <= 0x04FF) || (ch >= 0x0500 && ch <= 0x052F))
					return 4;

				// Greek
				if ((ch >= 0x0370 && ch <= 0x03FF) || (ch >= 0x1F00 && ch <= 0x1FFF))
					return 5;

				// CJK
				if ((ch >= 0x4E00 && ch <= 0x9FFF) || (ch >= 0x3400 && ch <= 0x4DBF) ||
					(ch >= 0x20000 && ch <= 0x2A6DF) || (ch >= 0xF900 && ch <= 0xFAFF) ||
					(ch >= 0x3040 && ch <= 0x309F) || (ch >= 0x30A0 && ch <= 0x30FF) ||
					(ch >= 0xAC00 && ch <= 0xD7AF))
					return 6;

				return 0;
			};

		// Main scan
		int currentW = 0;
		int lastBreakIdx = -1;

		for (int i = 0; i < len; i++)
		{
			wchar_t ch = start[i];

			// Skip isolated low surrogate
			if (isLow(ch))
				continue;

			// Resolve advance
			int adv = charWidth[i];

			if (isTab[i])
			{
				adv = tabSize - (currentW % tabSize);
				if (adv == 0) adv = tabSize;
			}

			// Surrogate pair: merge width of low surrogate
			if (isHigh(ch) && i + 1 < len && isLow(start[i + 1]))
				adv += charWidth[i + 1];

			// Wrap BEFORE adding this character
			if (currentW + adv > nMaxWidth)
			{
				if (lastBreakIdx >= 0)
				{
					// Avoid splitting surrogate pair
					if (isHigh(start[lastBreakIdx]) &&
						lastBreakIdx + 1 < len &&
						isLow(start[lastBreakIdx + 1]))
					{
						return start + lastBreakIdx + 2;
					}
					return start + lastBreakIdx + 1;
				}

				// Hard break
				if (i > 0)
				{
					if (isLow(ch) && isHigh(start[i - 1]))
						return start + i - 1;
					return start + i;
				}

				// Single wide character
				if (isHigh(ch) && i + 1 < len && isLow(start[i + 1]))
					return start + i + 2;
				return start + i + 1;
			}

			currentW += adv;

			// Skip low surrogate (already merged)
			if (isHigh(ch) && i + 1 < len && isLow(start[i + 1]))
				continue;

			// Standard break opportunities
			if (isBreakable(ch))
				lastBreakIdx = i;

			// Script boundary break (Arabic/Hebrew excluded)
			int sPrev = (i > 0) ? script(start[i - 1]) : 0;
			int sCurr = script(ch);

			if (i > 0 &&
				sPrev != 0 && sCurr != 0 &&
				sPrev != sCurr &&
				sPrev != 1 && sCurr != 1 &&   // Arabic
				sPrev != 2 && sCurr != 2)     // Hebrew
			{
				lastBreakIdx = i - 1;
			}

			// CJK: break after any char except forbidden starts
			if (sCurr == 6 && i > 0)
			{
				wchar_t next = (i + 1 < len) ? start[i + 1] : 0;
				bool forbidden = (next == 0x3002 ||  // 。 Ideographic full stop
				  next == 0x3001 ||  // 、 Ideographic comma
				  next == 0x300B ||  // 》 Right angle bracket
				  next == 0x300D ||  // 」 Right corner bracket
				  next == 0x3011 ||  // 』 Right black lenticular bracket
				  next == 0x3009);   // 〉 Right angle bracket

				if (!forbidden)
					lastBreakIdx = i;
			}
		}

		// Entire line fits
		return start + len;
	}
	// Measures the visual width of a shaped line [lineStart, lineEnd)
	// Uses HarfBuzz clusters so multi‑glyph clusters (Arabic, ligatures) are handled correctly.
	int MeasureLineWidth(BitFont* pFont, const wchar_t* lineStart, const wchar_t* lineEnd, int tabSize)
	{
		// Shape the logical substring into glyphs (HB handles bidi + clusters)
		auto glyphs = ShapeText(pFont, lineStart, lineEnd - lineStart);
		if (glyphs.empty())
			return 0;

		int width = 0;

		// Walk shaped glyphs in visual order
		for (size_t i = 0; i < glyphs.size(); i++)
		{
			const auto& g = glyphs[i];

			// Map glyph → original logical codepoint index
			wchar_t ch = lineStart[g.cluster];

			// Tabs: snap to next tab stop (same logic used in rendering)
			if (ch == L'\t')
			{
				// advance = distance to next tab boundary
				width += tabSize - (width % tabSize);
			}
			else
			{
				// Normal glyph: use HarfBuzz x_advance (already in 26.6 → pixels)
				width += g.x_advance;
			}
		}

		return width;
	}

	hb_codepoint_t GetFallbackGlyph(hb_font_t* hbFont, bool isRTL)
	{
		if (!hbFont) return 0; // .notdef

		hb_codepoint_t glyph = 0;

		// Universal: replacement character U+FFFD �
		if (hb_font_get_nominal_glyph(hbFont, 0xFFFD, &glyph))
			return glyph;

		//  White square — visible, script-neutral placeholder
		if (hb_font_get_nominal_glyph(hbFont, 0x25A1, &glyph))
			return glyph;

		//  Script-specific fallback
		if (isRTL)
		{
			if (hb_font_get_nominal_glyph(hbFont, 0x061F, &glyph)) return glyph; // ؟ Arabic question mark
		}

		// 4) Latin '?' — almost every font has this
		if (hb_font_get_nominal_glyph(hbFont, L'?', &glyph))
			return glyph;

		// 5) .notdef — always exists at index 0
		return 0;
	}
	std::vector<ShapedGlyph> ShapeText(BitFont* pFont, const wchar_t* text, int len)
	{
		std::vector<ShapedGlyph> result;
		if (!pFont || !text || len <= 0)
			return result;

		//  FriBidi: logical to visual, levels, maps
		std::vector<FriBidiChar>     logical(len);
		std::vector<FriBidiChar>     visual(len);
		std::vector<FriBidiStrIndex> lToV(len); // logical to visual
		std::vector<FriBidiLevel>    levels(len);

		for (int i = 0; i < len; ++i)
			logical[i] = (FriBidiChar)(unsigned short)text[i];

		FriBidiParType baseDir = FRIBIDI_PAR_ON;

		FriBidiLevel maxLevel = fribidi_log2vis(logical.data(), (FriBidiStrIndex)len, &baseDir, visual.data(), lToV.data(), nullptr, levels.data());

		// build visual → logical map
		std::vector<int> vToL(len);
		for (int li = 0; li < len; ++li)
			vToL[lToV[li]] = li;

		// levels in visual order
		std::vector<FriBidiLevel> visualLevels(len);
		for (int li = 0; li < len; ++li)
			visualLevels[lToV[li]] = levels[li];

		// Split into runs by contiguous visual level 


		std::vector<Run> runs;

		if (maxLevel == 0)
		{
			bool isArabic = IsRTLText(text, len);
			runs.push_back({ 0, len, false, isArabic, 0, len });
		}
		else
		{
			int vi = 0;
			while (vi < len)
			{
				int vj = vi + 1;
				while (vj < len && visualLevels[vj] == visualLevels[vi])
					++vj;

				bool isRTL = (visualLevels[vi] & 1) != 0;

				int minL = vToL[vi];
				int maxL = vToL[vi];
				for (int v = vi + 1; v < vj; ++v)
				{
					int li = vToL[v];
					if (li < minL) minL = li;
					if (li > maxL) maxL = li;
				}

				int logStart = minL;
				int logLen = maxL - minL + 1;

				bool isArabic = IsRTLText(text + logStart, logLen);

				runs.push_back({ logStart, logLen, isRTL, isArabic, vi, vj });
				vi = vj;
			}
		}

		// Minimal Fix Helper  
		auto AddRunToHB = [&](hb_buffer_t* buf, const Run& run)
			{
				for (int logical = run.start; logical < run.start + run.length; logical++)
				{
					bool belongs = false;

					for (int v = run.vi; v < run.vj; v++)
					{
						if (vToL[v] == logical)
						{
							belongs = true;
							break;
						}
					}
					if (!belongs)
						continue;

					hb_buffer_add_utf16(buf, (const uint16_t*)text, len, logical, 1);
				}
			};

		//  Shape each run with HarfBuzz  
		hb_feature_t features[] = {
			{ HB_TAG('r','l','i','g'), 1, 0, (unsigned)-1 },
			{ HB_TAG('c','c','m','p'), 1, 0, (unsigned)-1 },
			{ HB_TAG('l','o','c','l'), 1, 0, (unsigned)-1 },
			{ HB_TAG('m','a','r','k'), 1, 0, (unsigned)-1 },
			{ HB_TAG('m','k','m','k'), 1, 0, (unsigned)-1 },
			{ HB_TAG('c','u','r','s'), 1, 0, (unsigned)-1 },
			{ HB_TAG('k','e','r','n'), 1, 0, (unsigned)-1 },
			{ HB_TAG('l','i','g','a'), 1, 0, (unsigned)-1 },
		};
		const unsigned featureCount = sizeof(features) / sizeof(features[0]);

		for (const auto& run : runs)
		{
			hb_font_t* hbFont = GetHbFontForText(pFont, run.isArabic);
			if (!hbFont)
				continue;

			FT_Face face = GetFTFaceForText(pFont, run.isArabic);
			if (face)
				hb_ft_font_changed(hbFont);

			hb_buffer_t* buf = hb_buffer_create();
			if (!buf)
				continue;
			hb_buffer_set_unicode_funcs(buf, hb_unicode_funcs_get_default());
			hb_buffer_set_cluster_level(buf, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

			// Minimal Fix: add only characters that belong to this visual run
			AddRunToHB(buf, run);

			if (run.isRTL)
			{
				hb_buffer_set_direction(buf, HB_DIRECTION_RTL);
				hb_buffer_set_script(buf, HB_SCRIPT_ARABIC);
				hb_buffer_set_language(buf, hb_language_from_string("ar", -1));
			}
			else
			{
				hb_buffer_guess_segment_properties(buf);
			}

			hb_shape(hbFont, buf, features, featureCount);

			unsigned int count = 0;
			hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &count);
			hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &count);
			hb_codepoint_t fallback = GetFallbackGlyph(hbFont, run.isArabic);

			for (unsigned i = 0; i < count; ++i)
			{
				ShapedGlyph g {};
				g.glyphId = info[i].codepoint;
				g.x_advance = pos[i].x_advance >> 6;
				g.x_offset = pos[i].x_offset >> 6;
				g.y_offset = pos[i].y_offset >> 6;
				g.cluster = info[i].cluster;

				wchar_t ch = L'?';
				if (g.cluster < (uint32_t)len)
					ch = text[g.cluster];

				g.ch = ch;
				g.isSpace = (ch == L' ' || ch == 0x00A0 || ch == L'\t');
				g.isRTL = run.isRTL;

				if (g.glyphId == 0)
				{
					if (fallback != 0)
						g.glyphId = fallback;
					else
						continue;
				}

				result.push_back(g);
			}

			hb_buffer_destroy(buf);
		}

		return result;
	}
	void BitFont_434700(BitFont*)
	{
		JMP_STD(0x434700);
	}
	BitFont* BitFont_CTOR_(BitFont* pFont, const char* pFileName)
	{
		CCINIClass ini_uimd {};
		ini_uimd.LoadFromFile(GameStrings::UIMD_INI);
		//  Basic defaults 
		pFont->InternalPTR = nullptr;
		pFont->Pointer_8 = nullptr;
		pFont->pGraphBuffer = (short*)1;
		pFont->field_1C = 1;
		pFont->Unknown_14 = 0;
		pFont->field_18 = nullptr;
		pFont->field_20 = 0;
		pFont->Color = 0x7FFF;
		pFont->DefaultColor2 = 0x3555;
		pFont->Unknown_28 = 64;
		pFont->State_2C = 0;
		pFont->Bounds = { 0, 0, 0, 0 };
		pFont->Bool_40 = true;
		pFont->field_41 = true;
		pFont->field_42 = false;
		pFont->field_43 = false;

		// Load internal font data

		if (!pFont->InternalPTR)
			pFont->InternalPTR = LoadTTFAsInternalData(pFileName);

		pFont->field_18 = (wchar_t*)(intptr_t)pFont->InternalPTR->Stride;
		pFont->field_1C = pFont->InternalPTR->Lines;

		// Read config
		int latinSize = ini_uimd.ReadInteger("FontSize", "LatinSize", 0);
		int arabicSize = ini_uimd.ReadInteger("FontSize", "ArabicSize", 0);
		int targetH = pFont->InternalPTR->FontHeight;
		if (targetH <= 0) targetH = 14;
		if (latinSize <= 0) latinSize = targetH;
		if (arabicSize <= 0) arabicSize = latinSize;
		// Init FreeType
		FT_Library gFTLibrary = nullptr;
		if (!gFTLibrary)
		{
			if (FT_Init_FreeType(&gFTLibrary) != 0)
			{
				pFont->State_2C = 1;
				return pFont;
			}
		}

		// Create a single FT_Face
		FT_Face ftFace = nullptr;
		if (FT_New_Face(gFTLibrary, pFileName, 0, &ftFace) != 0 || !ftFace)
		{
			pFont->State_2C = 1;
			return pFont;
		}

		// Set FT pixel size ONCE
		int basePixelSize = std::max(latinSize, arabicSize);
		if (basePixelSize <= 0) basePixelSize = 14;

		if (FT_Set_Pixel_Sizes(ftFace, 0, basePixelSize) != 0)
		{
			FT_Done_Face(ftFace);
			pFont->State_2C = 1;
			return pFont;
		}

		// Measure REAL Latin + Arabic metrics
		int latinAsc = MeasureRealAscender(ftFace, LATIN_ASC_TEST) + 2; // fixed 
		int latinDesc = MeasureRealDescender(ftFace, LATIN_DESC_TEST);

		int arabicAsc = MeasureRealAscender(ftFace, ARABIC_ASC_TEST);
		int arabicDesc = MeasureRealDescender(ftFace, ARABIC_DESC_TEST);

		// Store per-script metrics
		gAscenderMap[pFont] = latinAsc;
		gDescenderMap[pFont] = latinDesc;

		gAscenderArabicMap[pFont] = arabicAsc;
		gDescenderArabicMap[pFont] = arabicDesc;

		gFTFaceMap[pFont] = ftFace;
		gFTFaceArabicMap[pFont] = ftFace;

		int maxAdv = (int)(ftFace->size->metrics.max_advance >> 6);

		// Create HarfBuzz fonts
		hb_font_t* hbLatin = hb_ft_font_create_referenced(ftFace);
		hb_ft_font_set_load_flags(hbLatin, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM);
		hb_font_t* hbArabic = hb_ft_font_create_referenced(ftFace);
		hb_ft_font_set_load_flags(hbArabic, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM);

		int baseX = 0, baseY = 0;
		hb_font_get_scale(hbLatin, &baseX, &baseY);
		if (baseX == 0 || baseY == 0)
		{
			int upem = ftFace->units_per_EM ? ftFace->units_per_EM : 2048;
			baseX = baseY = upem;
		}

		float latinRatio = (float)latinSize / (float)basePixelSize;
		float arabicRatio = (float)arabicSize / (float)basePixelSize;

		hb_font_set_scale(hbLatin, (int)(baseX * latinRatio), (int)(baseY * latinRatio));
		hb_font_set_scale(hbArabic, (int)(baseX * arabicRatio), (int)(baseY * arabicRatio));

		gHbFontMap[pFont] = hbLatin;
		gHbFontArabicMap[pFont] = hbArabic;

		// Unified line metrics
		FontLineMetrics m;
		m.ascender = std::max(latinAsc, arabicAsc);
		m.descender = std::min(latinDesc, arabicDesc);
		m.height = m.ascender - m.descender;

		pFont->InternalPTR->FontHeight = m.height;
		pFont->InternalPTR->Lines = m.height;
		pFont->InternalPTR->FontWidth = maxAdv;
		pFont->InternalPTR->Stride = maxAdv;

		pFont->field_18 = (wchar_t*)(intptr_t)maxAdv;
		pFont->field_1C = m.height;

		//  Original post-init
		BitFont_434700(pFont);
		return pFont;
	}

	// Measures total width/height of a multi-line UTF‑16 string using shaping + wrapping.
	// Uses FindLineEnd() for wrapping and MeasureLineWidth() for per-line width.
	bool BitFont_GetTextDimension_(BitFont* pFont, const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth)
	{
		if (pWidth)  *pWidth = 0;
		if (pHeight) *pHeight = 0;

		// Validate input
		if (!pFont || !pFont->InternalPTR || !pText || !*pText)
			return false;

		const int lineH = pFont->InternalPTR->FontHeight; // fixed line height
		const int tabSize = (pFont->Unknown_28 > 0) ? pFont->Unknown_28 : 64;
		const int extraW = pFont->State_2C; // extra spacing after space-like chars

		int maxW = 0;
		int totalH = 0;

		const wchar_t* lineStart = pText;

		// Iterate logical lines until NUL
		while (*lineStart)
		{
			// Compute end of this line (wrap-aware)
			const wchar_t* lineEnd = FindLineEnd(pFont, lineStart, nMaxWidth);

			// Advance to next line (skip CR/LF sequences)
			const wchar_t* next = lineEnd;
			if (*next == L'\r')
				next = (*(next + 1) == L'\n') ? next + 2 : next + 1;
			else if (*next == L'\n')
				next = next + 1;

			const int lineLen = (int)(lineEnd - lineStart);

			if (lineLen > 0)
			{
				// Base width from shaped glyphs (tabs handled inside MeasureLineWidth)
				int lineWidth = MeasureLineWidth(pFont, lineStart, lineEnd, tabSize);

				// Add extra spacing after each non-tab, non-last character
				if (extraW > 0 && lineLen > 1)
				{
					for (int i = 0; i < lineLen - 1; i++)
					{
						wchar_t ch = lineStart[i];
						if (ch != L'\t')
							lineWidth += extraW;
					}
				}

				// Track widest line
				maxW = std::max(maxW, lineWidth);

				// Add one visual line height
				totalH += lineH;
			}
			else if (lineStart == pText && lineLen == 0)
			{
				// First line is empty → still counts as one line
				totalH = lineH;
			}

			lineStart = next;
		}

		if (pWidth)  *pWidth = maxW;
		if (pHeight) *pHeight = totalH;
		return true;
	}

	BitFont::InternalData* LoadTTFAsInternalData(const char* pFileName)
	{
		CCINIClass ini_uimd {};
		ini_uimd.LoadFromFile(GameStrings::UIMD_INI);
		int LatinSize = ini_uimd.ReadInteger("FontSize", "LatinSize", 0);
		//int ArabicSize = ini_uimd.ReadInteger("FontSize", "ArabicSize",0));
		FT_Library gFTLibrary = nullptr;
		if (!gFTLibrary)
			if (FT_Init_FreeType(&gFTLibrary) != 0) return nullptr;

		FT_Face ftFace;
		if (FT_New_Face(gFTLibrary, pFileName, 0, &ftFace) != 0)
			return nullptr;

		FT_Set_Pixel_Sizes(ftFace, 0, LatinSize);

		// scan metrics only — no rendering needed
		int maxW = 0;
		int maxAbove = 0;
		int maxBelow = 0;

		for (int cp = 0x20; cp < 0x10000; cp++)
		{
			FT_UInt idx = FT_Get_Char_Index(ftFace, cp);
			if (!idx) continue;

			if (FT_Load_Glyph(ftFace, idx, FT_LOAD_DEFAULT) != 0) // metrics only
				continue;

			FT_GlyphSlot slot = ftFace->glyph;

			int w = (int)(slot->metrics.width >> 6);
			int above = (int)(slot->metrics.horiBearingY >> 6);
			int below = (int)((slot->metrics.height - slot->metrics.horiBearingY) >> 6);

			if (w > maxW)     maxW = w;
			if (above > maxAbove) maxAbove = above;
			if (below > maxBelow) maxBelow = below;
		}

		if (maxW == 0 || (maxAbove + maxBelow) == 0)
		{
			FT_Done_Face(ftFace);
			return nullptr;
		}

		int glyphH = maxAbove + maxBelow;
		int bytesPerRow = (maxW + 7) / 8;
		int dataSize = 1 + bytesPerRow * glyphH;

		// Allocate
		BitFont::InternalData* data = new BitFont::InternalData();
		memset(data, 0, sizeof(BitFont::InternalData));

		data->FontWidth = maxW;
		data->Stride = bytesPerRow;
		data->FontHeight = glyphH;
		data->Lines = glyphH;
		data->Count = 0x20000;
		data->SymbolDataSize = dataSize;
		data->SymbolTable = new short[0x20000]();
		data->Bitmaps = new char[0x20000 * dataSize]();

		// render glyphs into atlas
		for (int cp = 0x20; cp < 0x20000; cp++)
		{
			FT_UInt idx = FT_Get_Char_Index(ftFace, cp);
			if (!idx) continue;

			if (FT_Load_Glyph(ftFace, idx, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO) != 0)
				continue;

			FT_GlyphSlot slot = ftFace->glyph;
			FT_Bitmap& bmp = slot->bitmap;

			if (bmp.pixel_mode != FT_PIXEL_MODE_MONO) continue;
			if (!bmp.buffer || bmp.rows == 0 || bmp.width == 0) continue;

			data->SymbolTable[cp] = (uint16_t)cp;

			char* dst = data->Bitmaps + cp * dataSize;
			int advance = (int)(slot->advance.x >> 6);
			dst[0] = (uint8_t)(advance < 255 ? advance : 255);

			// correct vertical placement in cell
			int baseY = maxAbove - slot->bitmap_top;  //  was: slot->bitmap_top
			int baseX = slot->bitmap_left;
			int pitch = std::abs(bmp.pitch);          //  handle negative pitch

			for (int row = 0; row < (int)bmp.rows; row++)
			{
				int destRow = baseY + row;
				if (destRow < 0 || destRow >= glyphH) continue;

				const uint8_t* srcRow = bmp.buffer + row * pitch;
				uint8_t* dstRow = (uint8_t*)(dst + 1 + destRow * bytesPerRow);

				for (int col = 0; col < (int)bmp.width; col++)
				{
					int destBit = col + baseX;
					if (destBit < 0 || destBit >= maxW) continue;

					int bit = (srcRow[col / 8] >> (7 - (col % 8))) & 1;
					if (!bit) continue;

					dstRow[destBit / 8] |= (uint8_t)(1 << (7 - (destBit % 8)));
				}
			}

			data->ValidSymbolCount++;
		}

		FT_Done_Face(ftFace);
		return data;
	}

	// Draws multi-line shaped text with wrapping, trimming, and RTL awareness.
	// Uses FindLineEnd() for wrapping and DrawText_FlushLine() for per-line rendering.
	bool BitText_DrawText_(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int X, int Y, int W, int H, int a8, int a9, int nColorAdjust)
	{
		if (!pFont || !pWideString || !pSurface)
			return false;

		FT_Face face = GetFTFace(pFont);
		if (!face)
			return false;

		WORD originalColor = pFont->Color;
		pFont->Lock(pSurface);
		pFont->SetField20(X);                     // internal left offset

		// Line height fallback (prevents infinite loop on bad fonts)
		int fontH = (pFont->field_1C > 0) ? pFont->field_1C : 14;

		int surfaceW = pSurface->GetWidth();
		int surfaceH = pSurface->GetHeight();

		// Wrap width and vertical clip region
		int wrapW = (W > 0) ? W : (surfaceW - X);
		int maxH = (H > 0) ? H : (surfaceH - Y);

		// Detect global RTL for trimming rules
		int totalLen = (int)wcslen(pWideString);
		bool textIsRTL = IsRTLText(pWideString, totalLen);

		int lineY = Y;
		int shadowPos = 0;

		const wchar_t* currentPos = pWideString;

		// Process logical lines until NUL or vertical clip
		while (*currentPos)
		{
			// Vertical clipping (single check per line)
			if (lineY + fontH > Y + maxH) break;
			if (lineY + fontH > surfaceH) break;

			const wchar_t* lineStart = currentPos;
			const wchar_t* lineEnd = FindLineEnd(pFont, currentPos, wrapW);

			// Advance past newline sequences (\r, \n, \r\n)
			if (*lineEnd == L'\r')
				currentPos = (*(lineEnd + 1) == L'\n') ? lineEnd + 2 : lineEnd + 1;
			else if (*lineEnd == L'\n')
				currentPos = lineEnd + 1;
			else
				currentPos = lineEnd;

			// Trim leading/trailing spaces for LTR only
			// (Arabic spacing is visually meaningful → preserved)
			if (!textIsRTL)
			{
				while (lineStart < lineEnd && *lineStart == L' ')
					++lineStart;
				while (lineEnd > lineStart && *(lineEnd - 1) == L' ')
					--lineEnd;
			}

			// Empty visual line → just advance Y
			if (lineStart >= lineEnd)
			{
				lineY += fontH;
				continue;
			}

			// Render shaped + aligned line
			DrawText_FlushLine(pFont, face, lineStart, lineEnd, X, lineY, wrapW, a8, a9, nColorAdjust, shadowPos);

			lineY += fontH;
		}

		pFont->UnLock(pSurface);
		pFont->Color = originalColor;
		return true;
	}

	// Computes a fade-to-white color based on glyph index.
	WORD CalculateFade(uint16_t baseColor, int index, int shadowStart, int fadeLen)
	{
		if (!shadowStart)
			return baseColor;  // no fade active

		if (index >= shadowStart)
			return 0xFFFF; // fully white

		int dist = shadowStart - index - 1;
		if (dist >= fadeLen)
			return baseColor; // outside fade zone

		int step = fadeLen - dist;
		int factor = step * 255 / fadeLen; // 0..255 fade strength

		int r = ((baseColor >> 11) & 0x1F) * 255 / 31;
		int g = ((baseColor >> 5) & 0x3F) * 255 / 63;
		int b = (baseColor & 0x1F) * 255 / 31;

		// Linear blend toward white
		r = r + (255 - r) * factor / 255;
		g = g + (255 - g) * factor / 255;
		b = b + (255 - b) * factor / 255;

		// Convert back to RGB565
		return (uint16_t)(((r * 31 / 255) << 11) | ((g * 63 / 255) << 5) | ((b * 31 / 255)));
	}

	void RenderGlyphs(BitFont* pFont, FT_Face face, const std::vector<ShapedGlyph>& glyphs, int startX, int startY, WORD color, int shadowStart, int fadeLen, bool isRTL, bool isDrawline)
	{
		if (!face || glyphs.empty()) return;

		uint16_t* graphbuf = (uint16_t*)pFont->pGraphBuffer;
		int pitch2 = pFont->PitchDiv2;
		if (!graphbuf || !pitch2) return;


		// Select FT face (Latin or Arabic) but DO NOT use its metrics
		FT_Face renderFace = isRTL ? GetFTFaceForText(pFont, true) : face;
		if (!renderFace) renderFace = face;

		color = pFont->Color;


		// Unified metrics (correct asc/desc)
		int ascender = isRTL ? gAscenderArabicMap[pFont] : gAscenderMap[pFont];


		int baselineY = startY + ascender;  // baseline position


		// Clipping
		int clipL = pFont->Bounds.Left;
		int clipR = pFont->Bounds.Right;
		int clipT = pFont->Bounds.Top;
		int clipB = pFont->Bounds.Bottom;

		if (clipR <= clipL || clipB <= clipT)
		{
			clipL = 0;
			clipT = 0;
			clipR = pitch2 - 1;
			clipB = pitch2 - 1;
		}

		clipL = std::max(0, clipL);
		clipT = std::max(0, clipT);
		clipR = std::min(pitch2 - 1, clipR);


		// Compute bounding box
		int totalWidth = 0;
		for (const auto& g : glyphs)
			totalWidth += std::abs(g.x_advance);

		int boxL = startX;
		int boxR = startX + totalWidth;
		if (boxR > pitch2 - 1) boxR = pitch2 - 1;

		int effL = std::max(clipL, boxL); // effective left
		int effR = std::min(clipR, boxR); // effective right

		if (effL >= effR) return;
		if (baselineY < clipT || baselineY > clipB) return;

		// Render loop
		int curX = startX;
		if (!isDrawline)
		{
			int len = (int)glyphs.size();
			fadeLen = len - shadowStart;   // chars before fade zone
			if (fadeLen < 0)
				fadeLen = 0;
			isDrawline = true;
		}
		for (int i = 0; i < (int)glyphs.size(); i++)
		{
			const auto& g = glyphs[i];

			color = CalculateFade(color, i, shadowStart, fadeLen);
			if (color == 0xFFFF)
				break;

			int advance = std::abs(g.x_advance);

			if (g.glyphId == 0)
			{
				curX += advance;
				continue;
			}

			if (FT_Load_Glyph(renderFace, g.glyphId, FT_LOAD_RENDER) != 0)
			{
				FT_UInt fb = FT_Get_Char_Index(renderFace, L'?');
				if (fb == 0 || FT_Load_Glyph(renderFace, fb, FT_LOAD_RENDER) != 0)
				{
					curX += advance;
					continue;
				}
			}

			FT_GlyphSlot slot = renderFace->glyph;
			FT_Bitmap& bmp = slot->bitmap;

			if (!bmp.buffer || bmp.rows == 0 || bmp.width == 0)
			{
				curX += advance;
				continue;
			}

			// Correct baseline positioning
			int drawX = curX + slot->bitmap_left + (g.x_offset >> 6);
			int drawY = baselineY - slot->bitmap_top + (g.y_offset >> 6);

			// Draw bitmap
			for (int row = 0; row < (int)bmp.rows; row++)
			{
				int py = drawY + row;
				if (py < clipT || py > clipB) continue;

				uint8_t* srcRow = bmp.buffer + row * bmp.pitch;
				uint16_t* dstRow = graphbuf + py * pitch2;

				for (int col = 0; col < (int)bmp.width; col++)
				{
					int px = drawX + col;
					if (px < effL || px > effR) continue;

					uint8_t alpha = srcRow[col];
					if (!alpha) continue;

					if (alpha == 255)
					{
						dstRow[px] = color;
					}
					else
					{
						RGBClass fg((int)color, true);
						RGBClass bg((int)dstRow[px], true);
						RGBClass out(bg.Red + (fg.Red - bg.Red) * alpha / 255, bg.Green + (fg.Green - bg.Green) * alpha / 255, bg.Blue + (fg.Blue - bg.Blue) * alpha / 255);
						dstRow[px] = (uint16_t)Drawing::RGB_To_Int(out.Red, out.Green, out.Blue);
					}
				}
			}

			curX += advance;
			pFont->Color = color;
			if (curX > effR)
				break;
		}
	}

	int MeasureGlyphs(BitFont* pFont, const std::vector<ShapedGlyph>& glyphs)
	{

		int  w = 0;
		for (const auto& g : glyphs)
			w += g.x_advance;
		return w;
	}

	int GetCharacterWidth(BitFont* pFont, wchar_t ch)
	{
		if (!pFont || !pFont->InternalPTR) return 0;

		FT_Face face = GetFTFace(pFont);
		if (face)
		{
			// TTF path — HarfBuzz x_advance already includes all spacing
			// State_2C must NOT be added here — it is a .FNT-only concept
			FT_UInt idx = FT_Get_Char_Index(face, (FT_ULong)ch);
			if (idx && FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT) == 0)
				return (int)(face->glyph->advance.x >> 6);

			// glyph not in font — return space width as fallback
			if (ch == L' ')
			{
				// derive from em size — more accurate than field_1C fraction
				if (face->units_per_EM > 0)
				{
					// space is typically ~25% of em
					int pixelSize = (int)(face->size->metrics.x_ppem);
					return pixelSize > 0 ? pixelSize / 4 : pFont->field_1C / 4;
				}
			}

			return 0;
		}
		uint8_t* bitmap = pFont->GetCharacterBitmap(ch);
		if (!bitmap) bitmap = pFont->GetCharacterBitmap(L'?');
		if (!bitmap) return 0;
		return bitmap[0];
	}

	// Draws one shaped line with alignment and bounds.
	void DrawText_FlushLine(BitFont* pFont, FT_Face face, const wchar_t* lineStart, const wchar_t* lineEnd, int X, int Y, int W, int flag, int a9, int nColorAdjust, int& shadowPos)
	{
		int len = (int)(lineEnd - lineStart);
		if (len <= 0 || !face)
			return;

		WORD color = pFont->Color;
		if (color == 0xFFFF)  // invisible
			return;

		bool isRTL = IsRTLText(lineStart, len);

		// Shape the logical substring [lineStart, lineEnd)
		auto glyphs = ShapeText(pFont, lineStart, len);
		if (glyphs.empty())
			return;
		// Total shaped width (x_advance sum)
		int lineW = MeasureGlyphs(pFont, glyphs);

		int boundL = X;
		int boundR = (W > 0 ? X + W : pFont->PitchDiv2 - 1);
		int boxW = boundR - boundL;

		if (lineW > boxW)
			lineW = boxW;

		// Alignment: left / center / right
		int startX = boundL;
		if (flag & 1)
			startX = boundL + (boxW - lineW) / 2;
		else if (flag & 2)
			startX = boundR - lineW;

		// Hard clamp to bounds
		if (startX < boundL)
			startX = boundL;
		if (startX > boundR)
			startX = boundR;
		if (startX + lineW > boundR)
			startX = boundR - lineW;
		// Select correct FT_Face (Arabic/Latin)
		FT_Face renderFace = GetFTFaceForText(pFont, isRTL);

		if (!renderFace)
			renderFace = face;

		// Render shaped glyphs
		RenderGlyphs(pFont, renderFace, glyphs, startX, Y, color, a9, nColorAdjust, isRTL, true);

		shadowPos += len; // consider using glyphs.size()
	}

	// Used as a simple left‑padding operation for RTL
	void MoveLastTwoToFront(wchar_t* str)
	{
		if (!str) return;

		int len = (int)wcslen(str);
		if (len < 2) return;

		// Only rotate if last two chars are spaces
		if (str[len - 2] != L' ' || str[len - 1] != L' ')
			return;

		wchar_t a = str[len - 2];
		wchar_t b = str[len - 1];

		// Shift right by 2
		for (int i = len - 3; i >= 0; --i)
			str[i + 2] = str[i];

		str[0] = a;
		str[1] = b;
	}

	// Draws a single line of text using HarfBuzz shaping.
	int BitFont_434500_(BitFont* pFont, wchar_t* pText, int xLeft, int yTop, int charCount, int nColorAdjust)
	{
		if (!pText || !*pText)
			return xLeft;

		int size = wcslen(pText);
		if (IsRTLText(pText, size))
			MoveLastTwoToFront(pText);

		FT_Face face = GetFTFace(pFont);
		if (!face)
			return xLeft;

		WORD savedColor = pFont->Color;

		// visible length
		int len = 0;
		while (pText[len] && pText[len] != L'\r' && pText[len] != L'\n')
		{
			if (charCount > 0 && len >= charCount)
				break;
			len++;
		}

		if (len <= 0)
		{
			pFont->Color = savedColor;
			return xLeft;
		}

		// HarfBuzz shaping
		bool isRTL = IsRTLText(pText, len);
		auto glyphs = ShapeText(pFont, pText, len);

		if (!glyphs.empty())
		{
			int lineW = 0;
			for (const auto& g : glyphs)
				lineW += g.x_advance >> 6;

			int boundL = xLeft;
			int boundR = pFont->PitchDiv2 - 1;

			int startX = boundL;
			if (startX + lineW > boundR)
				startX = boundR - lineW;
			if (startX < boundL)
				startX = boundL;

			FT_Face renderFace = GetFTFaceForText(pFont, isRTL);
			if (!renderFace)
				renderFace = face;

			RenderGlyphs(pFont, renderFace, glyphs, startX, yTop, savedColor, nColorAdjust, 0, isRTL, false);

			xLeft = startX + lineW;
		}

		pFont->Color = savedColor;
		return xLeft;
	}
}

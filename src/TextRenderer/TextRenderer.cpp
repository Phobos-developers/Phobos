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
		if (len <= 0) return false;

		// Convert wchar_t to UTF-32 (or use hb_buffer_add_utf16 directly)
		hb_buffer_t* buffer = hb_buffer_create();
		hb_buffer_add_utf16(buffer, (const uint16_t*)text, len, 0, len);

		// Guess the script and direction
		hb_buffer_guess_segment_properties(buffer);

		// Get the direction
		hb_direction_t dir = hb_buffer_get_direction(buffer);

		hb_buffer_destroy(buffer);

		return dir == HB_DIRECTION_RTL;
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
}

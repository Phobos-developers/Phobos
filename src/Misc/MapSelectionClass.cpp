#include "MapSelectionClass.h"

#include <ScenarioClass.h>
#include <SideClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <Ext/Side/Body.h>
#include <Drawing.h>
#include <Surface.h>
#include <MouseClass.h>
#include <WWMouseClass.h>
#include <DisplayClass.h>
#include <VocClass.h>
#include <VoxClass.h>
#include <ThemeClass.h>
#include <PCX.h>
#include <FileSystem.h>
#include <BitFont.h>
#include <StringTable.h>
#include <CCINIClass.h>
#include <CCFileClass.h>
#include <MixFileClass.h>
#include <GScreenClass.h>

#include <Utilities/Constructs.h>
#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>
#include <Audio.h>
#include <Unsorted.h>

#include <algorithm>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using LoadFile_FastCall_t = void* (__fastcall*)(const char* pFileName, bool bLoadAsSHP);

static inline void* LoadGameFile(const char* pFileName, bool bLoadAsSHP = false)
{
	LoadFile_FastCall_t const pFunc = reinterpret_cast<LoadFile_FastCall_t>(0x5B40B0);
	return pFunc(pFileName, bLoadAsSHP);
}

static void StopMapSelAudio()
{
	PlaySoundA(NULL, NULL, 0);
	AudioStream* pStream = AudioStream::Instance;
	if (pStream)
	{
		pStream->PlayWAV("", false);
	}
}

static std::string_view Trim(std::string_view sv)
{
	size_t s = sv.find_first_not_of(" \t\r\n");

	if (s == std::string_view::npos)
		return { };

	size_t e = sv.find_last_not_of(" \t\r\n");

	return sv.substr(s, e - s + 1);
}

static void PlayMapSelAudio(const char* pAudioName)
{
	if (!pAudioName || !pAudioName[0])
		return;

	std::string soundName;
	float volumeFloat = 1.0f;
	std::string_view rawAudio { pAudioName };
	size_t commaPos = rawAudio.find(',');
	if (commaPos != std::string_view::npos)
	{
		int vol = atoi(std::string(Trim(rawAudio.substr(commaPos + 1))).c_str());
		if (vol > 0)
		{
			volumeFloat = std::clamp(vol, 1, 100) / 100.0f;
		}
		soundName = Trim(rawAudio.substr(0, commaPos));
	}
	else
	{
		soundName = Trim(rawAudio);
	}

	if (soundName.empty() || _stricmp(soundName.c_str(), "none") == 0 || _stricmp(soundName.c_str(), "no") == 0)
		return;

	StopMapSelAudio();

	// If it has a file extension (e.g. .wav), search and play as audio file without requiring soundmd.ini
	size_t dotPos = soundName.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		if (GetFileAttributesA(soundName.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			PlaySoundA(soundName.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);

			return;
		}

		static std::vector<BYTE> s_AudioMemBuf;
		CCFileClass testFile(soundName.c_str());
		if (testFile.Exists() && testFile.Open(FileAccessMode::Read))
		{
			int sz = testFile.GetFileSize();
			if (sz > 44)
			{
				s_AudioMemBuf.resize(sz);
				testFile.ReadBytes(s_AudioMemBuf.data(), sz);
				testFile.Close();
				PlaySoundA(reinterpret_cast<LPCSTR>(s_AudioMemBuf.data()), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);

				return;
			}
			testFile.Close();
		}
	}

	// If no extension or not found as direct file, look up in soundmd.ini via VocClass
	int vocIdx = VocClass::FindIndex(soundName.c_str());
	if (vocIdx >= 0)
	{
		VocClass::PlayGlobal(vocIdx, 0x2000, volumeFloat);

		return;
	}

	// Try appending .wav if sound name was provided without extension and wasn't in soundmd.ini
	std::string withWav = soundName + ".wav";
	if (GetFileAttributesA(withWav.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		PlaySoundA(withWav.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);

		return;
	}

	static std::vector<BYTE> s_AudioMemBuf2;
	CCFileClass testWav(withWav.c_str());
	if (testWav.Exists() && testWav.Open(FileAccessMode::Read))
	{
		int sz = testWav.GetFileSize();
		if (sz > 44)
		{
			s_AudioMemBuf2.resize(sz);
			testWav.ReadBytes(s_AudioMemBuf2.data(), sz);
			testWav.Close();
			PlaySoundA(reinterpret_cast<LPCSTR>(s_AudioMemBuf2.data()), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);

			return;
		}
		testWav.Close();
	}
}

static void PlayMapSelSFX(const char* pAudioName)
{
	if (!pAudioName || !pAudioName[0])
		return;

	std::string soundName;
	float volumeFloat = 1.0f;
	std::string_view rawAudio { pAudioName };
	size_t commaPos = rawAudio.find(',');
	if (commaPos != std::string_view::npos)
	{
		int vol = atoi(std::string(Trim(rawAudio.substr(commaPos + 1))).c_str());
		if (vol > 0)
		{
			volumeFloat = std::clamp(vol, 1, 100) / 100.0f;
		}
		soundName = Trim(rawAudio.substr(0, commaPos));
	}
	else
	{
		soundName = Trim(rawAudio);
	}

	if (soundName.empty() || _stricmp(soundName.c_str(), "none") == 0 || _stricmp(soundName.c_str(), "no") == 0)
		return;

	// For SFX: try VocClass (soundmd.ini) first so it doesn't interrupt VoiceOver
	std::string rawName = soundName;
	size_t dotPos = rawName.find_last_of('.');
	if (dotPos != std::string::npos && _stricmp(rawName.substr(dotPos).c_str(), ".wav") == 0)
		rawName = rawName.substr(0, dotPos);

	int vocIdx = VocClass::FindIndex(rawName.c_str());
	if (vocIdx < 0)
		vocIdx = VocClass::FindIndex(soundName.c_str());

	if (vocIdx >= 0)
	{
		VocClass::PlayGlobal(vocIdx, 0x2000, volumeFloat);

		return;
	}

	PlayMapSelAudio(pAudioName);
}

static int GetTypewriterVocIndex(const std::string& customTypeSound, float& outVolume)
{
	outVolume = 0.5f;
	if (customTypeSound.empty())
	{
		// Default to TextBleep (official RA2/YR typing sound)
		static int s_DefaultTypeVocIdx = -2;
		if (s_DefaultTypeVocIdx != -2)
			return s_DefaultTypeVocIdx;

		if (RulesClass::Instance && RulesClass::Instance->MessageCharTyped >= 0)
		{
			s_DefaultTypeVocIdx = RulesClass::Instance->MessageCharTyped;
			return s_DefaultTypeVocIdx;
		}

		const char* typingSoundNames[] = {
			"TextBleep", "MessageText", "KeyClick", "Typing", "Type", "TextType",
			"MessageCharTyped", "MessageClick", "ScoreScreenTick", "Button"
		};
		for (const char* name : typingSoundNames)
		{
			int idx = VocClass::FindIndex(name);
			if (idx >= 0)
			{
				s_DefaultTypeVocIdx = idx;
				Debug::Log("[MapSelection] Found typewriter sound '%s' (idx %d)\n", name, idx);
				return s_DefaultTypeVocIdx;
			}
		}
		s_DefaultTypeVocIdx = -1;
		return s_DefaultTypeVocIdx;
	}

	std::string soundName;
	std::string_view rawAudio { customTypeSound };
	size_t commaPos = rawAudio.find(',');
	if (commaPos != std::string_view::npos)
	{
		int vol = atoi(std::string(Trim(rawAudio.substr(commaPos + 1))).c_str());
		if (vol > 0)
		{
			outVolume = std::clamp(vol, 1, 100) / 100.0f;
		}
		soundName = Trim(rawAudio.substr(0, commaPos));
	}
	else
	{
		soundName = Trim(rawAudio);
	}

	if (soundName.empty() || _stricmp(soundName.c_str(), "none") == 0 || _stricmp(soundName.c_str(), "no") == 0)
		return -1;

	return VocClass::FindIndex(soundName.c_str());
}

static bool g_InMapSelectionBinkVideo = false;

DEFINE_HOOK(0x432D03, BinkMovie_CheckInput_MapSelection, 0x8)
{
	enum { SkipMovie = 0x432E27, ContinueMovie = 0x432D0B };

	if (g_InMapSelectionBinkVideo)
	{
		// Only skip movie if ESC key is physically pressed (ignore mouse clicks)
		if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
			return SkipMovie;
		else
			return ContinueMovie;
	}

	// Default game behavior
	if (R->EAX() == 0)
		return SkipMovie;

	return ContinueMovie;
}

static bool PlayMapSelBinkVideo(const char* pVideoName)
{
	if (!pVideoName || !pVideoName[0])
		return false;

	std::string movieName = pVideoName;
	size_t dotPos = movieName.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string ext = movieName.substr(dotPos);
		if (_stricmp(ext.c_str(), ".vqa") == 0 || _stricmp(ext.c_str(), ".bik") == 0)
		{
			movieName = movieName.substr(0, dotPos);
		}
	}

	std::string bikNameLower = movieName + ".bik";
	std::string bikNameUpper = movieName + ".BIK";

	CCFileClass testBikLower(bikNameLower.c_str());
	CCFileClass testBikUpper(bikNameUpper.c_str());

	if (testBikLower.Exists() || testBikUpper.Exists())
	{
		Debug::Log("[MapSelection] Playing Bink video '%s' (ESC to skip)\n", movieName.c_str());
		g_InMapSelectionBinkVideo = true;
		Game::PlayMovie(movieName.c_str(), -1, -1, -1, -1, -1);
		g_InMapSelectionBinkVideo = false;

		return true;
	}

	Debug::Log("[MapSelection] Bink video '%s' not found, skipping video playback.\n", bikNameLower.c_str());

	return false;
}

struct CaseInsensitiveLess
{
	bool operator()(const std::string& a, const std::string& b) const
	{
		return _stricmp(a.c_str(), b.c_str()) < 0;
	}
};

static std::map<std::string, std::wstring, CaseInsensitiveLess> g_ExtraCSFMap;
static bool g_ExtraCSFLoaded = false;

static void LoadExtraCSFFile(const char* pFileName)
{
	if (!pFileName || !pFileName[0])
		return;

	std::vector<BYTE> raw;
	CCFileClass file(pFileName);
	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		int sz = file.GetFileSize();
		if (sz >= 24)
		{
			raw.resize(sz);
			file.ReadBytes(raw.data(), sz);
			file.Close();
		}
	}
	if (raw.size() < 24)
		return;

	DWORD signature = *reinterpret_cast<const DWORD*>(&raw[0]);
	if (signature != 0x43534620) // " FSC"
		return;

	int numLabels = *reinterpret_cast<const int*>(&raw[8]);
	size_t pos = 24;

	for (int i = 0; i < numLabels && pos + 12 <= raw.size(); ++i)
	{
		DWORD lblSig = *reinterpret_cast<const DWORD*>(&raw[pos]);
		int numStr = *reinterpret_cast<const int*>(&raw[pos + 4]);
		int nameLen = *reinterpret_cast<const int*>(&raw[pos + 8]);
		pos += 12;

		if (pos + nameLen > raw.size())
			break;

		std::string labelName(reinterpret_cast<const char*>(&raw[pos]), nameLen);
		pos += nameLen;

		for (int s = 0; s < numStr && pos + 8 <= raw.size(); ++s)
		{
			DWORD strSig = *reinterpret_cast<const DWORD*>(&raw[pos]);
			int strLen = *reinterpret_cast<const int*>(&raw[pos + 4]);
			pos += 8;

			if (pos + strLen * 2 > raw.size())
				break;

			std::wstring value;
			value.reserve(strLen);
			for (int c = 0; c < strLen; ++c)
			{
				WORD ch = *reinterpret_cast<const WORD*>(&raw[pos + c * 2]);
				value.push_back(static_cast<wchar_t>(~ch));
			}
			pos += strLen * 2;

			if (strSig == 0x53545257) // "WRTS"
			{
				if (pos + 4 <= raw.size())
				{
					int extraLen = *reinterpret_cast<const int*>(&raw[pos]);
					pos += 4 + extraLen;
				}
			}

			if (s == 0)
			{
				g_ExtraCSFMap[labelName] = value;
			}
		}
	}
	Debug::Log("[MapSelection] Loaded %zu strings from extra CSF '%s'\n", g_ExtraCSFMap.size(), pFileName);
}

static void EnsureExtraCSFLoaded()
{
	if (g_ExtraCSFLoaded)
		return;
	g_ExtraCSFLoaded = true;

	LoadExtraCSFFile("stringtable00.csf");
	LoadExtraCSFFile("stringtable.csf");
	LoadExtraCSFFile("new_strings_v2_6_7.csf");
}

static std::vector<std::wstring> WordWrapText(const std::wstring& text, size_t maxCharsPerLine = 46)
{
	std::vector<std::wstring> lines;
	if (text.empty())
		return lines;

	std::wstring currentLine;
	std::wstring word;

	for (size_t i = 0; i <= text.length(); ++i)
	{
		wchar_t c = (i < text.length()) ? text[i] : L'\0';
		if (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r' || c == L'\0')
		{
			if (!word.empty())
			{
				if (currentLine.empty())
				{
					currentLine = word;
				}
				else if (currentLine.length() + 1 + word.length() <= maxCharsPerLine)
				{
					currentLine += L" " + word;
				}
				else
				{
					lines.push_back(currentLine);
					currentLine = word;
				}
				word.clear();
			}
			if (c == L'\n')
			{
				if (!currentLine.empty())
				{
					lines.push_back(currentLine);
					currentLine.clear();
				}
			}
		}
		else
		{
			word.push_back(c);
		}
	}
	if (!currentLine.empty())
	{
		lines.push_back(currentLine);
	}
	return lines;
}

static ColorStruct ConvertSchemeColorToRGB(const ColorStruct& hslColor)
{
	ColorStruct rgbColor { 255, 255, 255 };
	reinterpret_cast<ColorStruct*(__thiscall*)(const ColorStruct*, ColorStruct*)>(0x517440)(&hslColor, &rgbColor);

	return rgbColor;
}

static ColorStruct ParseColorString(const char* pStr, const ColorStruct& defaultColor)
{
	if (!pStr || !pStr[0])
		return defaultColor;

	std::string s { Trim(pStr) };

	if (s.empty())
		return defaultColor;

	int r = 0, g = 0, b = 0;
	if (sscanf_s(s.c_str(), "%d,%d,%d", &r, &g, &b) == 3)
	{
		return ColorStruct { static_cast<BYTE>(std::clamp(r, 0, 255)),
		                     static_cast<BYTE>(std::clamp(g, 0, 255)),
		                     static_cast<BYTE>(std::clamp(b, 0, 255)) };
	}

	int schemeIdx = ColorScheme::FindIndex(s.c_str());
	if (schemeIdx >= 0 && schemeIdx < ColorScheme::Array.Count)
	{
		ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(schemeIdx);
		if (pScheme)
		{
			return ConvertSchemeColorToRGB(pScheme->BaseColor);
		}
	}

	if (isdigit(static_cast<unsigned char>(s[0])))
	{
		int idx = atoi(s.c_str());
		if (idx >= 0 && idx < ColorScheme::Array.Count)
		{
			ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(idx);
			if (pScheme)
			{
				return ConvertSchemeColorToRGB(pScheme->BaseColor);
			}
		}
	}

	return defaultColor;
}

static std::wstring ResolveCSFOrText(const std::string& inputKey)
{
	if (inputKey.empty())
		return L"";

	std::string key { Trim(inputKey) };

	if (key.empty())
		return L"";

	EnsureExtraCSFLoaded();

	// Try directly as CSF label in main StringTable
	const wchar_t* pCsf = StringTable::TryFetchString(key.c_str(), nullptr);
	if (pCsf && pCsf[0])
		return pCsf;

	// Try in Extra CSF Map (stringtable00.csf, etc.)
	auto it = g_ExtraCSFMap.find(key);
	if (it != g_ExtraCSFMap.end())
		return it->second;

	// Try common prefixes: loadbrief:, briefvideo:, birefvideo:, brief:, name:, DESC:, TXT_
	const char* testPrefixes[] = {
		"loadbrief:", "LOADBRIEF:", "briefvideo:", "BRIEFVIDEO:", "birefvideo:", "BIREFVIDEO:",
		"Brief:", "brief:", "BRIEF:", "name:", "NAME:", "DESC:", "desc:", "TXT_"
	};
	for (const auto& pref : testPrefixes)
	{
		std::string alt = pref + key;
		pCsf = StringTable::TryFetchString(alt.c_str(), nullptr);
		if (pCsf && pCsf[0])
			return pCsf;

		it = g_ExtraCSFMap.find(alt);
		if (it != g_ExtraCSFMap.end())
			return it->second;
	}

	// If input is integer (e.g. 769), try TXT_<id>
	if (std::all_of(key.begin(), key.end(), ::isdigit))
	{
		std::string txtSym = "TXT_" + key;
		pCsf = StringTable::TryFetchString(txtSym.c_str(), nullptr);
		if (pCsf && pCsf[0])
			return pCsf;

		it = g_ExtraCSFMap.find(txtSym);
		if (it != g_ExtraCSFMap.end())
			return it->second;
	}

	// Return as wide string directly if not found in CSF
	return std::wstring(key.begin(), key.end());
}

MapSelectionClass::MapSelectionClass() = default;

MapSelectionClass::~MapSelectionClass()
{
	if (this->Palette)
	{
		delete this->Palette;
		this->Palette = nullptr;
	}
	if (this->OverlayPalette && this->OverlayPalette != this->Palette)
	{
		delete this->OverlayPalette;
		this->OverlayPalette = nullptr;
	}
	for (auto& anim : this->BackgroundAnims)
	{
		if (anim.Palette && anim.Palette != this->Palette && anim.Palette != this->OverlayPalette)
		{
			delete anim.Palette;
			anim.Palette = nullptr;
		}
	}
}

static SHPStruct* LoadMapSelSHP(const char* pFileName)
{
	if (!pFileName || !pFileName[0])
		return nullptr;

	SHPStruct* pSHP = static_cast<SHPStruct*>(LoadGameFile(pFileName, true));
	if (!pSHP)
	{
		CCFileClass file(pFileName);
		if (file.Exists())
		{
			pSHP = static_cast<SHPStruct*>(file.ReadWholeFile());
		}
	}

	Debug::Log("[MapSelection] LoadMapSelSHP '%s' -> %p\n", pFileName, pSHP);

	return pSHP;
}

static ConvertClass* LoadMapSelPalette(const char* pFileName, DSurface* pSurface)
{
	if (!pFileName || !pFileName[0])
		return nullptr;

	const ColorStruct* rawPal = static_cast<const ColorStruct*>(LoadGameFile(pFileName, false));
	std::vector<BYTE> localBuf;
	if (!rawPal)
	{
		CCFileClass file(pFileName);
		if (file.Exists() && file.Open(FileAccessMode::Read))
		{
			localBuf.resize(sizeof(ColorStruct) * 256);
			file.ReadBytes(localBuf.data(), sizeof(ColorStruct) * 256);
			file.Close();
			rawPal = reinterpret_cast<const ColorStruct*>(localBuf.data());
		}
	}

	if (!rawPal)
	{
		Debug::Log("[MapSelection] LoadMapSelPalette '%s' not found.\n", pFileName);
		return nullptr;
	}

	BytePalette colorData;
	for (int i = 0; i < 256; ++i)
	{
		colorData[i].R = static_cast<BYTE>(rawPal[i].R << 2);
		colorData[i].G = static_cast<BYTE>(rawPal[i].G << 2);
		colorData[i].B = static_cast<BYTE>(rawPal[i].B << 2);
	}

	ConvertClass* pConvert = GameCreate<ConvertClass>(colorData, FileSystem::TEMPERAT_PAL, pSurface, 1, false);
	Debug::Log("[MapSelection] LoadMapSelPalette '%s' created ConvertClass %p\n", pFileName, pConvert);

	return pConvert;
}

static bool LoadMapSelPCXData(const char* pFileName, MapSelectPCX& out)
{
	if (!pFileName || !pFileName[0])
		return false;

	std::vector<BYTE> raw;
	CCFileClass file(pFileName);
	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		int sz = file.GetFileSize();
		if (sz >= 128)
		{
			raw.resize(sz);
			file.ReadBytes(raw.data(), sz);
			file.Close();
		}
	}

	if (raw.size() < 128)
		return false;

	int xmin = raw[4] | (raw[5] << 8);
	int ymin = raw[6] | (raw[7] << 8);
	int xmax = raw[8] | (raw[9] << 8);
	int ymax = raw[10] | (raw[11] << 8);

	int width = xmax - xmin + 1;
	int height = ymax - ymin + 1;
	int bytesPerLine = raw[66] | (raw[67] << 8);

	if (width <= 0 || height <= 0 || bytesPerLine <= 0)
		return false;

	out.Width = width;
	out.Height = height;
	out.Pixels.resize(width * height, 0);

	size_t pos = 128;
	for (int y = 0; y < height; ++y)
	{
		int x = 0;
		while (x < bytesPerLine && pos < raw.size())
		{
			BYTE b = raw[pos++];
			int count = 1;
			BYTE val = b;
			if ((b & 0xC0) == 0xC0)
			{
				count = b & 0x3F;
				if (pos < raw.size())
					val = raw[pos++];
			}
			for (int c = 0; c < count && x < bytesPerLine; ++c, ++x)
			{
				if (x < width)
				{
					out.Pixels[y * width + x] = val;
				}
			}
		}
	}

	Debug::Log("[MapSelection] Decoded clickmap PCX '%s' (%dx%d, %zu pixels)\n", pFileName, width, height, out.Pixels.size());

	return true;
}

static bool MatchScenarioNames(const char* a, const char* b)
{
	if (!a || !b || !a[0] || !b[0])
		return false;

	if (_stricmp(a, b) == 0)
		return true;

	const char* baseA = strrchr(a, '/');
	if (!baseA) baseA = strrchr(a, '\\');
	baseA = baseA ? baseA + 1 : a;

	const char* baseB = strrchr(b, '/');
	if (!baseB) baseB = strrchr(b, '\\');
	baseB = baseB ? baseB + 1 : b;

	return _stricmp(baseA, baseB) == 0;
}

static bool DrawElement(
	DSurface* pSurface,
	const RectangleStruct& destRect,
	BSurface* pPCXSurface,
	SHPStruct* pSHP,
	ConvertClass* pPalette,
	int frameIndex = 0,
	int zAdjust = -2,
	BlitterFlags blitFlags = BlitterFlags::None)
{
	if (!pSurface || (!pPCXSurface && !pSHP))
		return false;

	if (pPCXSurface)
	{
		PCX::Instance.BlitToSurface(const_cast<RectangleStruct*>(&destRect), pSurface, pPCXSurface);
		return true;
	}
	else if (pSHP && pPalette)
	{
		Point2D drawPos = { destRect.X, destRect.Y };
		RectangleStruct bounds = pSurface->GetRect();
		CC_Draw_Shape(
			pSurface,
			pPalette,
			pSHP,
			frameIndex,
			&drawPos,
			&bounds,
			blitFlags,
			0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
		return true;
	}
	return false;
}

static void EnsureMixFileLoaded(const char* pMixName)
{
	if (!pMixName || !pMixName[0])
		return;

	for (int i = 0; i < MixFileClass::Array.Count; ++i)
	{
		MixFileClass* pMix = MixFileClass::Array.GetItemOrDefault(i);

		if (pMix && pMix->FileName && _stricmp(pMix->FileName, pMixName) == 0)
			return;
	}

	CCFileClass testFile(pMixName);

	if (testFile.Exists())
	{
		MixFileClass* pMix = GameCreate<MixFileClass>(pMixName);

		Debug::Log("[MapSelection] Mounted Mix '%s' (%p, CountFiles=%d)\n", pMixName, pMix, pMix ? pMix->CountFiles : 0);
	}
}

bool MapSelectionClass::OpenMapSelectionWindow(ScenarioClass* pScenario)
{
	if (!pScenario)
		return false;

	Debug::Log("[MapSelection] OpenMapSelectionWindow called for '%s'\n", pScenario->FileName);

	MapSelectionClass mapSelect;
	if (mapSelect.Initialize(pScenario))
	{
		return mapSelect.Run();
	}

	Debug::Log("[MapSelection] Initialize returned false, map selection aborted.\n");

	return false;
}

bool MapSelectionClass::Initialize(ScenarioClass* pScenario)
{
	if (!pScenario)
		return false;

	EnsureMixFileLoaded("mapsel.mix");
	EnsureMixFileLoaded("expandmd03.mix");

	this->CurrentScenario = pScenario;
	this->LoadConfig(pScenario);

	if (this->Choices.empty())
	{
		Debug::Log("[MapSelection] No choices configured for '%s', bypassing.\n", pScenario->FileName);
		return false;
	}

	this->LoadAssets();
	this->IsInitialized = true;

	return true;
}

void MapSelectionClass::LoadConfig(ScenarioClass* pScenario)
{
	CCINIClass ini;
	bool loaded = false;

	CCFileClass fileMD("mapselmd.ini");
	if (fileMD.Exists() && fileMD.Open(FileAccessMode::Read))
	{
		ini.ReadCCFile(&fileMD);
		loaded = true;
		Debug::Log("[MapSelection] Loaded mapselmd.ini successfully.\n");
	}

	if (!loaded)
	{
		Debug::Log("[MapSelection] Could not open mapselmd.ini\n");
		return;
	}

	// Active side section
	std::string sideSection = "GDI";
	int sideIdx = pScenario->PlayerSideIndex;
	if (sideIdx < 0 && HouseClass::CurrentPlayer)
		sideIdx = HouseClass::CurrentPlayer->SideIndex;

	if (sideIdx >= 0)
	{
		const SideClass* pSide = SideClass::Array.GetItemOrDefault(sideIdx);
		if (pSide && pSide->ID && pSide->ID[0])
		{
			if (ini.GetSection(pSide->ID))
				sideSection = pSide->ID;
		}

		if (ini.GetSection("Sides"))
		{
			char keyName[16];
			sprintf_s(keyName, "%d", sideIdx);
			char buf[64] = { 0 };
			ini.ReadString("Sides", keyName, "", buf, sizeof(buf));
			if (buf[0] && ini.GetSection(buf))
				sideSection = buf;
		}
	}

	Debug::Log("[MapSelection] Active side section determined as: '[%s]'\n", sideSection.c_str());

	// Find stage progression
	std::string targetStage { };
	char stageBuf[128] = { 0 };

	const char* curScen = pScenario->FileName;
	const char* nextScen = pScenario->NextScenario;

	Debug::Log("[MapSelection] Matching stage for curScen='%s', nextScen='%s'\n", curScen ? curScen : "", nextScen ? nextScen : "");

	if (nextScen && nextScen[0] && ini.GetSection(nextScen))
	{
		targetStage = nextScen;
	}
	else if (curScen && curScen[0] && ini.GetSection(curScen))
	{
		targetStage = curScen;
	}

	if (targetStage.empty())
	{
		int keyCount = ini.GetKeyCount(sideSection.c_str());
		for (int i = 1; i <= keyCount; ++i)
		{
			char keyName[32];
			sprintf_s(keyName, "%d", i);
			ini.ReadString(sideSection.c_str(), keyName, "", stageBuf, sizeof(stageBuf));
			if (!stageBuf[0])
				continue;

			char scenVal[260] = { 0 };
			ini.ReadString(stageBuf, "Scenario", "", scenVal, sizeof(scenVal));

			if (curScen && curScen[0] && MatchScenarioNames(scenVal, curScen))
			{
				targetStage = stageBuf;
				break;
			}
			else if (nextScen && nextScen[0] && MatchScenarioNames(scenVal, nextScen))
			{
				targetStage = stageBuf;
				break;
			}
			else if (nextScen && nextScen[0] && _stricmp(stageBuf, nextScen) == 0)
			{
				targetStage = stageBuf;
				break;
			}
		}
	}

	if (targetStage.empty())
	{
		int keyCount = ini.GetKeyCount(sideSection.c_str());
		if (keyCount > 0)
		{
			ini.ReadString(sideSection.c_str(), "1", "", stageBuf, sizeof(stageBuf));
			targetStage = stageBuf;
		}
	}

	this->CurrentStageName = targetStage;
	Debug::Log("[MapSelection] Selected active stage: '[%s]'\n", this->CurrentStageName.c_str());

	if (this->CurrentStageName.empty())
		return;

	const char* pStage = this->CurrentStageName.c_str();

	char clickMapBuf[128] = { 0 };
	ini.ReadString(pStage, "ClickMap", "", clickMapBuf, sizeof(clickMapBuf));
	this->ClickMapFileName = clickMapBuf;

	char mapVQBuf[128] = { 0 };
	ini.ReadString(pStage, "MapVQ", "", mapVQBuf, sizeof(mapVQBuf));
	this->MapVQFileName = mapVQBuf;

	char voiceOverBuf[128] = { 0 };
	ini.ReadString(pStage, "VoiceOver", "", voiceOverBuf, sizeof(voiceOverBuf));
	this->VoiceOverFileName = voiceOverBuf;

	char themeBuf[128] = { 0 };
	ini.ReadString(pStage, "Theme", "", themeBuf, sizeof(themeBuf));
	this->ThemeName = themeBuf;

	// Determine default text color from active side rules (MessageTextColor -> Color -> White fallback)
	ColorStruct sideTextColor { 255, 255, 255 }; // White fallback
	bool foundSideColor = false;

	SideClass* pSide = nullptr;
	if (sideIdx >= 0 && sideIdx < SideClass::Array.Count)
	{
		pSide = SideClass::Array.GetItemOrDefault(sideIdx);
	}
	if (!pSide)
	{
		pSide = SideClass::Find(sideSection.c_str());
	}
	if (!pSide)
	{
		for (int s = 0; s < SideClass::Array.Count; ++s)
		{
			SideClass* pCheck = SideClass::Array.GetItemOrDefault(s);
			if (!pCheck) continue;
			if ((pCheck->ID && _stricmp(pCheck->ID, sideSection.c_str()) == 0) ||
			    (pCheck->Name && _stricmp(pCheck->Name, sideSection.c_str()) == 0))
			{
				pSide = pCheck;
				break;
			}
		}
	}
	if (!pSide)
	{
		if (_stricmp(sideSection.c_str(), "Nod") == 0)
		{
			pSide = SideClass::Find("TSNodSide");
			if (!pSide) pSide = SideClass::Find("Nod");
			if (!pSide) pSide = SideClass::Find("Soviets");
			if (!pSide) pSide = SideClass::Find("Soviet");
		}
		else if (_stricmp(sideSection.c_str(), "GDI") == 0)
		{
			pSide = SideClass::Find("TSGDISide");
			if (!pSide) pSide = SideClass::Find("GDI");
			if (!pSide) pSide = SideClass::Find("Allies");
			if (!pSide) pSide = SideClass::Find("Allied");
		}
	}

	if (pSide)
	{
		// Check MessageTextColor from rulesmd.ini via SideExt
		SideExt* pSideExt = SideExt::TryFetch(pSide);
		if (pSideExt && pSideExt->MessageTextColor >= 0 && pSideExt->MessageTextColor < ColorScheme::Array.Count)
		{
			ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(pSideExt->MessageTextColor);
			if (pScheme)
			{
				sideTextColor = ConvertSchemeColorToRGB(pScheme->BaseColor);
				foundSideColor = true;
				Debug::Log("[MapSelection] Text color from SideExt MessageTextColor: RGB(%d,%d,%d) (Side: '%s')\n",
					sideTextColor.R, sideTextColor.G, sideTextColor.B, pSide->ID ? pSide->ID : "");
			}
		}

		// If MessageTextColor not set, check primary country Color= tag from rules
		if (!foundSideColor && pSide->HouseTypes.Count > 0)
		{
			for (int h = 0; h < pSide->HouseTypes.Count; ++h)
			{
				int houseIdx = pSide->HouseTypes.GetItem(h);
				HouseTypeClass* pHouseType = HouseTypeClass::Array.GetItemOrDefault(houseIdx);
				if (pHouseType && pHouseType->ColorSchemeIndex >= 0 && pHouseType->ColorSchemeIndex < ColorScheme::Array.Count)
				{
					ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(pHouseType->ColorSchemeIndex);
					if (pScheme)
					{
						sideTextColor = ConvertSchemeColorToRGB(pScheme->BaseColor);
						foundSideColor = true;
						Debug::Log("[MapSelection] Text color from HouseType '%s' ColorScheme: RGB(%d,%d,%d)\n",
							pHouseType->ID, sideTextColor.R, sideTextColor.G, sideTextColor.B);
						break;
					}
				}
			}
		}
	}

	// Check active player House ColorScheme if still not found
	if (!foundSideColor)
	{
		HouseClass* pHouse = HouseClass::CurrentPlayer;
		if (!pHouse && pScenario && pScenario->HumanPlayerHouseTypeIndex >= 0 && pScenario->HumanPlayerHouseTypeIndex < HouseClass::Array.Count)
		{
			pHouse = HouseClass::Array.GetItemOrDefault(pScenario->HumanPlayerHouseTypeIndex);
		}
		if (pHouse && pHouse->ColorSchemeIndex >= 0 && pHouse->ColorSchemeIndex < ColorScheme::Array.Count)
		{
			ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(pHouse->ColorSchemeIndex);
			if (pScheme)
			{
				sideTextColor = ConvertSchemeColorToRGB(pScheme->BaseColor);
				foundSideColor = true;
			}
		}
	}

	// Direct rules check for [TSNodSide] / [TSGDISide] / [Nod] / [GDI] if still not found
	if (!foundSideColor)
	{
		CCINIClass rulesIni;
		CCFileClass rulesFile("rulesmd.ini");
		if (rulesFile.Exists() && rulesFile.Open(FileAccessMode::Read))
		{
			rulesIni.ReadCCFile(&rulesFile);
			char msgColorBuf[64] = { 0 };
			const char* checkSections[] = {
				sideSection.c_str(),
				(_stricmp(sideSection.c_str(), "Nod") == 0) ? "TSNodSide" : "TSGDISide",
				(_stricmp(sideSection.c_str(), "Nod") == 0) ? "Nod" : "GDI",
				(_stricmp(sideSection.c_str(), "Nod") == 0) ? "Soviet" : "Allies",
				(_stricmp(sideSection.c_str(), "Nod") == 0) ? "Soviets" : "Allied"
			};
			for (const char* sec : checkSections)
			{
				rulesIni.ReadString(sec, "MessageTextColor", "", msgColorBuf, sizeof(msgColorBuf));
				if (!msgColorBuf[0])
					rulesIni.ReadString(sec, "Color", "", msgColorBuf, sizeof(msgColorBuf));

				if (msgColorBuf[0])
				{
					sideTextColor = ParseColorString(msgColorBuf, sideTextColor);
					foundSideColor = true;
					Debug::Log("[MapSelection] Text color resolved from rulesmd.ini [%s] '%s': RGB(%d,%d,%d)\n",
						sec, msgColorBuf, sideTextColor.R, sideTextColor.G, sideTextColor.B);
					break;
				}
			}
		}
	}

	char colorBuf[64] = { 0 };
	ini.ReadString(pStage, "TextColor", "", colorBuf, sizeof(colorBuf));
	this->DefaultTextColor = ParseColorString(colorBuf, sideTextColor);

	// Check Sounds section from stage, side, side-specific [Sounds<Side>], or global [Sounds]
	char soundsSecBuf[128] = { 0 };
	ini.ReadString(pStage, "Sounds", "", soundsSecBuf, sizeof(soundsSecBuf));
	if (!soundsSecBuf[0])
		ini.ReadString(sideSection.c_str(), "Sounds", "", soundsSecBuf, sizeof(soundsSecBuf));

	std::string resolvedSoundsSec;
	if (soundsSecBuf[0])
	{
		resolvedSoundsSec = soundsSecBuf;
	}
	else
	{
		std::string sideSoundsNamed = "Sounds" + sideSection;
		char sideSoundsIndexed[32] = { 0 };
		if (sideIdx >= 0)
			sprintf_s(sideSoundsIndexed, "Sounds%d", sideIdx);

		if (ini.GetSection(sideSoundsNamed.c_str()))
		{
			resolvedSoundsSec = sideSoundsNamed;
		}
		else if (sideSoundsIndexed[0] && ini.GetSection(sideSoundsIndexed))
		{
			resolvedSoundsSec = sideSoundsIndexed;
		}
		else if (ini.GetSection("Sounds"))
		{
			resolvedSoundsSec = "Sounds";
		}
	}
	const char* pSoundsSec = !resolvedSoundsSec.empty() ? resolvedSoundsSec.c_str() : nullptr;

	auto readSoundKey = [&](const char* key) -> std::string {
		char buf[128] = { 0 };
		ini.ReadString(pStage, key, "", buf, sizeof(buf));

		if (!buf[0] && pSoundsSec)
		{
			ini.ReadString(pSoundsSec, key, "", buf, sizeof(buf));
		}

		if (!buf[0] && ini.GetSection("Sounds"))
		{
			ini.ReadString("Sounds", key, "", buf, sizeof(buf));
		}
		return std::string(buf);
	};

	this->OverlaySound = readSoundKey("Overlay");
	this->TargetFlyInSound = readSoundKey("TargetFlyIn");
	this->EnterRegionSound = readSoundKey("EnterRegion");
	this->ExitRegionSound = readSoundKey("ExitRegion");
	this->ClickRegionSound = readSoundKey("ClickRegion");
	this->MouseOnMapSound = readSoundKey("MouseOnMapSound");
	this->MouseOffMapSound = readSoundKey("MouseOffMapSound");
	this->TypeSound = readSoundKey("TypeSound");

	char palBuf[128] = { 0 };
	ini.ReadString(pStage, "Palette", "mapsel.pal", palBuf, sizeof(palBuf));
	this->PaletteFileName = palBuf[0] ? palBuf : "mapsel.pal";

	char ovrPalBuf[128] = { 0 };
	ini.ReadString(pStage, "OverlayPalette", "msovrly.pal", ovrPalBuf, sizeof(ovrPalBuf));
	this->OverlayPaletteFileName = ovrPalBuf[0] ? ovrPalBuf : "msovrly.pal";

	char bgPCXBuf[128] = { 0 };
	ini.ReadString(pStage, "MapPCX", "", bgPCXBuf, sizeof(bgPCXBuf));
	this->BackgroundPCXFileName = bgPCXBuf;

	char bgSHPBuf[128] = { 0 };
	ini.ReadString(pStage, "Map", "", bgSHPBuf, sizeof(bgSHPBuf));
	if (!bgSHPBuf[0])
		ini.ReadString(pStage, "MapVQ", "", bgSHPBuf, sizeof(bgSHPBuf));
	this->BackgroundFileName = bgSHPBuf;

	char rectBuf[64] = { 0 };
	ini.ReadString(pStage, "TextRect", "", rectBuf, sizeof(rectBuf));
	if (rectBuf[0])
	{
		int rx = 0, ry = 0, rw = 0, rh = 0;
		if (sscanf_s(rectBuf, "%d,%d,%d,%d", &rx, &ry, &rw, &rh) == 4)
		{
			this->TextRectangle = { rx, ry, rw, rh };
		}
	}

	// Overlays (PCX takes precedence over SHP if provided)
	char overlaysPCXBuf[512] = { 0 };
	ini.ReadString(pStage, "OverlaysPCX", "", overlaysPCXBuf, sizeof(overlaysPCXBuf));
	if (overlaysPCXBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(overlaysPCXBuf, ",", &context);
		while (token)
		{
			while (*token == ' ')
				token++;
			if (*token)
			{
				this->OverlayPCXNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	char overlaysBuf[512] = { 0 };
	ini.ReadString(pStage, "Overlays", "", overlaysBuf, sizeof(overlaysBuf));
	if (overlaysBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(overlaysBuf, ",", &context);
		while (token)
		{
			while (*token == ' ')
				token++;
			if (*token)
			{
				this->OverlaySHPNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	// Target fly-in and target marker custom assets
	char flyInPCXBuf[512] = { 0 };
	ini.ReadString(pStage, "TargetFlyInAnimPCX", "", flyInPCXBuf, sizeof(flyInPCXBuf));
	if (!flyInPCXBuf[0])
		ini.ReadString(pStage, "TargetFlyInPCX", "", flyInPCXBuf, sizeof(flyInPCXBuf));
	if (flyInPCXBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(flyInPCXBuf, ",", &context);
		while (token)
		{
			while (*token == ' ')
				token++;
			if (*token)
			{
				this->TargetFlyInPCXNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	char flyInBuf[128] = { 0 };
	ini.ReadString(pStage, "TargetFlyInAnim", "", flyInBuf, sizeof(flyInBuf));
	this->TargetFlyInFileName = flyInBuf;

	char markerPCXBuf[512] = { 0 };
	ini.ReadString(pStage, "TargetMarkerAnimPCX", "", markerPCXBuf, sizeof(markerPCXBuf));
	if (!markerPCXBuf[0])
		ini.ReadString(pStage, "TargetMarkerPCX", "", markerPCXBuf, sizeof(markerPCXBuf));
	if (markerPCXBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(markerPCXBuf, ",", &context);
		while (token)
		{
			while (*token == ' ')
				token++;
			if (*token)
			{
				this->TargetMarkerPCXNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	char markerBuf[128] = { 0 };
	ini.ReadString(pStage, "TargetMarkerAnim", "", markerBuf, sizeof(markerBuf));
	if (!markerBuf[0])
		ini.ReadString(pStage, "TargetMarker", "", markerBuf, sizeof(markerBuf));
	this->TargetMarkerFileName = markerBuf;

	// Parse stage Targets list: Targets=<count>,<x1>,<y1>,<x2>,<y2>,...
	std::vector<Point2D> stageTargets;
	char stageTargetsBuf[256] = { 0 };
	ini.ReadString(pStage, "Targets", "", stageTargetsBuf, sizeof(stageTargetsBuf));
	if (stageTargetsBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(stageTargetsBuf, ",", &context);
		if (token)
		{
			int count = atoi(token);
			for (int t = 0; t < count; ++t)
			{
				char* tokX = strtok_s(nullptr, ",", &context);
				char* tokY = strtok_s(nullptr, ",", &context);
				if (tokX && tokY)
				{
					stageTargets.push_back({ atoi(tokX), atoi(tokY) });
				}
			}
		}
	}

	// Choices: iterate in strict numerical order (matching OpenTS mschoice.cpp)
	char scenBuf[260] = { 0 };
	ini.ReadString(pStage, "Scenario", "", scenBuf, sizeof(scenBuf));

	for (int choiceIndex = 1; choiceIndex < 256; ++choiceIndex)
	{
		char keyName[16];
		sprintf_s(keyName, "%d", choiceIndex);

		char choiceStage[128] = { 0 };
		ini.ReadString(pStage, keyName, "", choiceStage, sizeof(choiceStage));
		if (!choiceStage[0])
			continue;

		MapSelectChoice choice;
		choice.Index = choiceIndex;
		choice.StageName = choiceStage;

		// Map corresponding target coordinate from stage Targets list
		size_t choiceIdxInStage = this->Choices.size();
		if (choiceIdxInStage < stageTargets.size())
		{
			choice.TargetCoord = stageTargets[choiceIdxInStage];
			choice.HasTargetCoord = true;
		}

		char buf[512] = { 0 };
		if (ini.GetSection(choiceStage))
		{
			ini.ReadString(choiceStage, "Scenario", "", buf, sizeof(buf));
			choice.ScenarioPath = buf;

			ini.ReadString(choiceStage, "Description", "", buf, sizeof(buf));
			choice.Description = buf;

			ini.ReadString(choiceStage, "Summary", "", buf, sizeof(buf));
			choice.Summary = buf;

			ini.ReadString(choiceStage, "VoiceOver", "", buf, sizeof(buf));
			choice.VoiceOver = buf;

			ini.ReadString(choiceStage, "EnterRegion", "", buf, sizeof(buf));
			choice.HoverSound = buf[0] ? buf : this->EnterRegionSound;

			ini.ReadString(choiceStage, "ClickRegion", "", buf, sizeof(buf));
			choice.ClickSound = buf[0] ? buf : this->ClickRegionSound;

			ini.ReadString(choiceStage, "TextColor", "", buf, sizeof(buf));
			if (buf[0])
			{
				choice.TextColor = ParseColorString(buf, this->DefaultTextColor);
				choice.HasCustomTextColor = true;
			}
			else
			{
				choice.TextColor = this->DefaultTextColor;
			}

			// Only fallback to sub-stage Targets if the active stage had no target for this choice
			if (!choice.HasTargetCoord)
			{
				ini.ReadString(choiceStage, "Targets", "", buf, sizeof(buf));
				if (buf[0])
				{
					int count = 0, tx = 0, ty = 0;
					if (sscanf_s(buf, "%d,%d,%d", &count, &tx, &ty) >= 3)
					{
						choice.TargetCoord = { tx, ty };
						choice.HasTargetCoord = true;
					}
				}
			}
		}
		else
		{
			choice.ScenarioPath = choiceStage;
		}

		if (!choice.ScenarioPath.empty())
		{
			Debug::Log("[MapSelection] Added choice %d (index=%d): Stage='%s', Scenario='%s', Target=(%d,%d)\n",
				(int)this->Choices.size() + 1, choice.Index, choice.StageName.c_str(), choice.ScenarioPath.c_str(), choice.TargetCoord.X, choice.TargetCoord.Y);
			this->Choices.push_back(choice);
		}
	}

	if (this->Choices.empty() && scenBuf[0])
	{
		MapSelectChoice choice;
		choice.Index = 1;
		choice.StageName = this->CurrentStageName;
		choice.ScenarioPath = scenBuf;

		char buf[512] = { 0 };
		ini.ReadString(pStage, "Description", "", buf, sizeof(buf));
		choice.Description = buf;

		ini.ReadString(pStage, "Summary", "", buf, sizeof(buf));
		choice.Summary = buf;

		ini.ReadString(pStage, "VoiceOver", "", buf, sizeof(buf));
		choice.VoiceOver = buf;

		ini.ReadString(pStage, "EnterRegion", "", buf, sizeof(buf));
		choice.HoverSound = buf[0] ? buf : this->EnterRegionSound;

		ini.ReadString(pStage, "ClickRegion", "", buf, sizeof(buf));
		choice.ClickSound = buf[0] ? buf : this->ClickRegionSound;

		ini.ReadString(pStage, "TextColor", "", buf, sizeof(buf));
		if (buf[0])
		{
			choice.TextColor = ParseColorString(buf, this->DefaultTextColor);
			choice.HasCustomTextColor = true;
		}
		else
		{
			choice.TextColor = this->DefaultTextColor;
		}

		ini.ReadString(pStage, "Targets", "", buf, sizeof(buf));
		if (buf[0])
		{
			int count = 0, tx = 0, ty = 0;
			if (sscanf_s(buf, "%d,%d,%d", &count, &tx, &ty) >= 3)
			{
				choice.TargetCoord = { tx, ty };
				choice.HasTargetCoord = true;
			}
		}

		Debug::Log("[MapSelection] Added single default choice: Stage='%s', Scenario='%s'\n", choice.StageName.c_str(), choice.ScenarioPath.c_str());
		this->Choices.push_back(choice);
	}

	// Parse animations from stage section
	for (int i = 1; i <= 32; ++i)
	{
		char keyName[32];

		// Stage PCX animation (Anim<N>PCX)
		sprintf_s(keyName, "Anim%dPCX", i);
		char animPCXDef[512] = { 0 };
		ini.ReadString(pStage, keyName, "", animPCXDef, sizeof(animPCXDef));
		if (animPCXDef[0])
		{
			char pcxFile[128] = { 0 };
			int ax = 0, ay = 0, arate = 5;
			if (sscanf_s(animPCXDef, "%127[^,],%d,%d,%d", pcxFile, (unsigned)_countof(pcxFile), &ax, &ay, &arate) >= 3)
			{
				auto pFrames = GeneralUtils::GetAnimationPCX(pcxFile);
				if (pFrames && !pFrames->empty())
				{
					MapSelectAnim anim;
					anim.X = ax;
					anim.Y = ay;
					anim.FrameDelay = (arate > 0) ? arate : 5;
					for (const auto& frame : *pFrames)
					{
						if (frame.Exists())
							anim.PCXFrames.push_back(frame.GetSurface());
					}
					if (!anim.PCXFrames.empty())
					{
						anim.TotalFrames = static_cast<int>(anim.PCXFrames.size());
						anim.CurrentFrame = 0;
						anim.Timer.Start(anim.FrameDelay);
						this->BackgroundAnims.push_back(anim);
						continue;
					}
				}
			}
		}

		// Stage SHP animation (Anim<N>)
		sprintf_s(keyName, "Anim%d", i);
		char animSHPDef[256] = { 0 };
		ini.ReadString(pStage, keyName, "", animSHPDef, sizeof(animSHPDef));
		if (animSHPDef[0])
		{
			char shpName[64] = { 0 };
			char animPal[64] = { 0 };
			int ax = 0, ay = 0, arate = 5;
			int parsed = sscanf_s(animSHPDef, "%63[^,],%d,%d,%d,%63s", shpName, (unsigned)_countof(shpName), &ax, &ay, &arate, animPal, (unsigned)_countof(animPal));
			if (parsed >= 3)
			{
				SHPStruct* pSHP = LoadMapSelSHP(shpName);
				if (pSHP)
				{
					MapSelectAnim anim;
					anim.SHP = pSHP;
					anim.X = ax;
					anim.Y = ay;
					anim.FrameDelay = (arate > 0) ? arate : 5;
					anim.CurrentFrame = 0;
					anim.TotalFrames = pSHP->Frames;
					anim.Timer.Start(anim.FrameDelay);
					if (parsed >= 5 && animPal[0])
					{
						anim.PaletteName = animPal;
					}
					this->BackgroundAnims.push_back(anim);
				}
			}
		}
	}

	auto parsePCXAnimSection = [&](const char* pSecName)
	{
		if (!ini.GetSection(pSecName))
			return;

		for (int i = 1; i <= 32; ++i)
		{
			char keyName[16];
			sprintf_s(keyName, "%d", i);
			char animPCXDef[512] = { 0 };
			ini.ReadString(pSecName, keyName, "", animPCXDef, sizeof(animPCXDef));
			if (animPCXDef[0])
			{
				char pcxFile[128] = { 0 };
				int ax = 0, ay = 0, arate = 5;
				if (sscanf_s(animPCXDef, "%127[^,],%d,%d,%d", pcxFile, (unsigned)_countof(pcxFile), &ax, &ay, &arate) >= 3)
				{
					auto pFrames = GeneralUtils::GetAnimationPCX(pcxFile);
					if (pFrames && !pFrames->empty())
					{
						MapSelectAnim anim;
						anim.X = ax;
						anim.Y = ay;
						anim.FrameDelay = (arate > 0) ? arate : 5;
						for (const auto& frame : *pFrames)
						{
							if (frame.Exists())
								anim.PCXFrames.push_back(frame.GetSurface());
						}
						if (!anim.PCXFrames.empty())
						{
							anim.TotalFrames = static_cast<int>(anim.PCXFrames.size());
							anim.CurrentFrame = 0;
							anim.Timer.Start(anim.FrameDelay);
							this->BackgroundAnims.push_back(anim);
						}
					}
				}
			}
		}
	};

	auto parseSHPAnimSection = [&](const char* pSecName)
	{
		if (!ini.GetSection(pSecName))
			return;

		for (int i = 1; i <= 32; ++i)
		{
			char keyName[16];
			sprintf_s(keyName, "%d", i);
			char animSHPDef[256] = { 0 };
			ini.ReadString(pSecName, keyName, "", animSHPDef, sizeof(animSHPDef));
			if (animSHPDef[0])
			{
				char shpName[64] = { 0 };
				char animPal[64] = { 0 };
				int ax = 0, ay = 0, arate = 5;
				int parsed = sscanf_s(animSHPDef, "%63[^,],%d,%d,%d,%63s", shpName, (unsigned)_countof(shpName), &ax, &ay, &arate, animPal, (unsigned)_countof(animPal));
				if (parsed >= 3)
				{
					SHPStruct* pSHP = LoadMapSelSHP(shpName);
					if (pSHP)
					{
						MapSelectAnim anim;
						anim.SHP = pSHP;
						anim.X = ax;
						anim.Y = ay;
						anim.FrameDelay = (arate > 0) ? arate : 5;
						anim.CurrentFrame = 0;
						anim.TotalFrames = pSHP->Frames;
						anim.Timer.Start(anim.FrameDelay);
						if (parsed >= 5 && animPal[0])
						{
							anim.PaletteName = animPal;
						}
						this->BackgroundAnims.push_back(anim);
					}
				}
			}
		}
	};

	// 1. Global animations across all sides
	parsePCXAnimSection("AnimsPCX");
	parseSHPAnimSection("Anims");

	// 2. Side-specific global animations: [Anims<SideIndex>], [Anims<SideIndex>PCX], [Anims<SideName>], [Anims<SideName>PCX]
	if (sideIdx >= 0)
	{
		char sideSecBuf[64];
		sprintf_s(sideSecBuf, "Anims%dPCX", sideIdx);
		parsePCXAnimSection(sideSecBuf);

		sprintf_s(sideSecBuf, "Anims%d", sideIdx);
		parseSHPAnimSection(sideSecBuf);
	}
	if (!sideSection.empty())
	{
		std::string sideNameSecPCX = "Anims" + sideSection + "PCX";
		parsePCXAnimSection(sideNameSecPCX.c_str());

		std::string sideNameSecSHP = "Anims" + sideSection;
		parseSHPAnimSection(sideNameSecSHP.c_str());

		// Anims= and AnimsPCX= section pointers under [SOMESIDE]
		char animsSecBuf[128] = { 0 };
		ini.ReadString(sideSection.c_str(), "Anims", "", animsSecBuf, sizeof(animsSecBuf));
		if (animsSecBuf[0])
			parseSHPAnimSection(animsSecBuf);

		char animsPCXSecBuf[128] = { 0 };
		ini.ReadString(sideSection.c_str(), "AnimsPCX", "", animsPCXSecBuf, sizeof(animsPCXSecBuf));
		if (animsPCXSecBuf[0])
			parsePCXAnimSection(animsPCXSecBuf);

		// Direct Anim<N> / Anim<N>PCX defined directly inside [SOMESIDE]
		for (int i = 1; i <= 32; ++i)
		{
			char keyName[32];
			sprintf_s(keyName, "Anim%dPCX", i);
			char animPCXDef[512] = { 0 };
			ini.ReadString(sideSection.c_str(), keyName, "", animPCXDef, sizeof(animPCXDef));
			if (animPCXDef[0])
			{
				char pcxFile[128] = { 0 };
				int ax = 0, ay = 0, arate = 5;
				if (sscanf_s(animPCXDef, "%127[^,],%d,%d,%d", pcxFile, (unsigned)_countof(pcxFile), &ax, &ay, &arate) >= 3)
				{
					auto pFrames = GeneralUtils::GetAnimationPCX(pcxFile);
					if (pFrames && !pFrames->empty())
					{
						MapSelectAnim anim;
						anim.X = ax;
						anim.Y = ay;
						anim.FrameDelay = (arate > 0) ? arate : 5;
						for (const auto& frame : *pFrames)
						{
							if (frame.Exists())
								anim.PCXFrames.push_back(frame.GetSurface());
						}
						if (!anim.PCXFrames.empty())
						{
							anim.TotalFrames = static_cast<int>(anim.PCXFrames.size());
							anim.CurrentFrame = 0;
							anim.Timer.Start(anim.FrameDelay);
							this->BackgroundAnims.push_back(anim);
						}
					}
				}
			}

			sprintf_s(keyName, "Anim%d", i);
			char animSHPDef[256] = { 0 };
			ini.ReadString(sideSection.c_str(), keyName, "", animSHPDef, sizeof(animSHPDef));
			if (animSHPDef[0])
			{
				char shpName[64] = { 0 };
				char animPal[64] = { 0 };
				int ax = 0, ay = 0, arate = 5;
				int parsed = sscanf_s(animSHPDef, "%63[^,],%d,%d,%d,%63s", shpName, (unsigned)_countof(shpName), &ax, &ay, &arate, animPal, (unsigned)_countof(animPal));
				if (parsed >= 3)
				{
					SHPStruct* pSHP = LoadMapSelSHP(shpName);
					if (pSHP)
					{
						MapSelectAnim anim;
						anim.SHP = pSHP;
						anim.X = ax;
						anim.Y = ay;
						anim.FrameDelay = (arate > 0) ? arate : 5;
						anim.CurrentFrame = 0;
						anim.TotalFrames = pSHP->Frames;
						anim.Timer.Start(anim.FrameDelay);
						if (parsed >= 5 && animPal[0])
						{
							anim.PaletteName = animPal;
						}
						this->BackgroundAnims.push_back(anim);
					}
				}
			}
		}
	}
}

void MapSelectionClass::LoadAssets()
{
	DSurface* pSurface = DSurface::Hidden ? DSurface::Hidden : DSurface::Composite;

	// Palette for UI & Animations (default: MAPSEL.PAL)
	const char* palName = !this->PaletteFileName.empty() ? this->PaletteFileName.c_str() : "mapsel.pal";
	this->Palette = LoadMapSelPalette(palName, pSurface);

	// Palette for Overlays (default: MSOVRLY.PAL)
	const char* ovrPalName = !this->OverlayPaletteFileName.empty() ? this->OverlayPaletteFileName.c_str() : "msovrly.pal";
	this->OverlayPalette = LoadMapSelPalette(ovrPalName, pSurface);
	if (!this->OverlayPalette)
		this->OverlayPalette = this->Palette;

	// Background PCX (MapPCX takes preference over Map SHP)
	if (!this->BackgroundPCXFileName.empty())
	{
		PhobosPCXFile pcx(this->BackgroundPCXFileName.c_str());
		if (pcx.Exists())
			this->BackgroundPCX = pcx.GetSurface();
	}

	// Background SHP (Map)
	if (!this->BackgroundPCX && !this->BackgroundFileName.empty())
	{
		this->BackgroundSHP = LoadMapSelSHP(this->BackgroundFileName.c_str());
	}

	// Load custom palettes for animations if specified
	for (auto& anim : this->BackgroundAnims)
	{
		if (!anim.PaletteName.empty())
		{
			anim.Palette = LoadMapSelPalette(anim.PaletteName.c_str(), pSurface);
		}
	}

	// If no background was loaded, try deriving a map PCX from clickmap name
	if (!this->BackgroundPCX && !this->BackgroundSHP && !this->ClickMapFileName.empty())
	{
		std::string mapPCX = this->ClickMapFileName;
		std::transform(mapPCX.begin(), mapPCX.end(), mapPCX.begin(), ::toupper);

		// GDICLK01.PCX -> GDIMAP01.PCX, NODCLK01.PCX -> NODMAP01.PCX
		size_t clkPos = mapPCX.find("CLK");
		if (clkPos != std::string::npos)
		{
			mapPCX.replace(clkPos, 3, "MAP");
			Debug::Log("[MapSelection] Trying derived background PCX '%s'\n", mapPCX.c_str());
			PhobosPCXFile pcx(mapPCX.c_str());
			if (pcx.Exists())
				this->BackgroundPCX = pcx.GetSurface();
		}

		// Last resort: use the clickmap PCX itself as visual background
		if (!this->BackgroundPCX)
		{
			Debug::Log("[MapSelection] Using clickmap '%s' as visual background fallback\n", this->ClickMapFileName.c_str());
			PhobosPCXFile pcx(this->ClickMapFileName.c_str());
			if (pcx.Exists())
				this->BackgroundPCX = pcx.GetSurface();
		}
	}

	// ClickMap PCX (decoded for hit-testing)
	if (!this->ClickMapFileName.empty())
	{
		LoadMapSelPCXData(this->ClickMapFileName.c_str(), this->ClickMapData);
	}
	if (this->ClickMapData.Pixels.empty())
	{
		LoadMapSelPCXData("GDICLK01.PCX", this->ClickMapData);
	}

	// Overlays (PCX takes preference over SHP)
	if (!this->OverlayPCXNames.empty())
	{
		for (const auto& name : this->OverlayPCXNames)
		{
			PhobosPCXFile pcx(name.c_str());
			if (pcx.Exists())
			{
				this->OverlayPCXs.push_back(pcx.GetSurface());
			}
		}
	}
	if (this->OverlayPCXs.empty() && !this->OverlaySHPNames.empty())
	{
		for (const auto& name : this->OverlaySHPNames)
		{
			SHPStruct* pSHP = LoadMapSelSHP(name.c_str());
			if (pSHP)
			{
				this->OverlaySHPs.push_back(pSHP);
			}
		}
	}

	// Target Fly-In (PCX takes preference over SHP)
	if (!this->TargetFlyInPCXNames.empty())
	{
		for (const auto& name : this->TargetFlyInPCXNames)
		{
			auto pFrames = GeneralUtils::GetAnimationPCX(name);
			if (pFrames && !pFrames->empty())
			{
				for (const auto& frame : *pFrames)
				{
					if (frame.Exists())
						this->TargetFlyInPCXSurfaces.push_back(frame.GetSurface());
				}
			}
		}
	}
	if (this->TargetFlyInPCXSurfaces.empty())
	{
		const char* flyInName = !this->TargetFlyInFileName.empty() ? this->TargetFlyInFileName.c_str() : "TARGET1.SHP";
		this->TargetFlyInSHP = LoadMapSelSHP(flyInName);
	}

	// Target Marker (PCX takes preference over SHP)
	if (!this->TargetMarkerPCXNames.empty())
	{
		for (const auto& name : this->TargetMarkerPCXNames)
		{
			auto pFrames = GeneralUtils::GetAnimationPCX(name);
			if (pFrames && !pFrames->empty())
			{
				for (const auto& frame : *pFrames)
				{
					if (frame.Exists())
						this->TargetMarkerPCXSurfaces.push_back(frame.GetSurface());
				}
			}
		}
	}
	if (this->TargetMarkerPCXSurfaces.empty())
	{
		const char* markerName = !this->TargetMarkerFileName.empty() ? this->TargetMarkerFileName.c_str() : "TARGET2.SHP";
		this->TargetMarkerSHP = LoadMapSelSHP(markerName);
	}

	// Target crosshairs animation timer
	this->TargetAnimTimer.Start(4);
}

void MapSelectionClass::CalculateLayout(DSurface* pSurface)
{
	if (!pSurface)
		return;

	int bgWidth = 640;
	int bgHeight = 400;

	if (this->BackgroundPCX)
	{
		bgWidth = this->BackgroundPCX->Width;
		bgHeight = this->BackgroundPCX->Height;
	}
	else if (this->BackgroundSHP)
	{
		bgWidth = this->BackgroundSHP->Width;
		bgHeight = this->BackgroundSHP->Height;
	}
	else if (this->ClickMapData.Width > 0 && this->ClickMapData.Height > 0)
	{
		bgWidth = this->ClickMapData.Width;
		bgHeight = this->ClickMapData.Height;
	}

	int screenWidth = pSurface->Width;
	int screenHeight = pSurface->Height;

	this->WindowRectangle.X = (screenWidth - bgWidth) / 2;
	this->WindowRectangle.Y = (screenHeight - bgHeight) / 2;
	this->WindowRectangle.Width = bgWidth;
	this->WindowRectangle.Height = bgHeight;

	Debug::Log("[MapSelection] Layout: Screen=(%dx%d), WindowRect=(%d,%d,%d,%d), TextRect=(%d,%d,%d,%d)\n",
		screenWidth, screenHeight,
		this->WindowRectangle.X, this->WindowRectangle.Y, this->WindowRectangle.Width, this->WindowRectangle.Height,
		this->TextRectangle.X, this->TextRectangle.Y, this->TextRectangle.Width, this->TextRectangle.Height);
}

int MapSelectionClass::GetChoiceIndexAtPoint(int screenX, int screenY)
{
	int relX = screenX - this->WindowRectangle.X;
	int relY = screenY - this->WindowRectangle.Y;

	if (relX < 0 || relX >= this->WindowRectangle.Width || relY < 0 || relY >= this->WindowRectangle.Height)
		return -1;

	// Clickmap pixel color index (exact OpenTS behavior)
	if (!this->ClickMapData.Pixels.empty())
	{
		if (relX >= 0 && relX < this->ClickMapData.Width && relY >= 0 && relY < this->ClickMapData.Height)
		{
			BYTE pixelColor = this->ClickMapData.Pixels[relY * this->ClickMapData.Width + relX];
			// 0 and 255 are unselectable background/ocean pixels
			if (pixelColor > 0 && pixelColor < 255)
			{
				for (size_t i = 0; i < this->Choices.size(); ++i)
				{
					if (this->Choices[i].Index == static_cast<int>(pixelColor))
						return static_cast<int>(i);
				}
			}
		}
		// Never select anything outside the designated mission color regions
		return -1;
	}

	return -1;
}

void MapSelectionClass::UpdateAnimations()
{
	for (auto& anim : this->BackgroundAnims)
	{
		if (anim.Timer.Completed() && anim.TotalFrames > 1)
		{
			anim.CurrentFrame = (anim.CurrentFrame + 1) % anim.TotalFrames;
			anim.Timer.Start(anim.FrameDelay);
			this->RepaintAll = true;
		}
	}

	if (this->TargetAnimTimer.Completed())
	{
		int totalFrames = 0;
		if (!this->TargetMarkerPCXSurfaces.empty())
		{
			totalFrames = static_cast<int>(this->TargetMarkerPCXSurfaces.size());
		}
		else if (this->TargetMarkerSHP)
		{
			totalFrames = static_cast<int>(this->TargetMarkerSHP->Frames);
		}

		if (totalFrames > 1)
		{
			int half = totalFrames / 2;
			int activeCount = totalFrames - half;
			this->IdleTargetAnimFrame = (this->IdleTargetAnimFrame + 1) % half;
			this->ActiveTargetAnimFrame = half + ((this->ActiveTargetAnimFrame - half + 1) % activeCount);
		}
		else
		{
			this->IdleTargetAnimFrame = 0;
			this->ActiveTargetAnimFrame = 0;
		}
		this->TargetAnimTimer.Start(4);
		this->RepaintAll = true;
	}

	// Briefing typewriter text advance
	if (this->HoveredChoiceIdx >= 0 && this->TypewriterTimer.Completed())
	{
		size_t totalChars = 0;
		for (const auto& l : this->BriefingLines)
			totalChars += l.length();

		if (static_cast<size_t>(this->TypewriterCharCount) < totalChars)
		{
			this->TypewriterCharCount += 1;
			this->TypewriterTimer.Start(1);
			this->RepaintAll = true;

			// Play typing sound through game DirectSound mixer (never cuts off VoiceOver)
			if (this->TypeSoundTimer.Completed())
			{
				float typeVol = 0.5f;
				int typeIdx = GetTypewriterVocIndex(this->TypeSound, typeVol);
				if (typeIdx >= 0)
				{
					VocClass::PlayGlobal(typeIdx, 0x2000, typeVol);
				}
				else if (!this->TypeSound.empty() && _stricmp(this->TypeSound.c_str(), "none") != 0 && _stricmp(this->TypeSound.c_str(), "no") != 0)
				{
					PlayMapSelSFX(this->TypeSound.c_str());
				}
				this->TypeSoundTimer.Start(3);
			}
		}
	}
}

void MapSelectionClass::Render(DSurface* pSurface)
{
	if (!pSurface)
		return;

	// Fill background black for letterboxing
	pSurface->Fill(0);

	ConvertClass* pDrawer = this->Palette ? this->Palette : FileSystem::PALETTE_PAL;
	ConvertClass* pOverlayDrawer = this->OverlayPalette ? this->OverlayPalette : pDrawer;

	// Draw Background (PCX or SHP)
	DrawElement(pSurface, this->WindowRectangle, this->BackgroundPCX, this->BackgroundSHP, pDrawer, 0, -2);

	// Draw Overlays (World map / territory borders)
	if (!this->OverlayPCXs.empty())
	{
		for (BSurface* pPCX : this->OverlayPCXs)
		{
			if (pPCX)
			{
				DrawElement(pSurface, this->WindowRectangle, pPCX, nullptr, nullptr, 0, -2);
			}
		}
	}
	else
	{
		for (SHPStruct* pOverlaySHP : this->OverlaySHPs)
		{
			if (pOverlaySHP)
			{
				DrawElement(pSurface, this->WindowRectangle, nullptr, pOverlaySHP, pOverlayDrawer, 0, -2);
			}
		}
	}

	// Draw Animation SHPs or PCX sequences (Faction Logo, Rotating Globe, Compass)
	for (const auto& anim : this->BackgroundAnims)
	{
		if (!anim.PCXFrames.empty())
		{
			int frame = (anim.CurrentFrame >= 0 && anim.CurrentFrame < static_cast<int>(anim.PCXFrames.size())) ? anim.CurrentFrame : 0;
			BSurface* pFramePCX = anim.PCXFrames[frame];
			if (pFramePCX)
			{
				RectangleStruct animRect = {
					this->WindowRectangle.X + anim.X,
					this->WindowRectangle.Y + anim.Y,
					pFramePCX->Width,
					pFramePCX->Height
				};
				DrawElement(pSurface, animRect, pFramePCX, nullptr, nullptr, 0, -2);
			}
		}
		else if (anim.SHP)
		{
			RectangleStruct animRect = {
				this->WindowRectangle.X + anim.X,
				this->WindowRectangle.Y + anim.Y,
				anim.SHP->Width,
				anim.SHP->Height
			};
			ConvertClass* pAnimDrawer = anim.Palette ? anim.Palette : pDrawer;
			DrawElement(pSurface, animRect, nullptr, anim.SHP, pAnimDrawer, anim.CurrentFrame, -2);
		}
	}

	// Draw target markers / highlights for choices
	for (size_t i = 0; i < this->Choices.size(); ++i)
	{
		const auto& choice = this->Choices[i];
		if (choice.HasTargetCoord)
		{
			int drawX = this->WindowRectangle.X + choice.TargetCoord.X;
			int drawY = this->WindowRectangle.Y + choice.TargetCoord.Y;

			if (!this->TargetMarkerPCXSurfaces.empty())
			{
				int frame = (static_cast<int>(i) == this->HoveredChoiceIdx) ? this->ActiveTargetAnimFrame : this->IdleTargetAnimFrame;
				if (frame >= 0 && frame < static_cast<int>(this->TargetMarkerPCXSurfaces.size()))
				{
					BSurface* pPCX = this->TargetMarkerPCXSurfaces[frame];
					if (pPCX)
					{
						int halfW = pPCX->Width / 2;
						int halfH = pPCX->Height / 2;
						RectangleStruct markerRect = { drawX - halfW, drawY - halfH, pPCX->Width, pPCX->Height };
						DrawElement(pSurface, markerRect, pPCX, nullptr, nullptr, 0, -2);
					}
				}
			}
			else if (this->TargetMarkerSHP)
			{
				int halfW = this->TargetMarkerSHP->Width / 2;
				int halfH = this->TargetMarkerSHP->Height / 2;
				RectangleStruct markerRect = { drawX - halfW, drawY - halfH, this->TargetMarkerSHP->Width, this->TargetMarkerSHP->Height };
				int frame = (static_cast<int>(i) == this->HoveredChoiceIdx) ? this->ActiveTargetAnimFrame : this->IdleTargetAnimFrame;
				DrawElement(pSurface, markerRect, nullptr, this->TargetMarkerSHP, pDrawer, frame, -2);
			}
			else
			{
				COLORREF color = (static_cast<int>(i) == this->HoveredChoiceIdx)
					? Drawing::RGB_To_Int(255, 255, 80)
					: Drawing::RGB_To_Int(0, 255, 0);

				RectangleStruct markerRect = { drawX - 8, drawY - 8, 16, 16 };
				pSurface->DrawRect(&markerRect, color);
			}
		}
	}

	// Draw briefing text box
	this->DrawBriefing(pSurface);
}

void MapSelectionClass::DrawBriefing(DSurface* pSurface)
{
	if (!pSurface || this->HoveredChoiceIdx < 0 || this->BriefingLines.empty())
		return;

	const auto& choice = this->Choices[this->HoveredChoiceIdx];

	int drawX = this->WindowRectangle.X + this->TextRectangle.X;
	int drawY = this->WindowRectangle.Y + this->TextRectangle.Y;

	// Text rendering on the metallic HUD plate (TextRect: 92, 322, 332, 78)
	COLORREF textColor = Drawing::RGB_To_Int(choice.TextColor.R, choice.TextColor.G, choice.TextColor.B);
	COLORREF glowWhite = Drawing::RGB_To_Int(255, 255, 255);
	COLORREF glowFade = Drawing::RGB_To_Int(
		(static_cast<int>(choice.TextColor.R) + 255) / 2,
		(static_cast<int>(choice.TextColor.G) + 255) / 2,
		(static_cast<int>(choice.TextColor.B) + 255) / 2
	);
	TextPrintType style = (TextPrintType::FullShadow | TextPrintType::Point6Grad);

	size_t totalBriefingChars = 0;
	for (const auto& l : this->BriefingLines)
		totalBriefingChars += l.length();

	bool isTypingInProgress = static_cast<size_t>(this->TypewriterCharCount) < totalBriefingChars;

	int lineHeight = 13;
	int maxVisibleLines = (this->TextRectangle.Height > 0) ? (this->TextRectangle.Height / lineHeight) : 6;
	if (maxVisibleLines < 1)
		maxVisibleLines = 1;

	// Determine which line the typing cursor is currently on
	int activeLine = 0;
	int charAcc = 0;
	for (size_t l = 0; l < this->BriefingLines.size(); ++l)
	{
		charAcc += static_cast<int>(this->BriefingLines[l].length());
		activeLine = static_cast<int>(l);
		if (this->TypewriterCharCount <= charAcc)
			break;
	}

	// Auto-scroll / roll upward: old lines disappear as new lines appear below
	int startLine = (activeLine >= maxVisibleLines) ? (activeLine - maxVisibleLines + 1) : 0;
	int endLine = std::min(static_cast<int>(this->BriefingLines.size()), startLine + maxVisibleLines);

	int charsAccountedFor = 0;
	for (int l = 0; l < startLine; ++l)
		charsAccountedFor += static_cast<int>(this->BriefingLines[l].length());

	for (int l = startLine; l < endLine; ++l)
	{
		int lineLen = static_cast<int>(this->BriefingLines[l].length());
		int charsToDrawOnLine = std::clamp(this->TypewriterCharCount - charsAccountedFor, 0, lineLen);
		charsAccountedFor += lineLen;

		if (charsToDrawOnLine <= 0)
			continue;

		const std::wstring& line = this->BriefingLines[l];
		std::wstring visiblePart = line.substr(0, charsToDrawOnLine);
		bool isActiveTypingLine = (l == activeLine && isTypingInProgress);

		int screenRow = l - startLine;
		int lineY = drawY + screenRow * lineHeight;

		if (!isActiveTypingLine)
		{
			RectangleStruct lineBounds = {
				drawX,
				lineY,
				this->TextRectangle.Width,
				lineHeight + 4
			};
			Point2D relPoint = { 0, 0 };
			pSurface->DrawText(visiblePart.c_str(), &lineBounds, &relPoint, textColor, 0, style);
		}
		else
		{
			int len = static_cast<int>(visiblePart.length());
			if (len >= 3)
			{
				std::wstring settled = visiblePart.substr(0, len - 2);
				std::wstring fadeChar = visiblePart.substr(len - 2, 1);
				std::wstring headChar = visiblePart.substr(len - 1, 1);

				int wSettled = 0, hDummy = 0;
				if (BitFont::Instance)
					BitFont::Instance->GetTextDimension(settled.c_str(), &wSettled, &hDummy, 1000);

				int wFade = 0;
				if (BitFont::Instance)
				{
					std::wstring settledPlusFade = visiblePart.substr(0, len - 1);
					BitFont::Instance->GetTextDimension(settledPlusFade.c_str(), &wFade, &hDummy, 1000);
				}

				if (!settled.empty())
				{
					RectangleStruct r1 = { drawX, lineY, this->TextRectangle.Width, lineHeight + 4 };
					Point2D p1 = { 0, 0 };
					pSurface->DrawText(settled.c_str(), &r1, &p1, textColor, 0, style);
				}
				{
					RectangleStruct r2 = { drawX + wSettled, lineY, this->TextRectangle.Width - wSettled, lineHeight + 4 };
					Point2D p2 = { 0, 0 };
					pSurface->DrawText(fadeChar.c_str(), &r2, &p2, glowFade, 0, style);
				}
				{
					RectangleStruct r3 = { drawX + wFade, lineY, this->TextRectangle.Width - wFade, lineHeight + 4 };
					Point2D p3 = { 0, 0 };
					pSurface->DrawText(headChar.c_str(), &r3, &p3, glowWhite, 0, style);
				}
			}
			else if (len == 2)
			{
				std::wstring fadeChar = visiblePart.substr(0, 1);
				std::wstring headChar = visiblePart.substr(1, 1);

				int wFade = 0, hDummy = 0;
				if (BitFont::Instance)
					BitFont::Instance->GetTextDimension(fadeChar.c_str(), &wFade, &hDummy, 1000);

				RectangleStruct r1 = { drawX, lineY, this->TextRectangle.Width, lineHeight + 4 };
				Point2D p1 = { 0, 0 };
				pSurface->DrawText(fadeChar.c_str(), &r1, &p1, glowFade, 0, style);

				RectangleStruct r2 = { drawX + wFade, lineY, this->TextRectangle.Width - wFade, lineHeight + 4 };
				Point2D p2 = { 0, 0 };
				pSurface->DrawText(headChar.c_str(), &r2, &p2, glowWhite, 0, style);
			}
			else if (len == 1)
			{
				RectangleStruct r = { drawX, lineY, this->TextRectangle.Width, lineHeight + 4 };
				Point2D p = { 0, 0 };
				pSurface->DrawText(visiblePart.c_str(), &r, &p, glowWhite, 0, style);
			}
		}
	}
}

void MapSelectionClass::PlayIntroSequence(DSurface* pSurface)
{
	if (!pSurface)
		return;

	ConvertClass* pDrawer = this->Palette ? this->Palette : FileSystem::PALETTE_PAL;
	ConvertClass* pOverlayDrawer = this->OverlayPalette ? this->OverlayPalette : pDrawer;

	if (WWMouseClass::Instance)
		WWMouseClass::Instance->HideCursor();

	while (ShowCursor(FALSE) >= 0);

	::SetCursor(NULL);

	auto pumpAndCheckSkip = [&]() -> bool
	{
		::SetCursor(NULL);
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return true;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Game::CallBack();
		HWND hGameWnd = Game::hWnd;
		HWND hActiveWnd = GetForegroundWindow();
		bool isWindowActive = (hGameWnd && (hActiveWnd == hGameWnd || IsChild(hGameWnd, hActiveWnd)));
		if (isWindowActive && ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_ESCAPE) & 0x8000)))
		{
			return true;
		}
		return false;
	};

	auto renderBase = [&](int completedOverlays, int completedTargets)
	{
		pSurface->Fill(0);

		// Background
		DrawElement(pSurface, this->WindowRectangle, this->BackgroundPCX, this->BackgroundSHP, pDrawer, 0, -2);

		// Completed Overlays (drawn solid)
		if (!this->OverlayPCXs.empty())
		{
			for (int o = 0; o < completedOverlays && o < static_cast<int>(this->OverlayPCXs.size()); ++o)
			{
				if (this->OverlayPCXs[o])
				{
					DrawElement(pSurface, this->WindowRectangle, this->OverlayPCXs[o], nullptr, nullptr, 0, -2);
				}
			}
		}
		else
		{
			for (int o = 0; o < completedOverlays && o < static_cast<int>(this->OverlaySHPs.size()); ++o)
			{
				if (this->OverlaySHPs[o])
				{
					DrawElement(pSurface, this->WindowRectangle, nullptr, this->OverlaySHPs[o], pOverlayDrawer, 0, -2);
				}
			}
		}

		// Background Anims
		for (const auto& anim : this->BackgroundAnims)
		{
			if (!anim.PCXFrames.empty())
			{
				int frame = (anim.CurrentFrame >= 0 && anim.CurrentFrame < static_cast<int>(anim.PCXFrames.size())) ? anim.CurrentFrame : 0;
				BSurface* pFramePCX = anim.PCXFrames[frame];
				if (pFramePCX)
				{
					RectangleStruct animRect = {
						this->WindowRectangle.X + anim.X,
						this->WindowRectangle.Y + anim.Y,
						pFramePCX->Width,
						pFramePCX->Height
					};
					DrawElement(pSurface, animRect, pFramePCX, nullptr, nullptr, 0, -2);
				}
			}
			else if (anim.SHP)
			{
				RectangleStruct animRect = {
					this->WindowRectangle.X + anim.X,
					this->WindowRectangle.Y + anim.Y,
					anim.SHP->Width,
					anim.SHP->Height
				};
				ConvertClass* pAnimDrawer = anim.Palette ? anim.Palette : pDrawer;
				DrawElement(pSurface, animRect, nullptr, anim.SHP, pAnimDrawer, anim.CurrentFrame, -2);
			}
		}

		// Completed Targets
		for (int t = 0; t < completedTargets && t < static_cast<int>(this->Choices.size()); ++t)
		{
			const auto& choice = this->Choices[t];
			if (choice.HasTargetCoord)
			{
				int drawX = this->WindowRectangle.X + choice.TargetCoord.X;
				int drawY = this->WindowRectangle.Y + choice.TargetCoord.Y;

				if (!this->TargetMarkerPCXSurfaces.empty())
				{
					if (this->IdleTargetAnimFrame >= 0 && this->IdleTargetAnimFrame < static_cast<int>(this->TargetMarkerPCXSurfaces.size()))
					{
						BSurface* pPCX = this->TargetMarkerPCXSurfaces[this->IdleTargetAnimFrame];
						if (pPCX)
						{
							int halfW = pPCX->Width / 2;
							int halfH = pPCX->Height / 2;
							RectangleStruct markerRect = { drawX - halfW, drawY - halfH, pPCX->Width, pPCX->Height };
							DrawElement(pSurface, markerRect, pPCX, nullptr, nullptr, 0, -2);
						}
					}
				}
				else if (this->TargetMarkerSHP)
				{
					int halfW = this->TargetMarkerSHP->Width / 2;
					int halfH = this->TargetMarkerSHP->Height / 2;
					RectangleStruct markerRect = { drawX - halfW, drawY - halfH, this->TargetMarkerSHP->Width, this->TargetMarkerSHP->Height };
					DrawElement(pSurface, markerRect, nullptr, this->TargetMarkerSHP, pDrawer, this->IdleTargetAnimFrame, -2);
				}
			}
		}
	};

	const BlitterFlags fadeFlags[] = {
		BlitterFlags::TransLucent75,
		BlitterFlags::TransLucent50,
		BlitterFlags::TransLucent25,
		BlitterFlags::None
	};

	size_t totalOverlays = !this->OverlayPCXs.empty() ? this->OverlayPCXs.size() : this->OverlaySHPs.size();

	// Animate Overlays fade-in
	for (size_t o = 0; o < totalOverlays; ++o)
	{
		if (!this->OverlaySound.empty())
		{
			PlayMapSelSFX(this->OverlaySound.c_str());
		}

		for (int stage = 0; stage < 4; ++stage)
		{
			if (pumpAndCheckSkip())
				return;

			this->UpdateAnimations();
			renderBase(static_cast<int>(o), 0);

			if (!this->OverlayPCXs.empty())
			{
				if (this->OverlayPCXs[o])
				{
					DrawElement(pSurface, this->WindowRectangle, this->OverlayPCXs[o], nullptr, nullptr, 0, -2);
				}
			}
			else if (o < this->OverlaySHPs.size() && this->OverlaySHPs[o])
			{
				DrawElement(pSurface, this->WindowRectangle, nullptr, this->OverlaySHPs[o], pOverlayDrawer, 0, -2, fadeFlags[stage]);
			}

			GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
			Sleep(50);
		}

		// Small delay between overlays
		for (int d = 0; d < 5; ++d)
		{
			if (pumpAndCheckSkip())
				return;
			this->UpdateAnimations();
			renderBase(static_cast<int>(o + 1), 0);
			GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
			Sleep(40);
		}
	}

	// Animate Target Fly-Ins
	for (size_t t = 0; t < this->Choices.size(); ++t)
	{
		const auto& choice = this->Choices[t];
		if (!choice.HasTargetCoord)
			continue;

		if (!this->TargetFlyInSound.empty())
		{
			PlayMapSelSFX(this->TargetFlyInSound.c_str());
		}

		int drawX = this->WindowRectangle.X + choice.TargetCoord.X;
		int drawY = this->WindowRectangle.Y + choice.TargetCoord.Y;

		if (!this->TargetFlyInPCXSurfaces.empty())
		{
			int maxFrames = static_cast<int>(this->TargetFlyInPCXSurfaces.size());
			for (int f = 0; f < maxFrames; ++f)
			{
				if (pumpAndCheckSkip())
					return;

				this->UpdateAnimations();
				renderBase(static_cast<int>(totalOverlays), static_cast<int>(t));

				BSurface* pFlyInPCX = this->TargetFlyInPCXSurfaces[f];
				if (pFlyInPCX)
				{
					int halfW = pFlyInPCX->Width / 2;
					int halfH = pFlyInPCX->Height / 2;
					RectangleStruct flyInRect = { drawX - halfW, drawY - halfH, pFlyInPCX->Width, pFlyInPCX->Height };
					DrawElement(pSurface, flyInRect, pFlyInPCX, nullptr, nullptr, 0, -2);
				}

				GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
				Sleep(50);
			}
		}
		else if (this->TargetFlyInSHP && this->TargetFlyInSHP->Frames > 0)
		{
			int halfW = this->TargetFlyInSHP->Width / 2;
			int halfH = this->TargetFlyInSHP->Height / 2;
			RectangleStruct flyInRect = { drawX - halfW, drawY - halfH, this->TargetFlyInSHP->Width, this->TargetFlyInSHP->Height };

			int maxFrames = this->TargetFlyInSHP->Frames;
			for (int f = 0; f < maxFrames; ++f)
			{
				if (pumpAndCheckSkip())
					return;

				this->UpdateAnimations();
				renderBase(static_cast<int>(totalOverlays), static_cast<int>(t));

				// Draw fly-in frame
				BlitterFlags flag = (f < 4) ? fadeFlags[f] : BlitterFlags::None;
				DrawElement(pSurface, flyInRect, nullptr, this->TargetFlyInSHP, pDrawer, f, -2, flag);

				GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
				Sleep(50);
			}
		}
	}
}

bool MapSelectionClass::Run()
{
	DSurface* pSurface = DSurface::Hidden;
	if (!pSurface)
		pSurface = DSurface::Composite;
	if (!pSurface)
		pSurface = DSurface::Primary;
	if (!pSurface)
		return false;

	// Hide mouse cursor during video playback, overlays sweep, and target fly-ins (matching OpenTS Hide_Mouse())
	if (WWMouseClass::Instance)
		WWMouseClass::Instance->HideCursor();

	while (ShowCursor(FALSE) >= 0);

	::SetCursor(NULL);

	// Play Bink video if MapVQ is specified and exists
	if (!this->MapVQFileName.empty())
	{
		PlayMapSelBinkVideo(this->MapVQFileName.c_str());

		// Drain any leftover input (e.g. ESC or mouse clicks used to skip video)
		MSG drainMsg;
		while (PeekMessage(&drainMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&drainMsg);
			DispatchMessage(&drainMsg);
		}
		while ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000))
		{
			Sleep(10);
		}
	}

	pSurface->Fill(0);
	this->CalculateLayout(pSurface);

	// Play animated intro sequence (Overlays sweep fade-in and TARGET1 fly-ins)
	this->PlayIntroSequence(pSurface);

	// Play Map Selection Theme Song (only if Theme= is explicitly specified)
	if (!this->ThemeName.empty())
	{
		int themeIdx = ThemeClass::Instance.FindIndex(this->ThemeName.c_str());
		if (themeIdx >= 0)
		{
			ThemeClass::Instance.Play(themeIdx);
		}
	}

	// Drain any input after intro sequence
	{
		MSG drainMsg;
		while (PeekMessage(&drainMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&drainMsg);
			DispatchMessage(&drainMsg);
		}
		while ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_ESCAPE) & 0x8000))
		{
			Sleep(10);
		}
	}

	// Ensure any audio is stopped before interactive loop starts
	StopMapSelAudio();

	// Intro presentation is finished and all target markers are placed on map; now show definitive cursor
	while (ShowCursor(FALSE) >= 0);
	::SetCursor(NULL);

	if (DisplayClass::Instance.CurrentSWTypeIndex != -1)
		DisplayClass::Instance.CurrentSWTypeIndex = -1;

	if (Unsorted::CurrentSWType != -1)
		Unsorted::CurrentSWType = -1;

	if (WWMouseClass::Instance)
	{
		WWMouseClass::Instance->HideCursor();
		WWMouseClass::Instance->ShowCursor();
		WWMouseClass::Instance->CaptureMouse();
		WWMouseClass::Instance->RefCount = 0;
	}

	MouseClass::Instance.SetCursor(MouseCursorType::Default, false);
	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);

	bool wasLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool wasMouseOnMap = false;

	this->HoveredChoiceIdx = -1;
	this->LastHoveredChoiceIdx = -1;
	this->SelectedChoiceIdx = -1;

	Debug::Log("[MapSelection] Entering modal Run loop (choices count: %zu)\n", this->Choices.size());

	while (this->SelectedChoiceIdx < 0)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Game::CallBack();
		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);

		Point2D mousePos = { 0, 0 };
		if (WWMouseClass::Instance)
		{
			mousePos.X = WWMouseClass::Instance->GetX();
			mousePos.Y = WWMouseClass::Instance->GetY();
		}

		bool isMouseOnMap = (mousePos.X >= this->WindowRectangle.X &&
		                     mousePos.X < (this->WindowRectangle.X + this->WindowRectangle.Width) &&
		                     mousePos.Y >= this->WindowRectangle.Y &&
		                     mousePos.Y < (this->WindowRectangle.Y + this->WindowRectangle.Height));

		if (isMouseOnMap != wasMouseOnMap)
		{
			if (isMouseOnMap && !this->MouseOnMapSound.empty())
			{
				PlayMapSelSFX(this->MouseOnMapSound.c_str());
			}
			else if (!isMouseOnMap && !this->MouseOffMapSound.empty())
			{
				PlayMapSelSFX(this->MouseOffMapSound.c_str());
			}
			wasMouseOnMap = isMouseOnMap;
		}

		this->HoveredChoiceIdx = isMouseOnMap ? this->GetChoiceIndexAtPoint(mousePos.X, mousePos.Y) : -1;

		if (this->HoveredChoiceIdx != this->LastHoveredChoiceIdx)
		{
			// Stop previous VoiceOver immediately if moving off or switching regions
			StopMapSelAudio();

			this->TypewriterCharCount = 0;
			this->TypewriterTimer.Start(1);
			this->TypeSoundTimer.Start(3);

			// Always restart active target marker animation from its initial frame upon hover enter
			int totalMarkerFrames = !this->TargetMarkerPCXSurfaces.empty()
				? static_cast<int>(this->TargetMarkerPCXSurfaces.size())
				: (this->TargetMarkerSHP ? static_cast<int>(this->TargetMarkerSHP->Frames) : 0);
			this->ActiveTargetAnimFrame = totalMarkerFrames > 1 ? (totalMarkerFrames / 2) : 0;
			this->TargetAnimTimer.Start(4);

			if (this->HoveredChoiceIdx >= 0 && this->HoveredChoiceIdx < static_cast<int>(this->Choices.size()))
			{
				const auto& choice = this->Choices[this->HoveredChoiceIdx];
				std::wstring fullText = ResolveCSFOrText(choice.Description);
				if (fullText.empty())
					fullText = ResolveCSFOrText(choice.Summary);
				if (fullText.empty())
					fullText = std::wstring(choice.StageName.begin(), choice.StageName.end());

				size_t charsPerLine = (this->TextRectangle.Width > 0) ? std::max<size_t>(20, static_cast<size_t>(this->TextRectangle.Width / 7)) : 46;
				this->BriefingLines = WordWrapText(fullText, charsPerLine);

				// Play hover / enter region SFX sound first
				if (!choice.HoverSound.empty())
				{
					PlayMapSelSFX(choice.HoverSound.c_str());
				}
				else if (!this->EnterRegionSound.empty())
				{
					PlayMapSelSFX(this->EnterRegionSound.c_str());
				}

				// Play VoiceOver speech
				if (!choice.VoiceOver.empty())
				{
					PlayMapSelAudio(choice.VoiceOver.c_str());
				}
			}
			else
			{
				this->BriefingLines.clear();
				if (this->LastHoveredChoiceIdx >= 0 && !this->ExitRegionSound.empty())
				{
					PlayMapSelSFX(this->ExitRegionSound.c_str());
				}
			}

			this->LastHoveredChoiceIdx = this->HoveredChoiceIdx;
			this->RepaintAll = true;
		}

		bool isLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		if (isLButtonDown && !wasLButtonDown)
		{
			// Only select if the mouse is hovering over a valid choice on the map
			if (this->HoveredChoiceIdx >= 0 && this->HoveredChoiceIdx < static_cast<int>(this->Choices.size()))
			{
				int clickedChoice = this->GetChoiceIndexAtPoint(mousePos.X, mousePos.Y);
				if (clickedChoice == this->HoveredChoiceIdx)
				{
					this->SelectedChoiceIdx = clickedChoice;
					Debug::Log("[MapSelection] Region clicked on hovered choice %d ('%s')\n",
						this->SelectedChoiceIdx, this->Choices[this->SelectedChoiceIdx].StageName.c_str());

					// Immediately cut off VoiceOver speech
					StopMapSelAudio();

					const auto& chosen = this->Choices[this->SelectedChoiceIdx];
					if (!chosen.ClickSound.empty())
					{
						PlayMapSelSFX(chosen.ClickSound.c_str());
					}
					else if (!this->ClickRegionSound.empty())
					{
						PlayMapSelSFX(this->ClickRegionSound.c_str());
					}
					break;
				}
			}
		}
		wasLButtonDown = isLButtonDown;

		this->UpdateAnimations();

		this->Render(pSurface);

		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
		GScreenClass::Instance.DoBlit(true, pSurface, nullptr);

		Sleep(1);
	}

	// Ensure any playing VoiceOver audio and custom theme music are stopped when loading begins
	StopMapSelAudio();
	if (!this->ThemeName.empty())
	{
		ThemeClass::Instance.Stop(true);
	}

	// Hide mouse cursor when leaving map selection (matching OpenTS Hide_Mouse())
	if (WWMouseClass::Instance)
		WWMouseClass::Instance->HideCursor();

	while (ShowCursor(FALSE) >= 0);

	::SetCursor(NULL);

	if (this->SelectedChoiceIdx >= 0 && this->SelectedChoiceIdx < static_cast<int>(this->Choices.size()))
	{
		const auto& choice = this->Choices[this->SelectedChoiceIdx];
		Debug::Log("[MapSelection] Final selected mission: '%s' (Scenario: '%s')\n", choice.StageName.c_str(), choice.ScenarioPath.c_str());

		if (this->CurrentScenario && !choice.ScenarioPath.empty())
		{
			this->CurrentScenario->Stage = 0;
			strncpy(this->CurrentScenario->FileName, choice.ScenarioPath.c_str(), 0x104u);
			this->CurrentScenario->FileName[259] = '\0';
			strncpy(this->CurrentScenario->NextScenario, choice.ScenarioPath.c_str(), 0x104u);
			this->CurrentScenario->NextScenario[259] = '\0';
		}

		return true;
	}

	return false;
}

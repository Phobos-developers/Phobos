#include "MapSelectionClass.h"

#include <ScenarioClass.h>
#include <SideClass.h>
#include <HouseClass.h>
#include <Drawing.h>
#include <Surface.h>
#include <MouseClass.h>
#include <WWMouseClass.h>
#include <VocClass.h>
#include <VoxClass.h>
#include <ThemeClass.h>
#include <PCX.h>
#include <FileSystem.h>
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
	auto const pFunc = reinterpret_cast<LoadFile_FastCall_t>(0x5B40B0);
	return pFunc(pFileName, bLoadAsSHP);
}

static void PlayMapSelAudio(const char* pAudioName)
{
	if (!pAudioName || !pAudioName[0])
		return;

	std::string baseName = pAudioName;
	// Strip volume percentage if present (e.g. "GSWEEP.AUD,60" -> "GSWEEP.AUD")
	size_t commaPos = baseName.find(',');
	if (commaPos != std::string::npos)
	{
		baseName = baseName.substr(0, commaPos);
	}
	while (!baseName.empty() && (baseName.back() == ' ' || baseName.back() == '\t' || baseName.back() == '\r' || baseName.back() == '\n'))
		baseName.pop_back();
	while (!baseName.empty() && (baseName.front() == ' ' || baseName.front() == '\t'))
		baseName.erase(baseName.begin());

	if (baseName.empty())
		return;

	// Strip any legacy extension (.aud, .wav)
	std::string rawName = baseName;
	size_t dotPos = rawName.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string ext = rawName.substr(dotPos);
		if (_stricmp(ext.c_str(), ".aud") == 0 || _stricmp(ext.c_str(), ".wav") == 0)
		{
			rawName = rawName.substr(0, dotPos);
		}
	}

	std::string wavNameLower = rawName + ".wav";
	std::string wavNameUpper = rawName + ".WAV";

	// Try Windows PlaySound on disk file first (100% reliable, async, non-blocking)
	if (GetFileAttributesA(wavNameLower.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		Debug::Log("[MapSelection] Playing WAV via PlaySound '%s'\n", wavNameLower.c_str());
		PlaySoundA(wavNameLower.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		return;
	}
	if (GetFileAttributesA(wavNameUpper.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		Debug::Log("[MapSelection] Playing WAV via PlaySound '%s'\n", wavNameUpper.c_str());
		PlaySoundA(wavNameUpper.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		return;
	}

	// Try playing .wav via AudioStream (from MIX files or memory)
	AudioStream* pStream = AudioStream::Instance;
	if (pStream)
	{
		CCFileClass testFileLower(wavNameLower.c_str());
		CCFileClass testFileUpper(wavNameUpper.c_str());

		if (testFileLower.Exists())
		{
			Debug::Log("[MapSelection] Playing WAV via AudioStream '%s'\n", wavNameLower.c_str());
			typedef bool(__thiscall* PlayWAV_t)(void* pThis, const char* pFileName, bool bLoop);
			auto playWAV = reinterpret_cast<PlayWAV_t>(0x407B60);
			playWAV(pStream, wavNameLower.c_str(), false);
			return;
		}
		else if (testFileUpper.Exists())
		{
			Debug::Log("[MapSelection] Playing WAV via AudioStream '%s'\n", wavNameUpper.c_str());
			typedef bool(__thiscall* PlayWAV_t)(void* pThis, const char* pFileName, bool bLoop);
			auto playWAV = reinterpret_cast<PlayWAV_t>(0x407B60);
			playWAV(pStream, wavNameUpper.c_str(), false);
			return;
		}
	}

	// Fallback to VocClass (Sound.ini sound entry)
	int vocIdx = VocClass::FindIndex(rawName.c_str());
	if (vocIdx < 0)
		vocIdx = VocClass::FindIndex(baseName.c_str());

	if (vocIdx >= 0)
	{
		Debug::Log("[MapSelection] Playing Voc sound '%s' (idx %d)\n", rawName.c_str(), vocIdx);
		VocClass::PlayGlobal(vocIdx, 0x2000, 1.0f);
		return;
	}

	// Fallback to VoxClass (EVA messages in eva.ini)
	int voxIdx = VoxClass::FindIndex(rawName.c_str());
	if (voxIdx < 0)
		voxIdx = VoxClass::FindIndex(baseName.c_str());

	if (voxIdx >= 0)
	{
		Debug::Log("[MapSelection] Playing Vox EVA sound '%s' (idx %d)\n", rawName.c_str(), voxIdx);
		VoxClass::PlayIndex(voxIdx, -1, -1);
		return;
	}

	Debug::Log("[MapSelection] Audio '%s' / '%s.wav' not found anywhere.\n", baseName.c_str(), rawName.c_str());
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
		Debug::Log("[MapSelection] Playing Bink video '%s'\n", movieName.c_str());
		Game::PlayMovie(movieName.c_str(), -1, -1, -1, -1, -1);
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

static ColorStruct ParseColorString(const char* pStr, const ColorStruct& defaultColor)
{
	if (!pStr || !pStr[0])
		return defaultColor;

	int r = 0, g = 0, b = 0;
	if (sscanf_s(pStr, "%d,%d,%d", &r, &g, &b) == 3)
	{
		return ColorStruct { static_cast<BYTE>(std::clamp(r, 0, 255)),
		                     static_cast<BYTE>(std::clamp(g, 0, 255)),
		                     static_cast<BYTE>(std::clamp(b, 0, 255)) };
	}
	return defaultColor;
}

static std::wstring ResolveCSFOrText(const std::string& inputKey)
{
	if (inputKey.empty())
		return L"";

	std::string key = inputKey;
	while (!key.empty() && (key.back() == ' ' || key.back() == '\t' || key.back() == '\r' || key.back() == '\n'))
		key.pop_back();
	while (!key.empty() && (key.front() == ' ' || key.front() == '\t'))
		key.erase(key.begin());

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
	if (pPalette)
	{
		delete pPalette;
		pPalette = nullptr;
	}
	if (pOverlayPalette)
	{
		delete pOverlayPalette;
		pOverlayPalette = nullptr;
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

	for (GenericNode* pNode = (GenericNode*)MixFileClass::MIXes.First(); pNode && pNode->IsValid(); pNode = pNode->Next())
	{
		auto pMix = reinterpret_cast<MixFileClass*>(pNode);
		if (pMix->FileName && _stricmp(pMix->FileName, pMixName) == 0)
		{
			return;
		}
	}

	CCFileClass testFile(pMixName);
	if (testFile.Exists())
	{
		auto pMix = GameCreate<MixFileClass>(pMixName);
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

	this->pCurrentScenario = pScenario;
	this->LoadConfig(pScenario);

	if (this->choices.empty())
	{
		Debug::Log("[MapSelection] No choices configured for '%s', bypassing.\n", pScenario->FileName);
		return false;
	}

	this->LoadAssets();
	this->isInitialized = true;

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

	CCINIClass battleIni;
	CCFileClass battleFile("battlemd.ini");
	if (battleFile.Exists() && battleFile.Open(FileAccessMode::Read))
	{
		battleIni.ReadCCFile(&battleFile);
	}

	CCINIClass missionIni;
	CCFileClass missionFile("missionmd.ini");
	if (missionFile.Exists() && missionFile.Open(FileAccessMode::Read))
	{
		missionIni.ReadCCFile(&missionFile);
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
	std::string targetStage {};
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

	this->currentStageName = targetStage;
	Debug::Log("[MapSelection] Selected active stage: '[%s]'\n", this->currentStageName.c_str());

	if (this->currentStageName.empty())
		return;

	const char* pStage = this->currentStageName.c_str();

	char clickMapBuf[128] = { 0 };
	ini.ReadString(pStage, "ClickMap", "", clickMapBuf, sizeof(clickMapBuf));
	this->clickMapFileName = clickMapBuf;

	char mapVQBuf[128] = { 0 };
	ini.ReadString(pStage, "MapVQ", "", mapVQBuf, sizeof(mapVQBuf));
	this->mapVQFileName = mapVQBuf;

	char voiceOverBuf[128] = { 0 };
	ini.ReadString(pStage, "VoiceOver", "", voiceOverBuf, sizeof(voiceOverBuf));
	this->voiceOverFileName = voiceOverBuf;

	char themeBuf[128] = { 0 };
	ini.ReadString(pStage, "Theme", "", themeBuf, sizeof(themeBuf));
	this->themeName = themeBuf;

	// Determine default text color from player's house
	ColorStruct playerHouseColor { 255, 239, 99 };
	HouseClass* pHouse = HouseClass::CurrentPlayer;
	if (!pHouse && pScenario && pScenario->HumanPlayerHouseTypeIndex >= 0 && pScenario->HumanPlayerHouseTypeIndex < HouseClass::Array.Count)
	{
		pHouse = HouseClass::Array.GetItemOrDefault(pScenario->HumanPlayerHouseTypeIndex);
	}
	if (pHouse)
	{
		int schemeIdx = pHouse->ColorSchemeIndex;
		ColorScheme* pScheme = ColorScheme::Array.GetItemOrDefault(schemeIdx);
		if (pScheme)
		{
			playerHouseColor = pScheme->BaseColor;
		}
	}
	else if (_stricmp(sideSection.c_str(), "Nod") == 0)
	{
		playerHouseColor = ColorStruct { 255, 50, 50 };
	}

	char colorBuf[64] = { 0 };
	ini.ReadString(pStage, "TextColor", "", colorBuf, sizeof(colorBuf));
	this->defaultTextColor = ParseColorString(colorBuf, playerHouseColor);

	char sfxBuf[128] = { 0 };
	ini.ReadString(pStage, "OverlaySound", "", sfxBuf, sizeof(sfxBuf));
	this->overlaySound = sfxBuf;

	ini.ReadString(pStage, "TargetSound", "", sfxBuf, sizeof(sfxBuf));
	this->targetFlyInSound = sfxBuf;

	ini.ReadString(pStage, "EnterRegionSound", "", sfxBuf, sizeof(sfxBuf));
	this->enterRegionSound = sfxBuf;

	ini.ReadString(pStage, "ExitRegionSound", "", sfxBuf, sizeof(sfxBuf));
	this->exitRegionSound = sfxBuf;

	ini.ReadString(pStage, "ClickSound", "", sfxBuf, sizeof(sfxBuf));
	this->clickRegionSound = sfxBuf;

	ini.ReadString(pStage, "MouseOnMapSound", "", sfxBuf, sizeof(sfxBuf));
	this->mouseOnMapSound = sfxBuf;

	ini.ReadString(pStage, "MouseOffMapSound", "", sfxBuf, sizeof(sfxBuf));
	this->mouseOffMapSound = sfxBuf;

	char palBuf[128] = { 0 };
	ini.ReadString(pStage, "Palette", "mapsel.pal", palBuf, sizeof(palBuf));
	this->paletteFileName = palBuf[0] ? palBuf : "mapsel.pal";

	char ovrPalBuf[128] = { 0 };
	ini.ReadString(pStage, "OverlayPalette", "msovrly.pal", ovrPalBuf, sizeof(ovrPalBuf));
	this->overlayPaletteFileName = ovrPalBuf[0] ? ovrPalBuf : "msovrly.pal";

	char bgPCXBuf[128] = { 0 };
	ini.ReadString(pStage, "MapPCX", "", bgPCXBuf, sizeof(bgPCXBuf));
	this->backgroundPCXFileName = bgPCXBuf;

	char bgSHPBuf[128] = { 0 };
	ini.ReadString(pStage, "Map", "", bgSHPBuf, sizeof(bgSHPBuf));
	if (!bgSHPBuf[0])
		ini.ReadString(pStage, "MapVQ", "", bgSHPBuf, sizeof(bgSHPBuf));
	this->backgroundFileName = bgSHPBuf;

	char rectBuf[64] = { 0 };
	ini.ReadString(pStage, "TextRect", "", rectBuf, sizeof(rectBuf));
	if (rectBuf[0])
	{
		int rx = 0, ry = 0, rw = 0, rh = 0;
		if (sscanf_s(rectBuf, "%d,%d,%d,%d", &rx, &ry, &rw, &rh) == 4)
		{
			this->textRectangle = { rx, ry, rw, rh };
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
			while (*token == ' ') token++;
			if (*token)
			{
				this->overlayPCXNames.push_back(token);
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
			while (*token == ' ') token++;
			if (*token)
			{
				this->overlaySHPNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	// Target fly-in and target marker custom assets
	char flyInPCXBuf[512] = { 0 };
	ini.ReadString(pStage, "TargetFlyInPCX", "", flyInPCXBuf, sizeof(flyInPCXBuf));
	if (flyInPCXBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(flyInPCXBuf, ",", &context);
		while (token)
		{
			while (*token == ' ') token++;
			if (*token)
			{
				this->targetFlyInPCXNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	char flyInBuf[128] = { 0 };
	ini.ReadString(pStage, "TargetFlyIn", "", flyInBuf, sizeof(flyInBuf));
	this->targetFlyInFileName = flyInBuf;

	char markerPCXBuf[512] = { 0 };
	ini.ReadString(pStage, "TargetMarkerPCX", "", markerPCXBuf, sizeof(markerPCXBuf));
	if (markerPCXBuf[0])
	{
		char* context = nullptr;
		char* token = strtok_s(markerPCXBuf, ",", &context);
		while (token)
		{
			while (*token == ' ') token++;
			if (*token)
			{
				this->targetMarkerPCXNames.push_back(token);
			}
			token = strtok_s(nullptr, ",", &context);
		}
	}

	char markerBuf[128] = { 0 };
	ini.ReadString(pStage, "TargetMarker", "", markerBuf, sizeof(markerBuf));
	this->targetMarkerFileName = markerBuf;

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
		size_t choiceIdxInStage = this->choices.size();
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

			ini.ReadString(choiceStage, "HoverSound", "", buf, sizeof(buf));
			choice.HoverSound = buf;

			ini.ReadString(choiceStage, "ClickSound", "", buf, sizeof(buf));
			if (buf[0]) choice.ClickSound = buf;
			else choice.ClickSound = this->clickRegionSound;

			ini.ReadString(choiceStage, "TextColor", "", buf, sizeof(buf));
			if (buf[0])
			{
				choice.TextColor = ParseColorString(buf, this->defaultTextColor);
				choice.HasCustomTextColor = true;
			}
			else
			{
				choice.TextColor = this->defaultTextColor;
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

		if (choice.Description.empty() && battleIni.GetSection(choiceStage))
		{
			battleIni.ReadString(choiceStage, "Description", "", buf, sizeof(buf));
			choice.Description = buf;
			battleIni.ReadString(choiceStage, "Summary", "", buf, sizeof(buf));
			choice.Summary = buf;
		}

		if (choice.Description.empty() && missionIni.GetSection(choice.ScenarioPath.c_str()))
		{
			missionIni.ReadString(choice.ScenarioPath.c_str(), "UIName", "", buf, sizeof(buf));
			choice.Description = buf;
			missionIni.ReadString(choice.ScenarioPath.c_str(), "Briefing", "", buf, sizeof(buf));
			choice.Summary = buf;
		}

		if (!choice.ScenarioPath.empty())
		{
			Debug::Log("[MapSelection] Added choice %d (index=%d): Stage='%s', Scenario='%s', Target=(%d,%d)\n",
				(int)this->choices.size() + 1, choice.Index, choice.StageName.c_str(), choice.ScenarioPath.c_str(), choice.TargetCoord.X, choice.TargetCoord.Y);
			this->choices.push_back(choice);
		}
	}

	if (this->choices.empty() && scenBuf[0])
	{
		MapSelectChoice choice;
		choice.Index = 1;
		choice.StageName = this->currentStageName;
		choice.ScenarioPath = scenBuf;

		char buf[512] = { 0 };
		ini.ReadString(pStage, "Description", "", buf, sizeof(buf));
		choice.Description = buf;

		ini.ReadString(pStage, "Summary", "", buf, sizeof(buf));
		choice.Summary = buf;

		ini.ReadString(pStage, "VoiceOver", "", buf, sizeof(buf));
		choice.VoiceOver = buf;

		ini.ReadString(pStage, "HoverSound", "", buf, sizeof(buf));
		choice.HoverSound = buf;

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
		this->choices.push_back(choice);
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
						this->backgroundAnims.push_back(anim);
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
					this->backgroundAnims.push_back(anim);
				}
			}
		}
	}

	// Global PCX animations from [AnimsPCX]
	if (ini.GetSection("AnimsPCX"))
	{
		for (int i = 1; i <= 32; ++i)
		{
			char keyName[16];
			sprintf_s(keyName, "%d", i);
			char animPCXDef[512] = { 0 };
			ini.ReadString("AnimsPCX", keyName, "", animPCXDef, sizeof(animPCXDef));
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
							this->backgroundAnims.push_back(anim);
						}
					}
				}
			}
		}
	}

	// Global SHP animations from [Anims]
	if (ini.GetSection("Anims"))
	{
		for (int i = 1; i <= 32; ++i)
		{
			char keyName[16];
			sprintf_s(keyName, "%d", i);
			char animSHPDef[256] = { 0 };
			ini.ReadString("Anims", keyName, "", animSHPDef, sizeof(animSHPDef));
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
						this->backgroundAnims.push_back(anim);
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
	const char* palName = !this->paletteFileName.empty() ? this->paletteFileName.c_str() : "mapsel.pal";
	this->pPalette = LoadMapSelPalette(palName, pSurface);

	// Palette for Overlays (default: MSOVRLY.PAL)
	const char* ovrPalName = !this->overlayPaletteFileName.empty() ? this->overlayPaletteFileName.c_str() : "msovrly.pal";
	this->pOverlayPalette = LoadMapSelPalette(ovrPalName, pSurface);
	if (!this->pOverlayPalette)
		this->pOverlayPalette = this->pPalette;

	// Background PCX (MapPCX takes preference over Map SHP)
	if (!this->backgroundPCXFileName.empty())
	{
		PhobosPCXFile pcx(this->backgroundPCXFileName.c_str());
		if (pcx.Exists())
			this->pBackgroundPCX = pcx.GetSurface();
	}

	// Background SHP (Map)
	if (!this->pBackgroundPCX && !this->backgroundFileName.empty())
	{
		this->pBackgroundSHP = LoadMapSelSHP(this->backgroundFileName.c_str());
	}

	// Load custom palettes for animations if specified
	for (auto& anim : this->backgroundAnims)
	{
		if (!anim.PaletteName.empty())
		{
			anim.Palette = LoadMapSelPalette(anim.PaletteName.c_str(), pSurface);
		}
	}

	// If no background was loaded, try deriving a map PCX from clickmap name
	if (!this->pBackgroundPCX && !this->pBackgroundSHP && !this->clickMapFileName.empty())
	{
		std::string mapPCX = this->clickMapFileName;
		std::transform(mapPCX.begin(), mapPCX.end(), mapPCX.begin(), ::toupper);

		// GDICLK01.PCX -> GDIMAP01.PCX, NODCLK01.PCX -> NODMAP01.PCX
		size_t clkPos = mapPCX.find("CLK");
		if (clkPos != std::string::npos)
		{
			mapPCX.replace(clkPos, 3, "MAP");
			Debug::Log("[MapSelection] Trying derived background PCX '%s'\n", mapPCX.c_str());
			PhobosPCXFile pcx(mapPCX.c_str());
			if (pcx.Exists())
				this->pBackgroundPCX = pcx.GetSurface();
		}

		// Last resort: use the clickmap PCX itself as visual background
		if (!this->pBackgroundPCX)
		{
			Debug::Log("[MapSelection] Using clickmap '%s' as visual background fallback\n", this->clickMapFileName.c_str());
			PhobosPCXFile pcx(this->clickMapFileName.c_str());
			if (pcx.Exists())
				this->pBackgroundPCX = pcx.GetSurface();
		}
	}

	// ClickMap PCX (decoded for hit-testing)
	if (!this->clickMapFileName.empty())
	{
		LoadMapSelPCXData(this->clickMapFileName.c_str(), this->clickMapData);
	}
	if (this->clickMapData.Pixels.empty())
	{
		LoadMapSelPCXData("GDICLK01.PCX", this->clickMapData);
	}

	// Overlays (PCX takes preference over SHP)
	if (!this->overlayPCXNames.empty())
	{
		for (const auto& name : this->overlayPCXNames)
		{
			PhobosPCXFile pcx(name.c_str());
			if (pcx.Exists())
			{
				this->overlayPCXs.push_back(pcx.GetSurface());
			}
		}
	}
	if (this->overlayPCXs.empty() && !this->overlaySHPNames.empty())
	{
		for (const auto& name : this->overlaySHPNames)
		{
			SHPStruct* pSHP = LoadMapSelSHP(name.c_str());
			if (pSHP)
			{
				this->overlaySHPs.push_back(pSHP);
			}
		}
	}

	// Target Fly-In (PCX takes preference over SHP)
	if (!this->targetFlyInPCXNames.empty())
	{
		for (const auto& name : this->targetFlyInPCXNames)
		{
			auto pFrames = GeneralUtils::GetAnimationPCX(name);
			if (pFrames && !pFrames->empty())
			{
				for (const auto& frame : *pFrames)
				{
					if (frame.Exists())
						this->targetFlyInPCXSurfaces.push_back(frame.GetSurface());
				}
			}
		}
	}
	if (this->targetFlyInPCXSurfaces.empty())
	{
		const char* flyInName = !this->targetFlyInFileName.empty() ? this->targetFlyInFileName.c_str() : "TARGET1.SHP";
		this->pTargetFlyInSHP = LoadMapSelSHP(flyInName);
	}

	// Target Marker (PCX takes preference over SHP)
	if (!this->targetMarkerPCXNames.empty())
	{
		for (const auto& name : this->targetMarkerPCXNames)
		{
			auto pFrames = GeneralUtils::GetAnimationPCX(name);
			if (pFrames && !pFrames->empty())
			{
				for (const auto& frame : *pFrames)
				{
					if (frame.Exists())
						this->targetMarkerPCXSurfaces.push_back(frame.GetSurface());
				}
			}
		}
	}
	if (this->targetMarkerPCXSurfaces.empty())
	{
		const char* markerName = !this->targetMarkerFileName.empty() ? this->targetMarkerFileName.c_str() : "TARGET2.SHP";
		this->pTargetMarkerSHP = LoadMapSelSHP(markerName);
	}

	// Target crosshairs animation timer
	this->targetAnimTimer.Start(4);
}

void MapSelectionClass::CalculateLayout(DSurface* pSurface)
{
	if (!pSurface)
		return;

	int bgWidth = 640;
	int bgHeight = 400;

	if (this->pBackgroundPCX)
	{
		bgWidth = this->pBackgroundPCX->Width;
		bgHeight = this->pBackgroundPCX->Height;
	}
	else if (this->pBackgroundSHP)
	{
		bgWidth = this->pBackgroundSHP->Width;
		bgHeight = this->pBackgroundSHP->Height;
	}
	else if (this->clickMapData.Width > 0 && this->clickMapData.Height > 0)
	{
		bgWidth = this->clickMapData.Width;
		bgHeight = this->clickMapData.Height;
	}

	int screenWidth = pSurface->Width;
	int screenHeight = pSurface->Height;

	this->windowRectangle.X = (screenWidth - bgWidth) / 2;
	this->windowRectangle.Y = (screenHeight - bgHeight) / 2;
	this->windowRectangle.Width = bgWidth;
	this->windowRectangle.Height = bgHeight;

	Debug::Log("[MapSelection] Layout: Screen=(%dx%d), WindowRect=(%d,%d,%d,%d), TextRect=(%d,%d,%d,%d)\n",
		screenWidth, screenHeight,
		this->windowRectangle.X, this->windowRectangle.Y, this->windowRectangle.Width, this->windowRectangle.Height,
		this->textRectangle.X, this->textRectangle.Y, this->textRectangle.Width, this->textRectangle.Height);
}

int MapSelectionClass::GetChoiceIndexAtPoint(int screenX, int screenY)
{
	int relX = screenX - this->windowRectangle.X;
	int relY = screenY - this->windowRectangle.Y;

	if (relX < 0 || relX >= this->windowRectangle.Width || relY < 0 || relY >= this->windowRectangle.Height)
		return -1;

	// Clickmap pixel color index (exact OpenTS behavior)
	if (!this->clickMapData.Pixels.empty())
	{
		if (relX >= 0 && relX < this->clickMapData.Width && relY >= 0 && relY < this->clickMapData.Height)
		{
			BYTE pixelColor = this->clickMapData.Pixels[relY * this->clickMapData.Width + relX];
			// 0 and 255 are unselectable background/ocean pixels
			if (pixelColor > 0 && pixelColor < 255)
			{
				for (size_t i = 0; i < this->choices.size(); ++i)
				{
					if (this->choices[i].Index == static_cast<int>(pixelColor))
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
	for (auto& anim : this->backgroundAnims)
	{
		if (anim.Timer.Completed() && anim.TotalFrames > 1)
		{
			anim.CurrentFrame = (anim.CurrentFrame + 1) % anim.TotalFrames;
			anim.Timer.Start(anim.FrameDelay);
			this->repaintAll = true;
		}
	}

	if (this->targetAnimTimer.Completed())
	{
		if (!this->targetMarkerPCXSurfaces.empty())
		{
			int count = static_cast<int>(this->targetMarkerPCXSurfaces.size());
			if (count > 0)
			{
				this->idleTargetAnimFrame = (this->idleTargetAnimFrame + 1) % count;
				this->activeTargetAnimFrame = (this->activeTargetAnimFrame + 1) % count;
			}
		}
		else
		{
			this->idleTargetAnimFrame = (this->idleTargetAnimFrame + 1) % 32;
			this->activeTargetAnimFrame = 32 + ((this->activeTargetAnimFrame - 32 + 1) % 32);
		}
		this->targetAnimTimer.Start(4);
		this->repaintAll = true;
	}

	// Briefing typewriter text advance
	if (this->hoveredChoiceIdx >= 0 && this->typewriterTimer.Completed())
	{
		size_t totalChars = 0;
		for (const auto& l : this->briefingLines)
			totalChars += l.length();

		if (static_cast<size_t>(this->typewriterCharCount) < totalChars)
		{
			this->typewriterCharCount += 1;
			this->typewriterTimer.Start(1);
			this->repaintAll = true;

			// Play typing sound through game DirectSound mixer (never cuts off VoiceOver)
			if (this->typeSoundTimer.Completed())
			{
				if (RulesClass::Instance && RulesClass::Instance->MessageCharTyped >= 0)
				{
					VocClass::PlayGlobal(RulesClass::Instance->MessageCharTyped, 0x2000, 0.4f);
				}
				else
				{
					int typeIdx = VocClass::FindIndex("Type");
					if (typeIdx >= 0)
					{
						VocClass::PlayGlobal(typeIdx, 0x2000, 0.4f);
					}
				}
				this->typeSoundTimer.Start(3);
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

	ConvertClass* pDrawer = this->pPalette ? this->pPalette : FileSystem::PALETTE_PAL;
	ConvertClass* pOverlayDrawer = this->pOverlayPalette ? this->pOverlayPalette : pDrawer;

	// Draw Background (PCX or SHP)
	DrawElement(pSurface, this->windowRectangle, this->pBackgroundPCX, this->pBackgroundSHP, pDrawer, 0, -2);

	// Draw Overlays (World map / territory borders)
	if (!this->overlayPCXs.empty())
	{
		for (auto pPCX : this->overlayPCXs)
		{
			if (pPCX)
			{
				DrawElement(pSurface, this->windowRectangle, pPCX, nullptr, nullptr, 0, -2);
			}
		}
	}
	else
	{
		for (auto pOverlaySHP : this->overlaySHPs)
		{
			if (pOverlaySHP)
			{
				DrawElement(pSurface, this->windowRectangle, nullptr, pOverlaySHP, pOverlayDrawer, 0, -2);
			}
		}
	}

	// Draw Animation SHPs or PCX sequences (Faction Logo, Rotating Globe, Compass)
	for (const auto& anim : this->backgroundAnims)
	{
		if (!anim.PCXFrames.empty())
		{
			int frame = (anim.CurrentFrame >= 0 && anim.CurrentFrame < static_cast<int>(anim.PCXFrames.size())) ? anim.CurrentFrame : 0;
			BSurface* pFramePCX = anim.PCXFrames[frame];
			if (pFramePCX)
			{
				RectangleStruct animRect = {
					this->windowRectangle.X + anim.X,
					this->windowRectangle.Y + anim.Y,
					pFramePCX->Width,
					pFramePCX->Height
				};
				DrawElement(pSurface, animRect, pFramePCX, nullptr, nullptr, 0, -2);
			}
		}
		else if (anim.SHP)
		{
			RectangleStruct animRect = {
				this->windowRectangle.X + anim.X,
				this->windowRectangle.Y + anim.Y,
				anim.SHP->Width,
				anim.SHP->Height
			};
			ConvertClass* pAnimDrawer = anim.Palette ? anim.Palette : pDrawer;
			DrawElement(pSurface, animRect, nullptr, anim.SHP, pAnimDrawer, anim.CurrentFrame, -2);
		}
	}

	// Draw target markers / highlights for choices
	for (size_t i = 0; i < this->choices.size(); ++i)
	{
		const auto& choice = this->choices[i];
		if (choice.HasTargetCoord)
		{
			int drawX = this->windowRectangle.X + choice.TargetCoord.X;
			int drawY = this->windowRectangle.Y + choice.TargetCoord.Y;

			if (!this->targetMarkerPCXSurfaces.empty())
			{
				int frame = (static_cast<int>(i) == this->hoveredChoiceIdx) ? this->activeTargetAnimFrame : this->idleTargetAnimFrame;
				if (frame >= 0 && frame < static_cast<int>(this->targetMarkerPCXSurfaces.size()))
				{
					BSurface* pPCX = this->targetMarkerPCXSurfaces[frame];
					if (pPCX)
					{
						int halfW = pPCX->Width / 2;
						int halfH = pPCX->Height / 2;
						RectangleStruct markerRect = { drawX - halfW, drawY - halfH, pPCX->Width, pPCX->Height };
						DrawElement(pSurface, markerRect, pPCX, nullptr, nullptr, 0, -2);
					}
				}
			}
			else if (this->pTargetMarkerSHP)
			{
				int halfW = this->pTargetMarkerSHP->Width / 2;
				int halfH = this->pTargetMarkerSHP->Height / 2;
				RectangleStruct markerRect = { drawX - halfW, drawY - halfH, this->pTargetMarkerSHP->Width, this->pTargetMarkerSHP->Height };
				int frame = (static_cast<int>(i) == this->hoveredChoiceIdx) ? this->activeTargetAnimFrame : this->idleTargetAnimFrame;
				DrawElement(pSurface, markerRect, nullptr, this->pTargetMarkerSHP, pDrawer, frame, -2);
			}
			else
			{
				COLORREF color = (static_cast<int>(i) == this->hoveredChoiceIdx)
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
	if (!pSurface || this->hoveredChoiceIdx < 0 || this->briefingLines.empty())
		return;

	const auto& choice = this->choices[this->hoveredChoiceIdx];

	int drawX = this->windowRectangle.X + this->textRectangle.X;
	int drawY = this->windowRectangle.Y + this->textRectangle.Y;

	// Text rendering on the metallic HUD plate (TextRect: 92, 322, 332, 78)
	COLORREF textColor = Drawing::RGB_To_Int(choice.TextColor.R, choice.TextColor.G, choice.TextColor.B);
	TextPrintType style = (TextPrintType::FullShadow | TextPrintType::Point6Grad);

	int remainingChars = this->typewriterCharCount;
	int lineHeight = 13;

	for (size_t i = 0; i < this->briefingLines.size(); ++i)
	{
		if (remainingChars <= 0)
			break;

		const std::wstring& line = this->briefingLines[i];
		std::wstring visiblePart;

		if (static_cast<int>(line.length()) <= remainingChars)
		{
			visiblePart = line;
			remainingChars -= static_cast<int>(line.length());
		}
		else
		{
			visiblePart = line.substr(0, remainingChars);
			remainingChars = 0;
		}

		if (!visiblePart.empty())
		{
			RectangleStruct lineBounds = {
				drawX,
				drawY + static_cast<int>(i) * lineHeight,
				this->textRectangle.Width,
				lineHeight + 4
			};
			Point2D relPoint = { 0, 0 };
			pSurface->DrawText(visiblePart.c_str(), &lineBounds, &relPoint, textColor, 0, style);
		}
	}
}

void MapSelectionClass::PlayIntroSequence(DSurface* pSurface)
{
	if (!pSurface)
		return;

	ConvertClass* pDrawer = this->pPalette ? this->pPalette : FileSystem::PALETTE_PAL;
	ConvertClass* pOverlayDrawer = this->pOverlayPalette ? this->pOverlayPalette : pDrawer;

	auto pumpAndCheckSkip = [&]() -> bool
	{
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
		DrawElement(pSurface, this->windowRectangle, this->pBackgroundPCX, this->pBackgroundSHP, pDrawer, 0, -2);

		// Completed Overlays (drawn solid)
		if (!this->overlayPCXs.empty())
		{
			for (int o = 0; o < completedOverlays && o < static_cast<int>(this->overlayPCXs.size()); ++o)
			{
				if (this->overlayPCXs[o])
				{
					DrawElement(pSurface, this->windowRectangle, this->overlayPCXs[o], nullptr, nullptr, 0, -2);
				}
			}
		}
		else
		{
			for (int o = 0; o < completedOverlays && o < static_cast<int>(this->overlaySHPs.size()); ++o)
			{
				if (this->overlaySHPs[o])
				{
					DrawElement(pSurface, this->windowRectangle, nullptr, this->overlaySHPs[o], pOverlayDrawer, 0, -2);
				}
			}
		}

		// Background Anims
		for (const auto& anim : this->backgroundAnims)
		{
			if (!anim.PCXFrames.empty())
			{
				int frame = (anim.CurrentFrame >= 0 && anim.CurrentFrame < static_cast<int>(anim.PCXFrames.size())) ? anim.CurrentFrame : 0;
				BSurface* pFramePCX = anim.PCXFrames[frame];
				if (pFramePCX)
				{
					RectangleStruct animRect = {
						this->windowRectangle.X + anim.X,
						this->windowRectangle.Y + anim.Y,
						pFramePCX->Width,
						pFramePCX->Height
					};
					DrawElement(pSurface, animRect, pFramePCX, nullptr, nullptr, 0, -2);
				}
			}
			else if (anim.SHP)
			{
				RectangleStruct animRect = {
					this->windowRectangle.X + anim.X,
					this->windowRectangle.Y + anim.Y,
					anim.SHP->Width,
					anim.SHP->Height
				};
				ConvertClass* pAnimDrawer = anim.Palette ? anim.Palette : pDrawer;
				DrawElement(pSurface, animRect, nullptr, anim.SHP, pAnimDrawer, anim.CurrentFrame, -2);
			}
		}

		// Completed Targets
		for (int t = 0; t < completedTargets && t < static_cast<int>(this->choices.size()); ++t)
		{
			const auto& choice = this->choices[t];
			if (choice.HasTargetCoord)
			{
				int drawX = this->windowRectangle.X + choice.TargetCoord.X;
				int drawY = this->windowRectangle.Y + choice.TargetCoord.Y;

				if (!this->targetMarkerPCXSurfaces.empty())
				{
					if (this->idleTargetAnimFrame >= 0 && this->idleTargetAnimFrame < static_cast<int>(this->targetMarkerPCXSurfaces.size()))
					{
						BSurface* pPCX = this->targetMarkerPCXSurfaces[this->idleTargetAnimFrame];
						if (pPCX)
						{
							int halfW = pPCX->Width / 2;
							int halfH = pPCX->Height / 2;
							RectangleStruct markerRect = { drawX - halfW, drawY - halfH, pPCX->Width, pPCX->Height };
							DrawElement(pSurface, markerRect, pPCX, nullptr, nullptr, 0, -2);
						}
					}
				}
				else if (this->pTargetMarkerSHP)
				{
					int halfW = this->pTargetMarkerSHP->Width / 2;
					int halfH = this->pTargetMarkerSHP->Height / 2;
					RectangleStruct markerRect = { drawX - halfW, drawY - halfH, this->pTargetMarkerSHP->Width, this->pTargetMarkerSHP->Height };
					DrawElement(pSurface, markerRect, nullptr, this->pTargetMarkerSHP, pDrawer, this->idleTargetAnimFrame, -2);
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

	size_t totalOverlays = !this->overlayPCXs.empty() ? this->overlayPCXs.size() : this->overlaySHPs.size();

	// Animate Overlays fade-in
	for (size_t o = 0; o < totalOverlays; ++o)
	{
		if (!this->overlaySound.empty())
		{
			PlayMapSelAudio(this->overlaySound.c_str());
		}

		for (int stage = 0; stage < 4; ++stage)
		{
			if (pumpAndCheckSkip())
				return;

			this->UpdateAnimations();
			renderBase(static_cast<int>(o), 0);

			if (!this->overlayPCXs.empty())
			{
				if (this->overlayPCXs[o])
				{
					DrawElement(pSurface, this->windowRectangle, this->overlayPCXs[o], nullptr, nullptr, 0, -2);
				}
			}
			else if (o < this->overlaySHPs.size() && this->overlaySHPs[o])
			{
				DrawElement(pSurface, this->windowRectangle, nullptr, this->overlaySHPs[o], pOverlayDrawer, 0, -2, fadeFlags[stage]);
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
	for (size_t t = 0; t < this->choices.size(); ++t)
	{
		const auto& choice = this->choices[t];
		if (!choice.HasTargetCoord)
			continue;

		if (!this->targetFlyInSound.empty())
		{
			PlayMapSelAudio(this->targetFlyInSound.c_str());
		}

		int drawX = this->windowRectangle.X + choice.TargetCoord.X;
		int drawY = this->windowRectangle.Y + choice.TargetCoord.Y;

		if (!this->targetFlyInPCXSurfaces.empty())
		{
			int maxFrames = static_cast<int>(this->targetFlyInPCXSurfaces.size());
			for (int f = 0; f < maxFrames; ++f)
			{
				if (pumpAndCheckSkip())
					return;

				this->UpdateAnimations();
				renderBase(static_cast<int>(totalOverlays), static_cast<int>(t));

				BSurface* pFlyInPCX = this->targetFlyInPCXSurfaces[f];
				if (pFlyInPCX)
				{
					int halfW = pFlyInPCX->Width / 2;
					int halfH = pFlyInPCX->Height / 2;
					RectangleStruct flyInRect = { drawX - halfW, drawY - halfH, pFlyInPCX->Width, pFlyInPCX->Height };
					DrawElement(pSurface, flyInRect, pFlyInPCX, nullptr, nullptr, 0, -2);
				}

				GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
				Sleep(35);
			}
		}
		else if (this->pTargetFlyInSHP && this->pTargetFlyInSHP->Frames > 0)
		{
			int halfW = this->pTargetFlyInSHP->Width / 2;
			int halfH = this->pTargetFlyInSHP->Height / 2;
			RectangleStruct flyInRect = { drawX - halfW, drawY - halfH, this->pTargetFlyInSHP->Width, this->pTargetFlyInSHP->Height };

			int maxFrames = this->pTargetFlyInSHP->Frames;
			for (int f = 0; f < maxFrames; ++f)
			{
				if (pumpAndCheckSkip())
					return;

				this->UpdateAnimations();
				renderBase(static_cast<int>(totalOverlays), static_cast<int>(t));

				// Draw fly-in frame
				BlitterFlags flag = (f < 4) ? fadeFlags[f] : BlitterFlags::None;
				DrawElement(pSurface, flyInRect, nullptr, this->pTargetFlyInSHP, pDrawer, f, -2, flag);

				GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
				Sleep(35);
			}
		}

		// Small delay after target settles
		for (int d = 0; d < 3; ++d)
		{
			if (pumpAndCheckSkip())
				return;
			this->UpdateAnimations();
			renderBase(static_cast<int>(totalOverlays), static_cast<int>(t + 1));
			GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
			Sleep(35);
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

	// Play Bink video if MapVQ is specified and exists
	if (!this->mapVQFileName.empty())
	{
		PlayMapSelBinkVideo(this->mapVQFileName.c_str());

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

	// Play stage VoiceOver audio if present
	if (!this->voiceOverFileName.empty())
	{
		PlayMapSelAudio(this->voiceOverFileName.c_str());
	}

	pSurface->Fill(0);
	this->CalculateLayout(pSurface);

	// Play animated intro sequence (Overlays sweep fade-in and TARGET1 fly-ins)
	this->PlayIntroSequence(pSurface);

	// Play Map Selection Theme Song (only if Theme= is explicitly specified)
	if (!this->themeName.empty())
	{
		int themeIdx = ThemeClass::Instance.FindIndex(this->themeName.c_str());
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

	bool wasLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool wasMouseOnMap = false;

	this->hoveredChoiceIdx = -1;
	this->lastHoveredChoiceIdx = -1;
	this->selectedChoiceIdx = -1;

	Debug::Log("[MapSelection] Entering modal Run loop (choices count: %zu)\n", this->choices.size());

	while (this->selectedChoiceIdx < 0)
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

		POINT mousePos = { 0, 0 };
		GetCursorPos(&mousePos);
		HWND hGameWnd = Game::hWnd;
		if (hGameWnd)
		{
			ScreenToClient(hGameWnd, &mousePos);

			RECT clientRect;
			if (GetClientRect(hGameWnd, &clientRect))
			{
				int clientW = clientRect.right - clientRect.left;
				int clientH = clientRect.bottom - clientRect.top;
				if (clientW > 0 && clientH > 0 && pSurface->Width > 0 && pSurface->Height > 0)
				{
					if (clientW != pSurface->Width || clientH != pSurface->Height)
					{
						mousePos.x = static_cast<int>(mousePos.x * (static_cast<double>(pSurface->Width) / clientW));
						mousePos.y = static_cast<int>(mousePos.y * (static_cast<double>(pSurface->Height) / clientH));
					}
				}
			}
		}

		bool isMouseOnMap = (mousePos.x >= this->windowRectangle.X && mousePos.x < this->windowRectangle.X + this->windowRectangle.Width &&
		                     mousePos.y >= this->windowRectangle.Y && mousePos.y < this->windowRectangle.Y + this->windowRectangle.Height);

		if (isMouseOnMap != wasMouseOnMap)
		{
			if (isMouseOnMap && !this->mouseOnMapSound.empty())
			{
				PlayMapSelAudio(this->mouseOnMapSound.c_str());
			}
			else if (!isMouseOnMap && !this->mouseOffMapSound.empty())
			{
				PlayMapSelAudio(this->mouseOffMapSound.c_str());
			}
			wasMouseOnMap = isMouseOnMap;
		}

		this->hoveredChoiceIdx = isMouseOnMap ? this->GetChoiceIndexAtPoint(mousePos.x, mousePos.y) : -1;

		if (this->hoveredChoiceIdx != this->lastHoveredChoiceIdx)
		{
			// Stop previous VoiceOver if moving off or switching regions
			PlaySoundA(NULL, NULL, 0);
			VoxClass::DeleteAll();

			this->typewriterCharCount = 0;
			this->typewriterTimer.Start(1);
			this->typeSoundTimer.Start(3);

			if (this->hoveredChoiceIdx >= 0 && this->hoveredChoiceIdx < static_cast<int>(this->choices.size()))
			{
				const auto& choice = this->choices[this->hoveredChoiceIdx];
				std::wstring fullText = ResolveCSFOrText(choice.Description);
				if (fullText.empty())
					fullText = ResolveCSFOrText(choice.Summary);
				if (fullText.empty())
					fullText = std::wstring(choice.StageName.begin(), choice.StageName.end());

				this->briefingLines = WordWrapText(fullText, 46);

				if (!choice.VoiceOver.empty())
				{
					PlayMapSelAudio(choice.VoiceOver.c_str());
				}
				else if (!choice.HoverSound.empty())
				{
					PlayMapSelAudio(choice.HoverSound.c_str());
				}
				else if (!this->enterRegionSound.empty())
				{
					PlayMapSelAudio(this->enterRegionSound.c_str());
				}
			}
			else
			{
				this->briefingLines.clear();
				if (this->lastHoveredChoiceIdx >= 0 && !this->exitRegionSound.empty())
				{
					PlayMapSelAudio(this->exitRegionSound.c_str());
				}
			}

			this->lastHoveredChoiceIdx = this->hoveredChoiceIdx;
			this->repaintAll = true;
		}

		bool isLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		if (isLButtonDown && !wasLButtonDown)
		{
			// Only select if the mouse is hovering over a valid choice on the map
			if (this->hoveredChoiceIdx >= 0 && this->hoveredChoiceIdx < static_cast<int>(this->choices.size()))
			{
				int clickedChoice = this->GetChoiceIndexAtPoint(mousePos.x, mousePos.y);
				if (clickedChoice == this->hoveredChoiceIdx)
				{
					this->selectedChoiceIdx = clickedChoice;
					Debug::Log("[MapSelection] Region clicked on hovered choice %d ('%s')\n",
						this->selectedChoiceIdx, this->choices[this->selectedChoiceIdx].StageName.c_str());

					// Immediately cut off VoiceOver and any background Windows speech
					PlaySoundA(NULL, NULL, 0);
					VoxClass::DeleteAll();

					const auto& chosen = this->choices[this->selectedChoiceIdx];
					if (!chosen.ClickSound.empty())
					{
						PlayMapSelAudio(chosen.ClickSound.c_str());
					}
					else if (!this->clickRegionSound.empty())
					{
						PlayMapSelAudio(this->clickRegionSound.c_str());
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
	PlaySoundA(NULL, NULL, 0);
	VoxClass::DeleteAll();
	if (!this->themeName.empty())
	{
		ThemeClass::Instance.Stop(true);
	}

	if (this->selectedChoiceIdx >= 0 && this->selectedChoiceIdx < static_cast<int>(this->choices.size()))
	{
		const auto& choice = this->choices[this->selectedChoiceIdx];
		Debug::Log("[MapSelection] Final selected mission: '%s' (Scenario: '%s')\n", choice.StageName.c_str(), choice.ScenarioPath.c_str());

		if (this->pCurrentScenario && !choice.ScenarioPath.empty())
		{
			this->pCurrentScenario->Stage = 0;
			strncpy(this->pCurrentScenario->FileName, choice.ScenarioPath.c_str(), 0x104u);
			this->pCurrentScenario->FileName[259] = '\0';
			strncpy(this->pCurrentScenario->NextScenario, choice.ScenarioPath.c_str(), 0x104u);
			this->pCurrentScenario->NextScenario[259] = '\0';
		}
		return true;
	}

	return false;
}

#pragma once

#include <GeneralStructures.h>
#include <Timer.h>
#include <vector>
#include <string>
#include <map>

struct SHPStruct;
class BSurface;
class DSurface;
class ConvertClass;
class ScenarioClass;

struct MapSelectChoice
{
	int Index { 0 };
	std::string StageName {};
	std::string ScenarioPath {};
	std::string Description {};
	std::string Summary {};
	std::string VoiceOver {};
	std::string HoverSound {};
	std::string ClickSound {};
	Point2D TargetCoord { 0, 0 };
	bool HasTargetCoord { false };
	ColorStruct TextColor { 255, 239, 99 };
	bool HasCustomTextColor { false };
};

struct MapSelectAnim
{
	SHPStruct* SHP { nullptr };
	std::vector<BSurface*> PCXFrames;
	std::string PaletteName {};
	ConvertClass* Palette { nullptr };
	int X { 0 };
	int Y { 0 };
	int FrameDelay { 5 };
	int CurrentFrame { 0 };
	int TotalFrames { 0 };
	SysTimerClass Timer {};
};

struct MapSelectPCX
{
	int Width { 0 };
	int Height { 0 };
	std::vector<BYTE> Pixels;
};

class MapSelectionClass
{
public:
	MapSelectionClass();
	~MapSelectionClass();

	static bool OpenMapSelectionWindow(ScenarioClass* pScenario);

	bool Initialize(ScenarioClass* pScenario);
	bool Run();

private:
	void LoadConfig(ScenarioClass* pScenario);
	void LoadAssets();
	void CalculateLayout(DSurface* pSurface);
	int GetChoiceIndexAtPoint(int screenX, int screenY);
	void UpdateAnimations();
	void Render(DSurface* pSurface);
	void DrawBriefing(DSurface* pSurface);
	void PlayIntroSequence(DSurface* pSurface);

	ScenarioClass* pCurrentScenario { nullptr };
	std::string currentStageName {};
	std::string selectedScenarioPath {};

	// Layout & resolution
	RectangleStruct windowRectangle { 0, 0, 800, 600 };
	RectangleStruct textRectangle { 92, 322, 332, 78 };

	// Background & Visual Assets
	std::string backgroundFileName {};
	std::string backgroundPCXFileName {};
	std::string clickMapFileName {};
	std::string paletteFileName { "mapsel.pal" };
	std::string overlayPaletteFileName { "msovrly.pal" };
	ConvertClass* pPalette { nullptr };
	ConvertClass* pOverlayPalette { nullptr };
	BSurface* pBackgroundPCX { nullptr };
	SHPStruct* pBackgroundSHP { nullptr };

	std::string targetMarkerFileName {};
	std::string targetFlyInFileName {};
	std::vector<std::string> targetMarkerPCXNames;
	std::vector<std::string> targetFlyInPCXNames;
	SHPStruct* pTargetMarkerSHP { nullptr };
	SHPStruct* pTargetFlyInSHP { nullptr };
	std::vector<BSurface*> targetMarkerPCXSurfaces;
	std::vector<BSurface*> targetFlyInPCXSurfaces;

	int idleTargetAnimFrame { 0 };
	int activeTargetAnimFrame { 32 };
	SysTimerClass targetAnimTimer {};
	MapSelectPCX clickMapData {};

	std::vector<std::string> overlaySHPNames;
	std::vector<std::string> overlayPCXNames;
	std::vector<SHPStruct*> overlaySHPs;
	std::vector<BSurface*> overlayPCXs;

	std::vector<MapSelectAnim> backgroundAnims;

	// Sound & Audio & Video
	std::string mapVQFileName {};
	std::string voiceOverFileName {};
	std::string overlaySound {};
	std::string targetFlyInSound {};
	std::string enterRegionSound {};
	std::string exitRegionSound {};
	std::string clickRegionSound {};
	std::string mouseOnMapSound {};
	std::string mouseOffMapSound {};
	std::string typeSound {};
	std::string themeName {};

	// Choices / Progression
	std::vector<MapSelectChoice> choices;
	int hoveredChoiceIdx { -1 };
	int selectedChoiceIdx { -1 };
	int lastHoveredChoiceIdx { -2 };

	// Briefing typewriter animation
	int typewriterCharCount { 0 };
	SysTimerClass typewriterTimer {};
	SysTimerClass typeSoundTimer {};
	std::vector<std::wstring> briefingLines;

	ColorStruct defaultTextColor { 255, 239, 99 };

	// State
	bool repaintAll { true };
	bool isInitialized { false };
};

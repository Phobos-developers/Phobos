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
	std::string StageName { };
	std::string ScenarioPath { };
	std::string Description { };
	std::string Summary { };
	std::string VoiceOver { };
	std::string HoverSound { };
	std::string ClickSound { };
	Point2D TargetCoord { 0, 0 };
	bool HasTargetCoord { false };
	ColorStruct TextColor { 255, 239, 99 };
	bool HasCustomTextColor { false };
};

struct MapSelectAnim
{
	SHPStruct* SHP { nullptr };
	std::vector<BSurface*> PCXFrames;
	std::string PaletteName { };
	ConvertClass* Palette { nullptr };
	int X { 0 };
	int Y { 0 };
	int FrameDelay { 5 };
	int CurrentFrame { 0 };
	int TotalFrames { 0 };
	SysTimerClass Timer { };
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

	ScenarioClass* CurrentScenario { nullptr };
	std::string CurrentStageName { };
	std::string SelectedScenarioPath { };

	// Layout & resolution
	RectangleStruct WindowRectangle { 0, 0, 800, 600 };
	RectangleStruct TextRectangle { 92, 322, 332, 78 };

	// Background & Visual Assets
	std::string BackgroundFileName { };
	std::string BackgroundPCXFileName { };
	std::string ClickMapFileName { };
	std::string PaletteFileName { "mapsel.pal" };
	std::string OverlayPaletteFileName { "msovrly.pal" };
	ConvertClass* Palette { nullptr };
	ConvertClass* OverlayPalette { nullptr };
	BSurface* BackgroundPCX { nullptr };
	SHPStruct* BackgroundSHP { nullptr };

	std::string TargetMarkerFileName { };
	std::string TargetFlyInFileName { };
	std::vector<std::string> TargetMarkerPCXNames;
	std::vector<std::string> TargetFlyInPCXNames;
	SHPStruct* TargetMarkerSHP { nullptr };
	SHPStruct* TargetFlyInSHP { nullptr };
	std::vector<BSurface*> TargetMarkerPCXSurfaces;
	std::vector<BSurface*> TargetFlyInPCXSurfaces;

	int IdleTargetAnimFrame { 0 };
	int ActiveTargetAnimFrame { 32 };
	SysTimerClass TargetAnimTimer { };
	MapSelectPCX ClickMapData { };

	std::vector<std::string> OverlaySHPNames;
	std::vector<std::string> OverlayPCXNames;
	std::vector<SHPStruct*> OverlaySHPs;
	std::vector<BSurface*> OverlayPCXs;

	std::vector<MapSelectAnim> BackgroundAnims;

	// Sound & Audio & Video
	std::string MapVQFileName { };
	std::string VoiceOverFileName { };
	std::string OverlaySound { };
	std::string TargetFlyInSound { };
	std::string EnterRegionSound { };
	std::string ExitRegionSound { };
	std::string ClickRegionSound { };
	std::string MouseOnMapSound { };
	std::string MouseOffMapSound { };
	std::string TypeSound { };
	std::string ThemeName { };

	// Choices / Progression
	std::vector<MapSelectChoice> Choices;
	int HoveredChoiceIdx { -1 };
	int SelectedChoiceIdx { -1 };
	int LastHoveredChoiceIdx { -2 };

	// Briefing typewriter animation
	int TypewriterCharCount { 0 };
	SysTimerClass TypewriterTimer { };
	SysTimerClass TypeSoundTimer { };
	std::vector<std::wstring> BriefingLines;

	ColorStruct DefaultTextColor { 255, 239, 99 };

	// State
	bool RepaintAll { true };
	bool IsInitialized { false };
};

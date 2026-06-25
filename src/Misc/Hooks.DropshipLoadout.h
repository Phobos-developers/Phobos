#pragma once

#include <Timer.h>
#include <GeneralStructures.h>
#include <Ext/HouseType/Body.h>
#include <vector>
#include <map>

// Forward declarations
struct SHPStruct;
class BSurface;
class ConvertClass;
class TechnoTypeClass;
class ShapeButtonClass;
class ToggleClass;

class DropshipLoadoutClass
{
public:
	DropshipLoadoutClass();
	~DropshipLoadoutClass();

	bool Initialize(bool bIgnoreFixedUnits = false, bool bPreloadCargo = false, int allowableUnitsIndex = 0, int startingMoney = 0, Nullable<bool> bAddUnusedMoneyToPlayer = {}, Nullable<bool> bRememberPurchasedCargo = {}, SuperWeaponTypeClass* pSWType = nullptr);
	void Run();

	static void OpenInGameWindow(bool bIgnoreFixedUnits = false, bool bPreloadCargo = false, int allowableUnitsIndex = 0, int startingMoney = 0, Nullable<bool> bAddUnusedMoneyToPlayer = {}, Nullable<bool> bRememberPurchasedCargo = {}, SuperWeaponTypeClass* pSWType = nullptr);

private:
	void LoadAssets();
	void CalculateLayout(DSurface* pSurface);
	void CreateControls();
	void HandleInput(int command, int buttonID);
	void UpdateAnimations();
	void Render(DSurface* pSurface);
	void DrawTooltip(DSurface* pSurface);
	void SaveCargo();
	int GetCarrierSizeLimit(int carrierIdx);
	bool CanCarrierHoldUnit(int carrierIdx, TechnoTypeClass* pUnitType);

	// Extensions
	HouseTypeExt::ExtData* pHouseTypeExt { nullptr };
	SuperWeaponTypeClass* pSWType { nullptr };
	class SWTypeExt::ExtData* pSWTypeExt { nullptr };

	// Config & state
	int nStartingDropships { 0 };
	long initialMoney { 0 };
	long currentMoney { 0 };
	int nSidebarCameos { 8 };
	int nDropshipBayCameos { 5 };
	int nDropshipBayTotalSlots { 0 };
	int firstBrowsableCameo { 0 };
	bool pressedSpaceKey { false };
	bool repaintAll { true };
	bool lastTimeWasOverCameos { false };
	bool freeDropshipSlots { false };
	bool bIgnoreFixedUnits { false };
	bool bPreloadCargo { false };
	Nullable<bool> bAddUnusedMoneyToPlayer {};
	Nullable<bool> bRememberPurchasedCargo {};
	int allowableUnitsIndex { 0 };
	int startingMoney { 0 };

	// Assets (Palette, surfaces, SHPs)
	ConvertClass* dropshipLoadout_Palette { nullptr };
	SHPStruct* dropshipLoadout_Background { nullptr };
	SHPStruct* dropshipLoadout_UpArrow { nullptr };
	SHPStruct* dropshipLoadout_DownArrow { nullptr };
	SHPStruct* dropshipLoadout_Loadout { nullptr };
	SHPStruct* dropshipLoadout_PilotLit { nullptr };
	std::vector<SHPStruct*> dropshipLoadout_DGreenList;

	BSurface* dropshipLoadout_BackgroundPCX { nullptr };
	BSurface* dropshipLoadout_UpArrowPCX { nullptr };
	BSurface* dropshipLoadout_DownArrowPCX { nullptr };
	std::vector<BSurface*> dropshipLoadout_LoadoutPCX;
	std::vector<BSurface*> dropshipLoadout_PilotLitPCX;
	std::vector<std::vector<BSurface*>> dropshipLoadout_DGreenListPCX;

	// Unit lists
	std::vector<TechnoTypeClass*> availableUnits;
	std::vector<int> availableUnitsMaximums;
	std::vector<std::vector<TechnoTypeClass*>> dropshipBayChosenUnitsLists;
	std::vector<std::vector<bool>> dropshipBayFixedUnitsLists;
	std::map<TechnoTypeClass*, int> dropshipBayChosenUnitsCount;
	TechnoTypeClass* lastSelected { nullptr };
	TechnoTypeClass* pHoveredUnitType { nullptr };
	int hoveredDropshipIdx { -1 };
	int hoveredSlotIdx { -1 };

	// Layout/Locations
	RectangleStruct windowRectangle;
	int upArrowX { 0 }, upArrowY { 0 };
	int downArrowX { 0 }, downArrowY { 0 };
	RectangleStruct upArrowLocation;
	RectangleStruct downArrowLocation;
	std::vector<RectangleStruct> sidebarCameLocations;
	std::vector<std::vector<RectangleStruct>> dropshipBayCameLocations;
	RectangleStruct loadoutLocation;
	RectangleStruct pilotLitLocation;
	std::vector<RectangleStruct> dGreenLocation;

	// Interactive Buttons
	std::vector<ShapeButtonClass*> buttonsList;
	ToggleClass* commandManager { nullptr };

	// Animations & Timers
	int currentLoadoutFrame { -1 };
	int currentPilotLitFrame { -1 };
	int loadoutFrameDelay { 11 };
	int pilotLitFrameDelay { 15 };
	int loadoutTotalFrames { 0 };
	int pilotLitTotalFrames { 0 };
	int animTimer_StartValue { 15 };
	int animTimer_DelayedStartValue_Loadout { 0 };
	int animTimer_DelayedStartValue_PilotLit { 0 };

	SysTimerClass animTimer_UpdateFrameTimer;
	SysTimerClass animTimer_DelayedStartTimer_Loadout;
	SysTimerClass animTimer_UpdateFrameTimer_Loadout;
	SysTimerClass animTimer_DelayedStartTimer_PilotLit;
	SysTimerClass animTimer_UpdateFrameTimer_PilotLit;

	int sidebarRowAnimationIndex { -1 };
	int currentSidebarRowAnimationFrame { 0 };
	int sidebarRowAnimationFrameDelay { 5 };
	int sidebarRowAnimationTotalFrames { 0 };
	SysTimerClass animTimer_UpdateFrameTimer_SidebarRowAnimation;

	// Sounds
	int buyClickSoundIdx { -1 };
	int sellClickSoundIdx { -1 };
	int arrowsClickSoundIdx { -1 };
	int startingDragDropSoundIdx { -1 };
	int endingDragDropSoundIdx { -1 };

	// Drag & Drop state
	bool bIsDragging { false };
	bool bDragPending { false };
	TechnoTypeClass* pDraggedUnitType { nullptr };
	int nSourceDropshipIdx { -1 };
	int nSourceSlotIdx { -1 };
	bool bDraggedIsFixed { false };
	Point2D dragStartMousePos { 0, 0 };
};

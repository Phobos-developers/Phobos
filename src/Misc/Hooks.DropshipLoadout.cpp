
#include <ScenarioClass.h>

#include <ThemeClass.h>
#include <WWMouseClass.h>
#include <MouseClass.h>
#include <DisplayClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <BitFont.h>
#include <BitText.h>
#include <Ext/TechnoType/Body.h>
#include <sstream>
#include <iomanip>

#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Scenario/Body.h>
#include <ToggleClass.h>
#include <ShapeButtonClass.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Misc/Hooks.DropshipLoadout.h>

static bool bDropshipLoadoutActive = false;
static int pendingScrolls = 0;

static void FillRectTranslucent(DSurface* pSurface, const RectangleStruct& rect, const ColorStruct& color, int opacity)
{
	if (!pSurface || opacity <= 0)
		return;

	if (opacity >= 255)
	{
		pSurface->FillRectTrans(const_cast<RectangleStruct*>(&rect), const_cast<ColorStruct*>(&color), opacity);
		return;
	}

	if (pSurface->GetBytesPerPixel() < 2)
	{
		pSurface->FillRectTrans(const_cast<RectangleStruct*>(&rect), const_cast<ColorStruct*>(&color), opacity);
		return;
	}

	RectangleStruct bound = Drawing::Intersect(rect, pSurface->GetRect());
	if (bound.Width <= 0 || bound.Height <= 0)
		return;

	const auto line_length = pSurface->GetPitch() / sizeof(WORD);
	auto ptr = (WORD*)pSurface->Lock(bound.X, bound.Y);

	if (!ptr)
		return;

	int alpha = opacity;
	int invAlpha = 255 - alpha;

	auto p = ptr;

	for (int y = 0; y < bound.Height; ++y)
	{
		auto q = p;

		for (int x = 0; x < bound.Width; ++x)
		{
			BYTE r, g, b;
			Drawing::Int_To_RGB(*q, r, g, b);

			int newR = (color.R * alpha + r * invAlpha) / 255;
			int newG = (color.G * alpha + g * invAlpha) / 255;
			int newB = (color.B * alpha + b * invAlpha) / 255;

			*q = (WORD)Drawing::RGB_To_Int(newR, newG, newB);
			++q;
		}

		p += line_length;
	}

	pSurface->Unlock();
}

bool IsDropshipLoadoutActive()
{
	return bDropshipLoadoutActive;
}

void DropshipLoadout_OnMouseWheelUp()
{
	pendingScrolls--;
}

void DropshipLoadout_OnMouseWheelDown()
{
	pendingScrolls++;
}

static ShapeButtonClass* CreateShapeButton(unsigned int nID, int nX, int nY, int nWidth, int nHeight, bool bIsAlpha)
{
	auto const pButton = GameAllocator<ShapeButtonClass>().allocate(1);

	if (!pButton)
		return nullptr;

	using ShapeButtonConstructor_t = ShapeButtonClass * (__thiscall*)(
		ShapeButtonClass* pThis,
		unsigned int nID,
		int nX,
		int nY,
		int nWidth,
		int nHeight,
		ConvertClass* pDrawer,
		bool bIsAlpha
	);

	auto const pConstructor = reinterpret_cast<ShapeButtonConstructor_t>(0x69DD30);
	return pConstructor(pButton, nID, nX, nY, nWidth, nHeight, nullptr, bIsAlpha);
}



DropshipLoadoutClass::DropshipLoadoutClass()
{
}

DropshipLoadoutClass::~DropshipLoadoutClass()
{
	if (commandManager)
	{
		commandManager->TurnOff();
		commandManager = nullptr;
	}

	for (size_t i = 0; i < buttonsList.size(); ++i)
	{
		auto button = buttonsList[i];

		if (button)
			GameDelete(button);
	}

	buttonsList.clear();
	dropshipLoadout_DGreenList.clear();
}

bool DropshipLoadoutClass::Initialize(bool bIgnoreFixedUnits, bool bPreloadCargo, int allowableUnitsIndex, int startingMoney, Nullable<bool> bAddUnusedMoneyToPlayer, Nullable<bool> bRememberPurchasedCargo, SuperWeaponTypeClass* pSWType)
{
	if (!HouseClass::CurrentPlayer)
		return false;

	this->bIgnoreFixedUnits = bIgnoreFixedUnits;
	this->bPreloadCargo = bPreloadCargo;
	this->bAddUnusedMoneyToPlayer = bAddUnusedMoneyToPlayer;
	this->bRememberPurchasedCargo = bRememberPurchasedCargo;
	this->allowableUnitsIndex = allowableUnitsIndex;
	this->startingMoney = startingMoney;
	this->pSWType = pSWType;
	this->pSWTypeExt = SWTypeExt::TryFetch(pSWType);

	pHouseTypeExt = HouseTypeExt::Fetch(HouseClass::CurrentPlayer->Type);

	if (!ScenarioClass::Instance)
		return false;

	if (pSWType)
	{
		nStartingDropships = 1;
	}
	else
	{
		nStartingDropships = pHouseTypeExt->DropshipLoadout_StartingDropships.isset() ? pHouseTypeExt->DropshipLoadout_StartingDropships.Get() : ScenarioExt::Global()->DropshipLoadout_StartingDropships;

		// Limit starting dropships to the number of configured carriers to prevent buying units in non-existent transport bays
		std::vector<TechnoTypeClass*> carriers;

		if (pHouseTypeExt->DropshipLoadout_Carriers.size() > 0)
		{
			for (auto carrier : pHouseTypeExt->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}
		else if (ScenarioExt::Global())
		{
			for (auto carrier : ScenarioExt::Global()->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}

		if (nStartingDropships > (int)carriers.size())
			nStartingDropships = (int)carriers.size();
	}

	if (nStartingDropships <= 0)
		return false;

	LoadAssets();

	return true;
}

void DropshipLoadoutClass::LoadAssets()
{
	auto const pGlobal = ScenarioExt::Global();
	auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer);

	if (pSWTypeExt)
	{
		// Palette
		if (pSWTypeExt->DropshipLoadout_Palette)
			dropshipLoadout_Palette = pSWTypeExt->DropshipLoadout_Palette;
		else
			dropshipLoadout_Palette = FileSystem::LoadPALFile("DROPSHIP.PAL", DSurface::Hidden);

		// Background PCX / SHP
		if (pSWTypeExt->DropshipLoadout_BackgroundPCX.isset() && pSWTypeExt->DropshipLoadout_BackgroundPCX.Get().Exists())
		{
			dropshipLoadout_BackgroundPCX = pSWTypeExt->DropshipLoadout_BackgroundPCX.Get().GetSurface();
		}
		else if (!pSWTypeExt->DropshipLoadout_BackgroundPCXPattern.empty())
		{
			char filename[260];
			_snprintf_s(filename, sizeof(filename), pSWTypeExt->DropshipLoadout_BackgroundPCXPattern.c_str(), 1);
			PhobosPCXFile runtimePCX(filename);

			if (runtimePCX.Exists())
				dropshipLoadout_BackgroundPCX = runtimePCX.GetSurface();
		}

		if (pSWTypeExt->DropshipLoadout_Background.isset())
			dropshipLoadout_Background = pSWTypeExt->DropshipLoadout_Background;
		else
			dropshipLoadout_Background = FileSystem::LoadSHPFile("DROP0001.SHP");

		// Loadout PCX / SHP
		if (pSWTypeExt->DropshipLoadout_LoadoutPCX.isset() && pSWTypeExt->DropshipLoadout_LoadoutPCX.Get().Exists())
			dropshipLoadout_LoadoutPCX.push_back(pSWTypeExt->DropshipLoadout_LoadoutPCX.Get().GetSurface());

		if (pSWTypeExt->DropshipLoadout_Loadout.isset())
			dropshipLoadout_Loadout = pSWTypeExt->DropshipLoadout_Loadout;
		else
			dropshipLoadout_Loadout = FileSystem::LoadSHPFile("LOADOUT.SHP");

		// PilotLit PCX / SHP
		if (pSWTypeExt->DropshipLoadout_PilotLitPCX.isset() && pSWTypeExt->DropshipLoadout_PilotLitPCX.Get().Exists())
			dropshipLoadout_PilotLitPCX.push_back(pSWTypeExt->DropshipLoadout_PilotLitPCX.Get().GetSurface());

		if (pSWTypeExt->DropshipLoadout_PilotLit.isset())
			dropshipLoadout_PilotLit = pSWTypeExt->DropshipLoadout_PilotLit;
		else
			dropshipLoadout_PilotLit = FileSystem::LoadSHPFile("PILOTLIT.SHP");

		// Up/Down Arrows PCX / SHP
		if (pSWTypeExt->DropshipLoadout_UpArrowPCX.isset() && pSWTypeExt->DropshipLoadout_UpArrowPCX.Get().Exists())
			dropshipLoadout_UpArrowPCX = pSWTypeExt->DropshipLoadout_UpArrowPCX.Get().GetSurface();

		if (pSWTypeExt->DropshipLoadout_UpArrow.isset())
			dropshipLoadout_UpArrow = pSWTypeExt->DropshipLoadout_UpArrow;
		else
			dropshipLoadout_UpArrow = FileSystem::LoadSHPFile("DROPUP.SHP");

		if (pSWTypeExt->DropshipLoadout_DownArrowPCX.isset() && pSWTypeExt->DropshipLoadout_DownArrowPCX.Get().Exists())
			dropshipLoadout_DownArrowPCX = pSWTypeExt->DropshipLoadout_DownArrowPCX.Get().GetSurface();

		if (pSWTypeExt->DropshipLoadout_DownArrow.isset())
			dropshipLoadout_DownArrow = pSWTypeExt->DropshipLoadout_DownArrow;
		else
			dropshipLoadout_DownArrow = FileSystem::LoadSHPFile("DROPDOWN.SHP");

		// DGreen PCX / SHP
		if (pSWTypeExt->DropshipLoadout_DGreenListPCX.size() > 0)
		{
			std::vector<BSurface*> rowAnimFrames;

			for (const auto& frame : pSWTypeExt->DropshipLoadout_DGreenListPCX)
			{
				if (frame.Exists())
					rowAnimFrames.push_back(frame.GetSurface());
			}

			if (!rowAnimFrames.empty())
				dropshipLoadout_DGreenListPCX.push_back(std::move(rowAnimFrames));
		}

		if (pSWTypeExt->DropshipLoadout_DGreenList.size() > 0)
		{
			for (const auto& shp : pSWTypeExt->DropshipLoadout_DGreenList)
				dropshipLoadout_DGreenList.push_back(shp);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				if (i == 0)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN1.SHP"));
				else if (i == 1)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN2.SHP"));
				else if (i == 2)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN3.SHP"));
				else if (i == 3)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN4.SHP"));
			}
		}

		// Sounds
		buyClickSoundIdx = pSWTypeExt->DropshipLoadout_BuyClickSound.isset() ? pSWTypeExt->DropshipLoadout_BuyClickSound : RulesClass::Instance->GenericClick;
		sellClickSoundIdx = pSWTypeExt->DropshipLoadout_SellClickSound.isset() ? pSWTypeExt->DropshipLoadout_SellClickSound : RulesClass::Instance->SellSound;
		arrowsClickSoundIdx = pSWTypeExt->DropshipLoadout_ArrowsClickSound.isset() ? pSWTypeExt->DropshipLoadout_ArrowsClickSound : RulesClass::Instance->GUITabSound;
		startingDragDropSoundIdx = pSWTypeExt->DropshipLoadout_StartingDragDropSound.isset() ? pSWTypeExt->DropshipLoadout_StartingDragDropSound : -1;
		endingDragDropSoundIdx = pSWTypeExt->DropshipLoadout_EndingDragDropSound.isset() ? pSWTypeExt->DropshipLoadout_EndingDragDropSound : -1;

		// Money
		bool usesPlayerWallet = false;
		long dropshipLoadout_InitialMoney = -1;
		if (this->startingMoney > 0)
		{
			dropshipLoadout_InitialMoney = this->startingMoney;
		}
		else if (this->startingMoney == 0)
		{
			if (pSWTypeExt->DropshipLoadout_Money.isset())
				dropshipLoadout_InitialMoney = pSWTypeExt->DropshipLoadout_Money;
		}

		if (dropshipLoadout_InitialMoney < 0)
		{
			dropshipLoadout_InitialMoney = HouseClass::CurrentPlayer->Available_Money();
			usesPlayerWallet = true;
		}

		this->initialMoney = dropshipLoadout_InitialMoney;
		this->currentMoney = dropshipLoadout_InitialMoney;

		bool rememberPurchasedCargo = pSWTypeExt->DropshipLoadout_RememberPurchasedCargo.Get();

		if (!pHouseExt->DropshipLoadout_SWInitialUnitsSet)
		{
			pHouseExt->DropshipLoadout_SWInitialUnits = pSWTypeExt->DropshipLoadout_InitialUnits;
			pHouseExt->DropshipLoadout_SWInitialUnitsSet = true;
		}

		// Initial units pool for SW (separate from map actions pool)
		std::vector<TechnoTypeClass*> initialUnitsRemaining;

		if (!bIgnoreFixedUnits)
		{
			for (auto pUnit : pHouseExt->DropshipLoadout_SWInitialUnits)
				if (pUnit)
					initialUnitsRemaining.push_back(pUnit);
		}

		// Preload Cargo
		long totalPreloadedCost = 0;
		bool canPreload = false;

		if (this->bPreloadCargo)
		{
			if (!pHouseExt->DropshipLoadout_SWCargo.empty())
			{
				const std::vector<TechnoTypeClass*>* pFixedList = nullptr;

				if (!this->bIgnoreFixedUnits && !pSWTypeExt->DropshipLoadout_FixedUnits.empty())
					pFixedList = &pSWTypeExt->DropshipLoadout_FixedUnits;

				std::vector<TechnoTypeClass*> fixedRemaining;

				if (pFixedList)
				{
					for (auto pUnit : *pFixedList)
					{
						if (pUnit)
							fixedRemaining.push_back(pUnit);
					}
				}

				for (auto pUnit : pHouseExt->DropshipLoadout_SWCargo)
				{
					if (!pUnit)
						continue;

					auto it = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);

					if (it != fixedRemaining.end())
					{
						fixedRemaining.erase(it);
					}
					else
					{
						// Check if it's a free initial unit
						auto itInitial = std::find(initialUnitsRemaining.begin(), initialUnitsRemaining.end(), pUnit);

						if (itInitial != initialUnitsRemaining.end())
							initialUnitsRemaining.erase(itInitial);
						else
							totalPreloadedCost += pUnit->Cost;
					}
				}

				if (usesPlayerWallet || rememberPurchasedCargo || currentMoney >= totalPreloadedCost)
					canPreload = true;
			}
		}

		if (canPreload)
		{
			if (!usesPlayerWallet && !rememberPurchasedCargo)
				currentMoney -= totalPreloadedCost;
		}
		else
		{
			this->bPreloadCargo = false;
		}

		// Available units lists
		std::vector<TechnoTypeClass*> allowableUnits;
		std::vector<int> allowableUnitMaximums;

		const std::vector<TechnoTypeClass*>* pSWAllowableList = nullptr;

		if (!pSWTypeExt->DropshipLoadout_AllowableUnits.empty())
			pSWAllowableList = &pSWTypeExt->DropshipLoadout_AllowableUnits;

		if (pSWAllowableList)
		{
			for (auto pUnit : *pSWAllowableList)
			{
				if (pUnit)
					allowableUnits.push_back(pUnit);
			}

			for (auto val : pSWTypeExt->DropshipLoadout_AllowableUnitMaximums)
			{
				allowableUnitMaximums.push_back(val);
			}
		}

		while (allowableUnitMaximums.size() < allowableUnits.size())
		{
			allowableUnitMaximums.push_back(-1);
		}

		for (size_t i = 0; i < allowableUnits.size(); ++i)
		{
			int maximumCount = -1;

			if (i < allowableUnitMaximums.size())
			{
				maximumCount = allowableUnitMaximums[i];

				if (maximumCount == 0)
					continue;
			}

			availableUnitsMaximums.push_back(maximumCount);
			TechnoTypeClass* pType = allowableUnits[i];
			availableUnits.push_back(pType);
		}

		// Ensure all initial units are in availableUnits so they can be bought back if removed
		for (auto pUnit : pSWTypeExt->DropshipLoadout_InitialUnits)
		{
			if (pUnit && std::find(availableUnits.begin(), availableUnits.end(), pUnit) == availableUnits.end())
			{
				availableUnits.push_back(pUnit);
				availableUnitsMaximums.push_back(-1);
			}
		}
	}
	else
	{
		if (pHouseTypeExt->DropshipLoadout_Palette)
			dropshipLoadout_Palette = pHouseTypeExt->DropshipLoadout_Palette;
		else if (pGlobal && pGlobal->DropshipLoadout_Palette)
			dropshipLoadout_Palette = pGlobal->DropshipLoadout_Palette;
		else
			dropshipLoadout_Palette = FileSystem::LoadPALFile("DROPSHIP.PAL", DSurface::Hidden);

		if (pHouseTypeExt->DropshipLoadout_BackgroundPCX.isset() && pHouseTypeExt->DropshipLoadout_BackgroundPCX.Get().Exists())
		{
			dropshipLoadout_BackgroundPCX = pHouseTypeExt->DropshipLoadout_BackgroundPCX.Get().GetSurface();
		}
		else if (!pHouseTypeExt->DropshipLoadout_BackgroundPCXPattern.empty())
		{
			// Re-format with the correct runtime nStartingDropships (parse-time value may have been 0)
			char filename[260];
			_snprintf_s(filename, sizeof(filename), pHouseTypeExt->DropshipLoadout_BackgroundPCXPattern.c_str(), nStartingDropships);
			PhobosPCXFile runtimePCX(filename);

			if (runtimePCX.Exists())
				dropshipLoadout_BackgroundPCX = runtimePCX.GetSurface();
			else if (pGlobal && pGlobal->DropshipLoadout_BackgroundPCX.Exists())
				dropshipLoadout_BackgroundPCX = pGlobal->DropshipLoadout_BackgroundPCX.GetSurface();
		}
		else if (pGlobal && pGlobal->DropshipLoadout_BackgroundPCX.Exists())
		{
			dropshipLoadout_BackgroundPCX = pGlobal->DropshipLoadout_BackgroundPCX.GetSurface();
		}

		if (pGlobal && pGlobal->DropshipLoadout_Background)
		{
			dropshipLoadout_Background = pGlobal->DropshipLoadout_Background;
		}
		else
		{
			char tempFilenameBuffer[32];
			_snprintf_s(tempFilenameBuffer, sizeof(tempFilenameBuffer), "DROP%04d.SHP", nStartingDropships);
			dropshipLoadout_Background = FileSystem::LoadSHPFile(_strdup(tempFilenameBuffer));
		}

		if (pHouseTypeExt->DropshipLoadout_LoadoutPCX.size() > 0)
		{
			for (auto& pFilePCX : pHouseTypeExt->DropshipLoadout_LoadoutPCX)
			{
				dropshipLoadout_LoadoutPCX.push_back(pFilePCX.GetSurface());
			}
		}
		else if (pGlobal && pGlobal->DropshipLoadout_LoadoutPCX.size() > 0)
		{
			for (auto& pFilePCX : pGlobal->DropshipLoadout_LoadoutPCX)
			{
				dropshipLoadout_LoadoutPCX.push_back(pFilePCX.GetSurface());
			}
		}

		if (pGlobal && pGlobal->DropshipLoadout_Loadout)
			dropshipLoadout_Loadout = pGlobal->DropshipLoadout_Loadout;
		else
			dropshipLoadout_Loadout = FileSystem::LoadSHPFile("LOADOUT.SHP");

		if (!pHouseTypeExt->DropshipLoadout_PilotLitPCX.empty())
		{
			for (const PhobosPCXFile& frame : pHouseTypeExt->DropshipLoadout_PilotLitPCX)
			{
				dropshipLoadout_PilotLitPCX.push_back(frame.GetSurface());
			}
		}
		else if (pGlobal && !pGlobal->DropshipLoadout_PilotLitPCX.empty())
		{
			for (auto& pFilePCX : pGlobal->DropshipLoadout_PilotLitPCX)
			{
				dropshipLoadout_PilotLitPCX.push_back(pFilePCX.GetSurface());
			}
		}

		if (pGlobal && pGlobal->DropshipLoadout_PilotLit)
			dropshipLoadout_PilotLit = pGlobal->DropshipLoadout_PilotLit;
		else
			dropshipLoadout_PilotLit = FileSystem::LoadSHPFile("PILOTLIT.SHP");

		if (pHouseTypeExt->DropshipLoadout_UpArrowPCX.isset() && pHouseTypeExt->DropshipLoadout_UpArrowPCX.Get().Exists())
			dropshipLoadout_UpArrowPCX = pHouseTypeExt->DropshipLoadout_UpArrowPCX.Get().GetSurface();
		else if (pGlobal && pGlobal->DropshipLoadout_UpArrowPCX.Exists())
			dropshipLoadout_UpArrowPCX = pGlobal->DropshipLoadout_UpArrowPCX.GetSurface();

		if (pGlobal && pGlobal->DropshipLoadout_UpArrow)
			dropshipLoadout_UpArrow = pGlobal->DropshipLoadout_UpArrow;
		else
			dropshipLoadout_UpArrow = FileSystem::LoadSHPFile("DROPUP.SHP");

		if (pHouseTypeExt->DropshipLoadout_DownArrowPCX.isset() && pHouseTypeExt->DropshipLoadout_DownArrowPCX.Get().Exists())
			dropshipLoadout_DownArrowPCX = pHouseTypeExt->DropshipLoadout_DownArrowPCX.Get().GetSurface();
		else if (pGlobal && pGlobal->DropshipLoadout_DownArrowPCX.Exists())
			dropshipLoadout_DownArrowPCX = pGlobal->DropshipLoadout_DownArrowPCX.GetSurface();

		if (pGlobal && pGlobal->DropshipLoadout_DownArrow)
			dropshipLoadout_DownArrow = pGlobal->DropshipLoadout_DownArrow;
		else
			dropshipLoadout_DownArrow = FileSystem::LoadSHPFile("DROPDOWN.SHP");

		if (pHouseTypeExt->DropshipLoadout_DGreenListPCX.size() > 0)
		{
			for (const auto& pAnimationVector : pHouseTypeExt->DropshipLoadout_DGreenListPCX)
			{
				auto& rowAnimFrames = dropshipLoadout_DGreenListPCX.emplace_back();

				if (pAnimationVector)
				{
					for (const auto& frame : *pAnimationVector)
					{
						rowAnimFrames.push_back(frame.GetSurface());
					}
				}
			}
		}
		else if (pGlobal && pGlobal->DropshipLoadout_DGreenListPCX.size() > 0)
		{
			for (auto& pFileGroupPCX : pGlobal->DropshipLoadout_DGreenListPCX)
			{
				auto& rowAnimFrames = dropshipLoadout_DGreenListPCX.emplace_back();

				if (pFileGroupPCX)
				{
					for (auto& pFilePCX : *pFileGroupPCX)
					{
						rowAnimFrames.push_back(pFilePCX.GetSurface());
					}
				}
			}

			for (int i = 0; i < 4 && dropshipLoadout_DGreenListPCX.size() < 4; i++)
			{
				dropshipLoadout_DGreenListPCX.emplace_back();
			}
		}

		for (int i = 0; i < 4; i++)
		{
			if (pGlobal && (pGlobal->DropshipLoadout_DGreenList.size() < 4 || pGlobal->DropshipLoadout_DGreenList[i] == nullptr))
			{
				if (i == 0)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN1.SHP"));
				else if (i == 1)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN2.SHP"));
				else if (i == 2)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN3.SHP"));
				else if (i == 3)
					dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN4.SHP"));
				else
					dropshipLoadout_DGreenList.push_back(nullptr);
			}
			else if (pGlobal)
			{
				dropshipLoadout_DGreenList.push_back(pGlobal->DropshipLoadout_DGreenList[i]);
			}
			else
			{
				dropshipLoadout_DGreenList.push_back(nullptr);
			}
		}

		if (pGlobal)
		{
			for (size_t i = 4; i < pGlobal->DropshipLoadout_DGreenList.size(); i++)
			{
				dropshipLoadout_DGreenList.push_back(pGlobal->DropshipLoadout_DGreenList[i]);
			}
		}

		buyClickSoundIdx = RulesClass::Instance->GenericClick;
		sellClickSoundIdx = RulesClass::Instance->SellSound;
		arrowsClickSoundIdx = RulesClass::Instance->GUITabSound;

		if (pHouseTypeExt->DropshipLoadout_BuyClickSound.isset())
			buyClickSoundIdx = pHouseTypeExt->DropshipLoadout_BuyClickSound;
		else if (pGlobal && pGlobal->DropshipLoadout_BuyClickSound.isset())
			buyClickSoundIdx = pGlobal->DropshipLoadout_BuyClickSound;

		if (pHouseTypeExt->DropshipLoadout_SellClickSound.isset())
			sellClickSoundIdx = pHouseTypeExt->DropshipLoadout_SellClickSound;
		else if (pGlobal && pGlobal->DropshipLoadout_SellClickSound.isset())
			sellClickSoundIdx = pGlobal->DropshipLoadout_SellClickSound;

		if (pHouseTypeExt->DropshipLoadout_ArrowsClickSound.isset())
			arrowsClickSoundIdx = pHouseTypeExt->DropshipLoadout_ArrowsClickSound;
		else if (pGlobal && pGlobal->DropshipLoadout_ArrowsClickSound.isset())
			arrowsClickSoundIdx = pGlobal->DropshipLoadout_ArrowsClickSound;

		startingDragDropSoundIdx = -1;
		if (pHouseTypeExt->DropshipLoadout_StartingDragDropSound.isset())
			startingDragDropSoundIdx = pHouseTypeExt->DropshipLoadout_StartingDragDropSound;
		else if (pGlobal && pGlobal->DropshipLoadout_StartingDragDropSound.isset())
			startingDragDropSoundIdx = pGlobal->DropshipLoadout_StartingDragDropSound;

		endingDragDropSoundIdx = -1;
		if (pHouseTypeExt->DropshipLoadout_EndingDragDropSound.isset())
			endingDragDropSoundIdx = pHouseTypeExt->DropshipLoadout_EndingDragDropSound;
		else if (pGlobal && pGlobal->DropshipLoadout_EndingDragDropSound.isset())
			endingDragDropSoundIdx = pGlobal->DropshipLoadout_EndingDragDropSound;

		bool usesPlayerWallet = false;
		long dropshipLoadout_InitialMoney = -1;
		if (this->startingMoney > 0)
		{
			dropshipLoadout_InitialMoney = this->startingMoney;
		}
		else if (this->startingMoney == 0)
		{
			if (pHouseTypeExt->DropshipLoadout_Money.isset())
				dropshipLoadout_InitialMoney = pHouseTypeExt->DropshipLoadout_Money;
			else if (pGlobal)
				dropshipLoadout_InitialMoney = pGlobal->DropshipLoadout_Money;
		}

		if (dropshipLoadout_InitialMoney < 0)
		{
			dropshipLoadout_InitialMoney = HouseClass::CurrentPlayer->Available_Money();
			usesPlayerWallet = true;
		}
		this->initialMoney = dropshipLoadout_InitialMoney;
		this->currentMoney = dropshipLoadout_InitialMoney;

		bool rememberPurchasedCargo = true;
		if (this->bRememberPurchasedCargo.isset())
			rememberPurchasedCargo = this->bRememberPurchasedCargo;
		else if (pHouseTypeExt->DropshipLoadout_RememberPurchasedCargo.isset())
			rememberPurchasedCargo = pHouseTypeExt->DropshipLoadout_RememberPurchasedCargo;
		else if (pGlobal)
			rememberPurchasedCargo = pGlobal->DropshipLoadout_RememberPurchasedCargo;

		// Initial units pool for scenario/country
		if (!pHouseExt->DropshipLoadout_InitialUnitsSet)
		{
			const std::vector<std::vector<TechnoTypeClass*>>* pInitialUnitsSrc = nullptr;
			if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
				pInitialUnitsSrc = &pHouseTypeExt->DropshipLoadout_InitialUnits;
			else if (pGlobal && !pGlobal->DropshipLoadout_InitialUnits.empty())
				pInitialUnitsSrc = &pGlobal->DropshipLoadout_InitialUnits;

			if (pInitialUnitsSrc)
				pHouseExt->DropshipLoadout_InitialUnits = *pInitialUnitsSrc;

			pHouseExt->DropshipLoadout_InitialUnitsSet = true;
		}

		std::vector<TechnoTypeClass*> initialUnitsRemaining;
		if (!this->bIgnoreFixedUnits)
		{
			for (size_t i = 0; i < pHouseExt->DropshipLoadout_InitialUnits.size() && i < (size_t)nStartingDropships; ++i)
			{
				for (auto pUnit : pHouseExt->DropshipLoadout_InitialUnits[i])
					if (pUnit)
						initialUnitsRemaining.push_back(pUnit);
			}
		}

		// Preload Cargo
		long totalPreloadedCost = 0;
		bool canPreload = false;

		if (this->bPreloadCargo)
		{
			if (pHouseExt->DropshipLoadout_Cargo.size() > 0)
			{
				// Collect all units in all saved cargos into a single pool
				std::vector<TechnoTypeClass*> cargoPool;
				for (size_t i = 0; i < pHouseExt->DropshipLoadout_Cargo.size() && i < (size_t)nStartingDropships; ++i)
				{
					for (auto pUnit : pHouseExt->DropshipLoadout_Cargo[i])
					{
						if (pUnit)
							cargoPool.push_back(pUnit);
					}
				}

				// Find fixed units source
				const std::vector<std::vector<TechnoTypeClass*>>* pFixedUnitsSrc = nullptr;
				if (!this->bIgnoreFixedUnits)
				{
					if (!pHouseTypeExt->DropshipLoadout_FixedUnits.empty())
						pFixedUnitsSrc = &pHouseTypeExt->DropshipLoadout_FixedUnits;
					else if (pGlobal && !pGlobal->DropshipLoadout_FixedUnits.empty())
						pFixedUnitsSrc = &pGlobal->DropshipLoadout_FixedUnits;
				}

				// Calculate total cost of custom units in saved cargo
				for (size_t i = 0; i < pHouseExt->DropshipLoadout_Cargo.size() && i < (size_t)nStartingDropships; i++)
				{
					const std::vector<TechnoTypeClass*>* pFixedList = nullptr;
					if (pFixedUnitsSrc && i < pFixedUnitsSrc->size())
						pFixedList = &((*pFixedUnitsSrc)[i]);

					std::vector<TechnoTypeClass*> fixedRemaining;
					if (pFixedList)
					{
						for (auto pUnit : *pFixedList)
							if (pUnit)
								fixedRemaining.push_back(pUnit);
					}

					for (auto pUnit : pHouseExt->DropshipLoadout_Cargo[i])
					{
						if (!pUnit)
							continue;

						auto it = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);
						if (it != fixedRemaining.end())
						{
							fixedRemaining.erase(it);
						}
						else
						{
							// Check if it's a free initial unit
							auto itInitial = std::find(initialUnitsRemaining.begin(), initialUnitsRemaining.end(), pUnit);
							if (itInitial != initialUnitsRemaining.end())
							{
								initialUnitsRemaining.erase(itInitial);
							}
							else
							{
								totalPreloadedCost += pUnit->Cost;
							}
						}
					}
				}

				if (usesPlayerWallet || rememberPurchasedCargo || currentMoney >= totalPreloadedCost)
				{
					canPreload = true;
				}
			}
		}

		if (canPreload)
		{
			if (!usesPlayerWallet && !rememberPurchasedCargo)
			{
				currentMoney -= totalPreloadedCost;
			}
		}
		else
		{
			this->bPreloadCargo = false;
		}

		// Available units lists
		std::vector<TechnoTypeClass*> allowableUnits;
		std::vector<int> allowableUnitMaximums;

		bool listFound = false;
		if (this->allowableUnitsIndex > 0)
		{
			if (pHouseTypeExt && this->allowableUnitsIndex < (int)pHouseTypeExt->DropshipLoadout_AllowableUnitsLists.size())
			{
				allowableUnits = pHouseTypeExt->DropshipLoadout_AllowableUnitsLists[this->allowableUnitsIndex];
				allowableUnitMaximums = pHouseTypeExt->DropshipLoadout_AllowableUnitMaximumsLists[this->allowableUnitsIndex];
				listFound = true;
			}
			else if (pGlobal && this->allowableUnitsIndex < (int)pGlobal->DropshipLoadout_AllowableUnitsLists.size())
			{
				allowableUnits = pGlobal->DropshipLoadout_AllowableUnitsLists[this->allowableUnitsIndex];
				allowableUnitMaximums = pGlobal->DropshipLoadout_AllowableUnitMaximumsLists[this->allowableUnitsIndex];
				listFound = true;
			}
		}
		else
		{
			if (pHouseTypeExt && !pHouseTypeExt->DropshipLoadout_AllowableUnitsLists.empty() && !pHouseTypeExt->DropshipLoadout_AllowableUnitsLists[0].empty())
			{
				allowableUnits = pHouseTypeExt->DropshipLoadout_AllowableUnitsLists[0];
				allowableUnitMaximums = pHouseTypeExt->DropshipLoadout_AllowableUnitMaximumsLists[0];
				listFound = true;
			}
			else if (pGlobal && !pGlobal->DropshipLoadout_AllowableUnitsLists.empty() && !pGlobal->DropshipLoadout_AllowableUnitsLists[0].empty())
			{
				allowableUnits = pGlobal->DropshipLoadout_AllowableUnitsLists[0];
				allowableUnitMaximums = pGlobal->DropshipLoadout_AllowableUnitMaximumsLists[0];
				listFound = true;
			}
			else
			{
				if (pHouseTypeExt && pHouseTypeExt->DropshipLoadout_AllowableUnits.size() > 0)
				{
					allowableUnits = pHouseTypeExt->DropshipLoadout_AllowableUnits;
					allowableUnitMaximums = pHouseTypeExt->DropshipLoadout_AllowableUnitMaximums;
					listFound = true;
				}
			}
		}

		if (!listFound && this->allowableUnitsIndex != 0)
		{
			Debug::Log("[DropshipLoadout] Warning: Requested allowable units list index %d not found, falling back to default rules/all units.\n", this->allowableUnitsIndex);
		}

		if (allowableUnits.size() > 0)
		{
			for (size_t i = 0; i < allowableUnits.size(); ++i)
			{
				int maximumCount = -1;

				if (i < allowableUnitMaximums.size())
				{
					maximumCount = allowableUnitMaximums[i];

					if (maximumCount == 0)
						continue;
				}

				availableUnitsMaximums.push_back(maximumCount);
				TechnoTypeClass* pType = allowableUnits[i];
				availableUnits.push_back(pType);
			}
		}
		else
		{
			for (const auto pType : TechnoTypeClass::Array)
			{
				if (pType && (pType->WhatAmI() == AbstractType::InfantryType || pType->WhatAmI() == AbstractType::UnitType))
				{
					availableUnits.push_back(pType);
					availableUnitsMaximums.push_back(-1);
				}
			}
		}

		// Ensure all initial units are in availableUnits so they can be bought back if removed
		const std::vector<std::vector<TechnoTypeClass*>>* pInitialUnitsSrc = nullptr;
		if (!this->bIgnoreFixedUnits)
		{
			if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
				pInitialUnitsSrc = &pHouseTypeExt->DropshipLoadout_InitialUnits;
			else if (pGlobal && !pGlobal->DropshipLoadout_InitialUnits.empty())
				pInitialUnitsSrc = &pGlobal->DropshipLoadout_InitialUnits;
		}

		if (pInitialUnitsSrc)
		{
			for (size_t i = 0; i < pInitialUnitsSrc->size() && i < (size_t)nStartingDropships; ++i)
			{
				for (auto pUnit : (*pInitialUnitsSrc)[i])
				{
					if (pUnit && std::find(availableUnits.begin(), availableUnits.end(), pUnit) == availableUnits.end())
					{
						availableUnits.push_back(pUnit);
						availableUnitsMaximums.push_back(-1);
					}
				}
			}
		}
	}
}

void DropshipLoadoutClass::CalculateLayout(DSurface* pSurface)
{
	if (!pSurface)
		return;

	const int cameoWidth = 60, cameoHeight = 48;
	int backgroundWidth = 0;
	int backgroundHeight = 0;

	if (dropshipLoadout_BackgroundPCX)
	{
		backgroundWidth = dropshipLoadout_BackgroundPCX->Width;
		backgroundHeight = dropshipLoadout_BackgroundPCX->Height;
	}
	else if (dropshipLoadout_Background)
	{
		backgroundWidth = dropshipLoadout_Background->Width;
		backgroundHeight = dropshipLoadout_Background->Height;
	}
	else
	{
		backgroundWidth = 640; // Fallback
		backgroundHeight = 480;
	}

	int backgroundX = (pSurface->GetWidth() - backgroundWidth) / 2;
	int backgroundY = (pSurface->GetHeight() - backgroundHeight) / 2;
	windowRectangle = { backgroundX, backgroundY, backgroundWidth, backgroundHeight };

	auto const pGlobal = ScenarioExt::Global();

	if (pSWTypeExt)
	{
		Point2D customUpArrowLocation = Point2D::Empty;
		if (pSWTypeExt->DropshipLoadout_UpArrowLocation.isset())
			customUpArrowLocation = pSWTypeExt->DropshipLoadout_UpArrowLocation;

		Point2D customDownArrowLocation = Point2D::Empty;
		if (pSWTypeExt->DropshipLoadout_DownArrowLocation.isset())
			customDownArrowLocation = pSWTypeExt->DropshipLoadout_DownArrowLocation;

		nSidebarCameos = 8;
		sidebarCameLocations.clear();

		if (pSWTypeExt->DropshipLoadout_SidebarCameosCount.isset() && pSWTypeExt->DropshipLoadout_SidebarCameosCount > 0)
		{
			nSidebarCameos = pSWTypeExt->DropshipLoadout_SidebarCameosCount;
			for (int i = 0; i < nSidebarCameos; ++i)
			{
				int cameoX = backgroundX + pSWTypeExt->DropshipLoadout_SidebarCameosLocations[i].X;
				int cameoY = backgroundY + pSWTypeExt->DropshipLoadout_SidebarCameosLocations[i].Y;
				sidebarCameLocations.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}
		else
		{
			for (int i = 0; i < nSidebarCameos; ++i)
			{
				int cameoX = backgroundX + 493 + 68 * (i % 2);
				int cameoY = backgroundY + 25 + 50 * (i / 2);
				sidebarCameLocations.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}

		int centerOfCameoColumns = 0;
		int arrowsY = 0;
		if (sidebarCameLocations.size() >= 2)
		{
			centerOfCameoColumns = sidebarCameLocations[0].X + sidebarCameLocations[0].Width + (sidebarCameLocations[1].X - (sidebarCameLocations[0].X + sidebarCameLocations[0].Width)) / 2;
			arrowsY = sidebarCameLocations.back().Y + sidebarCameLocations.back().Height + 6;
		}
		else
		{
			centerOfCameoColumns = backgroundX + 500;
			arrowsY = backgroundY + 400;
		}

		int dropshipLoadout_UpArrowWidth = 30;
		if (dropshipLoadout_UpArrowPCX)
			dropshipLoadout_UpArrowWidth = dropshipLoadout_UpArrowPCX->Width;
		else if (dropshipLoadout_UpArrow)
			dropshipLoadout_UpArrowWidth = dropshipLoadout_UpArrow->Width;

		int dropshipLoadout_UpArrowHeight = 30;
		if (dropshipLoadout_UpArrowPCX)
			dropshipLoadout_UpArrowHeight = dropshipLoadout_UpArrowPCX->Height;
		else if (dropshipLoadout_UpArrow)
			dropshipLoadout_UpArrowHeight = dropshipLoadout_UpArrow->Height;

		upArrowX = customUpArrowLocation != Point2D::Empty ? (backgroundX + customUpArrowLocation.X) : (centerOfCameoColumns - dropshipLoadout_UpArrowWidth);
		upArrowY = customUpArrowLocation != Point2D::Empty ? (backgroundY + customUpArrowLocation.Y) : arrowsY;
		upArrowLocation = { upArrowX, upArrowY, dropshipLoadout_UpArrowWidth, dropshipLoadout_UpArrowHeight };

		int dropshipLoadout_DownArrowWidth = 30;
		if (dropshipLoadout_DownArrowPCX)
			dropshipLoadout_DownArrowWidth = dropshipLoadout_DownArrowPCX->Width;
		else if (dropshipLoadout_DownArrow)
			dropshipLoadout_DownArrowWidth = dropshipLoadout_DownArrow->Width;

		int dropshipLoadout_DownArrowHeight = 30;
		if (dropshipLoadout_DownArrowPCX)
			dropshipLoadout_DownArrowHeight = dropshipLoadout_DownArrowPCX->Height;
		else if (dropshipLoadout_DownArrow)
			dropshipLoadout_DownArrowHeight = dropshipLoadout_DownArrow->Height;

		downArrowX = customDownArrowLocation != Point2D::Empty ? (backgroundX + customDownArrowLocation.X) : centerOfCameoColumns;
		downArrowY = customDownArrowLocation != Point2D::Empty ? (backgroundY + customDownArrowLocation.Y) : arrowsY;
		downArrowLocation = { downArrowX, downArrowY, dropshipLoadout_DownArrowWidth, dropshipLoadout_DownArrowHeight };

		dGreenLocation.clear();

		if (pSWTypeExt->DropshipLoadout_DGreenAnimationsCount.isset())
		{
			for (int i = 0; i < pSWTypeExt->DropshipLoadout_DGreenAnimationsCount; i++)
			{
				Point2D location = pSWTypeExt->DropshipLoadout_DGreenLocations[i];
				dGreenLocation.push_back({ backgroundX + location.X, backgroundY + location.Y, 0, 0 });
			}
		}
		else
		{
			int dGreenX = 371;
			int dGreenY = 10;

			for (int i = 0; i < 4; i++)
			{
				dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
				dGreenY += 50;
			}

			if (dropshipLoadout_DGreenListPCX.size() > 0)
			{
				for (size_t i = 4; i < dropshipLoadout_DGreenListPCX.size(); i++)
				{
					dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
					dGreenY += 50;
				}
			}
			else if (dropshipLoadout_DGreenList.size() > 0)
			{
				for (size_t i = 4; i < dropshipLoadout_DGreenList.size(); i++)
				{
					dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
					dGreenY += 50;
				}
			}
		}

		if (dropshipLoadout_DGreenListPCX.size() > 0)
		{
			for (size_t i = 0; i < dropshipLoadout_DGreenListPCX.size(); i++)
			{
				if (i < dGreenLocation.size() && dropshipLoadout_DGreenListPCX[i].size() > 0)
				{
					dGreenLocation[i].Width = dropshipLoadout_DGreenListPCX[i][0]->Width;
					dGreenLocation[i].Height = dropshipLoadout_DGreenListPCX[i][0]->Height;
				}
			}
		}
		else if (dropshipLoadout_DGreenList.size() > 0)
		{
			for (size_t i = 0; i < dropshipLoadout_DGreenList.size(); i++)
			{
				if (i < dGreenLocation.size() && dropshipLoadout_DGreenList[i] != nullptr)
				{
					dGreenLocation[i].Width = dropshipLoadout_DGreenList[i]->Width;
					dGreenLocation[i].Height = dropshipLoadout_DGreenList[i]->Height;
				}
			}
		}

		int dropshipLoadout_LoadoutWidth = 100;
		if (dropshipLoadout_LoadoutPCX.size() > 0)
			dropshipLoadout_LoadoutWidth = dropshipLoadout_LoadoutPCX[0]->Width;
		else if (dropshipLoadout_Loadout)
			dropshipLoadout_LoadoutWidth = dropshipLoadout_Loadout->Width;

		int dropshipLoadout_LoadoutHeight = 100;
		if (dropshipLoadout_LoadoutPCX.size() > 0)
			dropshipLoadout_LoadoutHeight = dropshipLoadout_LoadoutPCX[0]->Height;
		else if (dropshipLoadout_Loadout)
			dropshipLoadout_LoadoutHeight = dropshipLoadout_Loadout->Height;

		int dropshipLoadout_LoadoutX = pSWTypeExt->DropshipLoadout_LoadoutLocation.isset() ? pSWTypeExt->DropshipLoadout_LoadoutLocation.Get(Point2D::Empty).X : 45;
		int dropshipLoadout_LoadoutY = pSWTypeExt->DropshipLoadout_LoadoutLocation.isset() ? pSWTypeExt->DropshipLoadout_LoadoutLocation.Get(Point2D::Empty).Y : 2;

		loadoutLocation = { backgroundX + dropshipLoadout_LoadoutX, backgroundY + dropshipLoadout_LoadoutY, dropshipLoadout_LoadoutWidth, dropshipLoadout_LoadoutHeight };

		int dropshipLoadout_PilotLitWidth = 100;
		if (dropshipLoadout_PilotLitPCX.size() > 0)
			dropshipLoadout_PilotLitWidth = dropshipLoadout_PilotLitPCX[0]->Width;
		else if (dropshipLoadout_PilotLit)
			dropshipLoadout_PilotLitWidth = dropshipLoadout_PilotLit->Width;

		int dropshipLoadout_PilotLitHeight = 100;
		if (dropshipLoadout_PilotLitPCX.size() > 0)
			dropshipLoadout_PilotLitHeight = dropshipLoadout_PilotLitPCX[0]->Height;
		else if (dropshipLoadout_PilotLit)
			dropshipLoadout_PilotLitHeight = dropshipLoadout_PilotLit->Height;

		int dropshipLoadout_PilotLitX = pSWTypeExt->DropshipLoadout_PilotLitLocation.isset() ? pSWTypeExt->DropshipLoadout_PilotLitLocation.Get(Point2D::Empty).X : 284;
		int dropshipLoadout_PilotLitY = pSWTypeExt->DropshipLoadout_PilotLitLocation.isset() ? pSWTypeExt->DropshipLoadout_PilotLitLocation.Get(Point2D::Empty).Y : 151;

		pilotLitLocation = { backgroundX + dropshipLoadout_PilotLitX, backgroundY + dropshipLoadout_PilotLitY, dropshipLoadout_PilotLitWidth, dropshipLoadout_PilotLitHeight };

		nDropshipBayCameos = 5;
		dropshipBayCameLocations.clear();

		if (pSWTypeExt->DropshipLoadout_DropshipCameosCount.Get(0) > 0)
		{
			nDropshipBayCameos = pSWTypeExt->DropshipLoadout_DropshipCameosCount;

			auto& list = dropshipBayCameLocations.emplace_back();

			for (int j = 0; j < nDropshipBayCameos; j++)
			{
				int offsetX = 0;
				int offsetY = 0;

				if (j < (int)pSWTypeExt->DropshipLoadout_DropshipCameosLocations.size())
				{
					offsetX = pSWTypeExt->DropshipLoadout_DropshipCameosLocations[j].X;
					offsetY = pSWTypeExt->DropshipLoadout_DropshipCameosLocations[j].Y;
				}

				int cameoX = backgroundX + offsetX;
				int cameoY = backgroundY + offsetY;
				list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}
		else
		{
			int cameoX = backgroundX + 55;
			int cameoY = backgroundY + 69;
			auto& list = dropshipBayCameLocations.emplace_back();
			list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			list.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
			list.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
			list.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
			list.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);
		}
	}
	else
	{
		Point2D customUpArrowLocation = Point2D::Empty;
		if (pHouseTypeExt->DropshipLoadout_UpArrowLocation.isset())
			customUpArrowLocation = pHouseTypeExt->DropshipLoadout_UpArrowLocation;
		else if (pGlobal && pGlobal->DropshipLoadout_UpArrowLocation != Point2D::Empty)
			customUpArrowLocation = pGlobal->DropshipLoadout_UpArrowLocation;

		Point2D customDownArrowLocation = Point2D::Empty;
		if (pHouseTypeExt->DropshipLoadout_DownArrowLocation.isset())
			customDownArrowLocation = pHouseTypeExt->DropshipLoadout_DownArrowLocation;
		else if (pGlobal && pGlobal->DropshipLoadout_DownArrowLocation != Point2D::Empty)
			customDownArrowLocation = pGlobal->DropshipLoadout_DownArrowLocation;

		nSidebarCameos = 8;
		sidebarCameLocations.clear();

		if (pHouseTypeExt->DropshipLoadout_SidebarCameosCount.isset() && pHouseTypeExt->DropshipLoadout_SidebarCameosCount > 0)
		{
			nSidebarCameos = pHouseTypeExt->DropshipLoadout_SidebarCameosCount;
			for (int i = 0; i < nSidebarCameos; ++i)
			{
				int cameoX = backgroundX + pHouseTypeExt->DropshipLoadout_SidebarCameoLocations[i].X;
				int cameoY = backgroundY + pHouseTypeExt->DropshipLoadout_SidebarCameoLocations[i].Y;
				sidebarCameLocations.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}
		else if (pGlobal && pGlobal->DropshipLoadout_SidebarCameosCount > 0)
		{
			nSidebarCameos = pGlobal->DropshipLoadout_SidebarCameosCount;
			for (int i = 0; i < nSidebarCameos; ++i)
			{
				int cameoX = backgroundX + pGlobal->DropshipLoadout_SidebarCameoLocations[i].X;
				int cameoY = backgroundY + pGlobal->DropshipLoadout_SidebarCameoLocations[i].Y;
				sidebarCameLocations.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}
		else
		{
			for (int i = 0; i < nSidebarCameos; ++i)
			{
				int cameoX = backgroundX + 493 + 68 * (i % 2);
				int cameoY = backgroundY + 25 + 50 * (i / 2);
				sidebarCameLocations.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
			}
		}

		int centerOfCameoColumns = 0;
		int arrowsY = 0;
		if (sidebarCameLocations.size() >= 2)
		{
			centerOfCameoColumns = sidebarCameLocations[0].X + sidebarCameLocations[0].Width + (sidebarCameLocations[1].X - (sidebarCameLocations[0].X + sidebarCameLocations[0].Width)) / 2;
			arrowsY = sidebarCameLocations.back().Y + sidebarCameLocations.back().Height + 6;
		}
		else
		{
			centerOfCameoColumns = backgroundX + 500;
			arrowsY = backgroundY + 400;
		}

		int dropshipLoadout_UpArrowWidth = 30;
		if (dropshipLoadout_UpArrowPCX)
			dropshipLoadout_UpArrowWidth = dropshipLoadout_UpArrowPCX->Width;
		else if (dropshipLoadout_UpArrow)
			dropshipLoadout_UpArrowWidth = dropshipLoadout_UpArrow->Width;

		int dropshipLoadout_UpArrowHeight = 30;
		if (dropshipLoadout_UpArrowPCX)
			dropshipLoadout_UpArrowHeight = dropshipLoadout_UpArrowPCX->Height;
		else if (dropshipLoadout_UpArrow)
			dropshipLoadout_UpArrowHeight = dropshipLoadout_UpArrow->Height;

		upArrowX = customUpArrowLocation != Point2D::Empty ? (backgroundX + customUpArrowLocation.X) : (centerOfCameoColumns - dropshipLoadout_UpArrowWidth);
		upArrowY = customUpArrowLocation != Point2D::Empty ? (backgroundY + customUpArrowLocation.Y) : arrowsY;
		upArrowLocation = { upArrowX, upArrowY, dropshipLoadout_UpArrowWidth, dropshipLoadout_UpArrowHeight };

		int dropshipLoadout_DownArrowWidth = 30;
		if (dropshipLoadout_DownArrowPCX)
			dropshipLoadout_DownArrowWidth = dropshipLoadout_DownArrowPCX->Width;
		else if (dropshipLoadout_DownArrow)
			dropshipLoadout_DownArrowWidth = dropshipLoadout_DownArrow->Width;

		int dropshipLoadout_DownArrowHeight = 30;
		if (dropshipLoadout_DownArrowPCX)
			dropshipLoadout_DownArrowHeight = dropshipLoadout_DownArrowPCX->Height;
		else if (dropshipLoadout_DownArrow)
			dropshipLoadout_DownArrowHeight = dropshipLoadout_DownArrow->Height;

		downArrowX = customDownArrowLocation != Point2D::Empty ? (backgroundX + customDownArrowLocation.X) : centerOfCameoColumns;
		downArrowY = customDownArrowLocation != Point2D::Empty ? (backgroundY + customDownArrowLocation.Y) : arrowsY;
		downArrowLocation = { downArrowX, downArrowY, dropshipLoadout_DownArrowWidth, dropshipLoadout_DownArrowHeight };

		dGreenLocation.clear();

		if (pHouseTypeExt->DropshipLoadout_DGreenAnimationsCount.isset())
		{
			for (int i = 0; i < pHouseTypeExt->DropshipLoadout_DGreenAnimationsCount; i++)
			{
				Point2D location = pHouseTypeExt->DropshipLoadout_DGreenLocations[i];
				dGreenLocation.push_back({ backgroundX + location.X, backgroundY + location.Y, 0, 0 });
			}
		}
		else if (pGlobal && pGlobal->DropshipLoadout_DGreenAnimationsCount)
		{
			for (int i = 0; i < pGlobal->DropshipLoadout_DGreenAnimationsCount; i++)
			{
				Point2D location = pGlobal->DropshipLoadout_DGreenLocations[i];
				dGreenLocation.push_back({ backgroundX + location.X, backgroundY + location.Y, 0, 0 });
			}
		}
		else
		{
			int dGreenX = 371;
			int dGreenY = 10;

			for (int i = 0; i < 4; i++)
			{
				dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
				dGreenY += 50;
			}

			if (dropshipLoadout_DGreenListPCX.size() > 0)
			{
				for (size_t i = 4; i < dropshipLoadout_DGreenListPCX.size(); i++)
				{
					dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
					dGreenY += 50;
				}
			}
			else if (dropshipLoadout_DGreenList.size() > 0)
			{
				for (size_t i = 4; i < dropshipLoadout_DGreenList.size(); i++)
				{
					dGreenLocation.push_back({ backgroundX + dGreenX, backgroundY + dGreenY, 0, 0 });
					dGreenY += 50;
				}
			}
		}

		if (dropshipLoadout_DGreenListPCX.size() > 0)
		{
			for (size_t i = 0; i < dropshipLoadout_DGreenListPCX.size(); i++)
			{
				if (i < dGreenLocation.size() && dropshipLoadout_DGreenListPCX[i].size() > 0)
				{
					dGreenLocation[i].Width = dropshipLoadout_DGreenListPCX[i][0]->Width;
					dGreenLocation[i].Height = dropshipLoadout_DGreenListPCX[i][0]->Height;
				}
			}
		}
		else if (dropshipLoadout_DGreenList.size() > 0)
		{
			for (size_t i = 0; i < dropshipLoadout_DGreenList.size(); i++)
			{
				if (i < dGreenLocation.size() && dropshipLoadout_DGreenList[i] != nullptr)
				{
					dGreenLocation[i].Width = dropshipLoadout_DGreenList[i]->Width;
					dGreenLocation[i].Height = dropshipLoadout_DGreenList[i]->Height;
				}
			}
		}

		int dropshipLoadout_LoadoutWidth = 100;
		if (dropshipLoadout_LoadoutPCX.size() > 0)
			dropshipLoadout_LoadoutWidth = dropshipLoadout_LoadoutPCX[0]->Width;
		else if (dropshipLoadout_Loadout)
			dropshipLoadout_LoadoutWidth = dropshipLoadout_Loadout->Width;

		int dropshipLoadout_LoadoutHeight = 100;
		if (dropshipLoadout_LoadoutPCX.size() > 0)
			dropshipLoadout_LoadoutHeight = dropshipLoadout_LoadoutPCX[0]->Height;
		else if (dropshipLoadout_Loadout)
			dropshipLoadout_LoadoutHeight = dropshipLoadout_Loadout->Height;

		int dropshipLoadout_LoadoutX = 45;
		int dropshipLoadout_LoadoutY = 2;

		if (pHouseTypeExt->DropshipLoadout_LoadoutLocation.isset())
		{
			dropshipLoadout_LoadoutX = pHouseTypeExt->DropshipLoadout_LoadoutLocation.Get(Point2D::Empty).X;
			dropshipLoadout_LoadoutY = pHouseTypeExt->DropshipLoadout_LoadoutLocation.Get(Point2D::Empty).Y;
		}
		else if (pGlobal && pGlobal->DropshipLoadout_LoadoutLocation != Point2D::Empty)
		{
			dropshipLoadout_LoadoutX = pGlobal->DropshipLoadout_LoadoutLocation.X;
			dropshipLoadout_LoadoutY = pGlobal->DropshipLoadout_LoadoutLocation.Y;
		}

		loadoutLocation = { backgroundX + dropshipLoadout_LoadoutX, backgroundY + dropshipLoadout_LoadoutY, dropshipLoadout_LoadoutWidth, dropshipLoadout_LoadoutHeight };

		int dropshipLoadout_PilotLitWidth = 100;
		if (dropshipLoadout_PilotLitPCX.size() > 0)
			dropshipLoadout_PilotLitWidth = dropshipLoadout_PilotLitPCX[0]->Width;
		else if (dropshipLoadout_PilotLit)
			dropshipLoadout_PilotLitWidth = dropshipLoadout_PilotLit->Width;

		int dropshipLoadout_PilotLitHeight = 100;
		if (dropshipLoadout_PilotLitPCX.size() > 0)
			dropshipLoadout_PilotLitHeight = dropshipLoadout_PilotLitPCX[0]->Height;
		else if (dropshipLoadout_PilotLit)
			dropshipLoadout_PilotLitHeight = dropshipLoadout_PilotLit->Height;

		int dropshipLoadout_PilotLitX = 284;
		int dropshipLoadout_PilotLitY = 151;

		if (pHouseTypeExt->DropshipLoadout_PilotLitLocation.isset())
		{
			dropshipLoadout_PilotLitX = pHouseTypeExt->DropshipLoadout_PilotLitLocation.Get(Point2D::Empty).X;
			dropshipLoadout_PilotLitY = pHouseTypeExt->DropshipLoadout_PilotLitLocation.Get(Point2D::Empty).Y;
		}
		else if (pGlobal && pGlobal->DropshipLoadout_PilotLitLocation != Point2D::Empty)
		{
			dropshipLoadout_PilotLitX = pGlobal->DropshipLoadout_PilotLitLocation.X;
			dropshipLoadout_PilotLitY = pGlobal->DropshipLoadout_PilotLitLocation.Y;
		}

		pilotLitLocation = { backgroundX + dropshipLoadout_PilotLitX, backgroundY + dropshipLoadout_PilotLitY, dropshipLoadout_PilotLitWidth, dropshipLoadout_PilotLitHeight };

		nDropshipBayCameos = 5;
		dropshipBayCameLocations.clear();

		if (!pHouseTypeExt->DropshipLoadout_DropshipCameoLocations.empty())
		{
			for (int i = 0; i < nStartingDropships; i++)
			{
				auto& list = dropshipBayCameLocations.emplace_back();
				int currentCameosCount = pHouseTypeExt->DropshipLoadout_DropshipCameosCount.Get(0) > 0 ? pHouseTypeExt->DropshipLoadout_DropshipCameosCount.Get(0) : 5;

				if (i < (int)pHouseTypeExt->DropshipLoadout_DropshipCameoLocations.size())
					currentCameosCount = (int)pHouseTypeExt->DropshipLoadout_DropshipCameoLocations[i].size();

				for (int j = 0; j < currentCameosCount; j++)
				{
					int offsetX = 0;
					int offsetY = 0;
					if (i < (int)pHouseTypeExt->DropshipLoadout_DropshipCameoLocations.size())
					{
						auto& row = pHouseTypeExt->DropshipLoadout_DropshipCameoLocations[i];
						if (j < (int)row.size())
						{
							offsetX = row[j].X;
							offsetY = row[j].Y;
						}
					}
					int cameoX = backgroundX + offsetX;
					int cameoY = backgroundY + offsetY;
					list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				}
			}
		}
		else if (pGlobal && !pGlobal->DropshipLoadout_DropshipCameoLocations.empty())
		{
			for (int i = 0; i < nStartingDropships; i++)
			{
				auto& list = dropshipBayCameLocations.emplace_back();
				int currentCameosCount = pGlobal->DropshipLoadout_DropshipCameosCount > 0 ? pGlobal->DropshipLoadout_DropshipCameosCount : 5;

				if (i < (int)pGlobal->DropshipLoadout_DropshipCameoLocations.size())
					currentCameosCount = (int)pGlobal->DropshipLoadout_DropshipCameoLocations[i].size();

				for (int j = 0; j < currentCameosCount; j++)
				{
					int offsetX = 0;
					int offsetY = 0;
					if (i < (int)pGlobal->DropshipLoadout_DropshipCameoLocations.size())
					{
						auto& row = pGlobal->DropshipLoadout_DropshipCameoLocations[i];
						if (j < (int)row.size())
						{
							offsetX = row[j].X;
							offsetY = row[j].Y;
						}
					}
					int cameoX = backgroundX + offsetX;
					int cameoY = backgroundY + offsetY;
					list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				}
			}
		}
		else
		{
			if (nStartingDropships == 1 || nStartingDropships == 2)
			{
				int cameoX = backgroundX + 55;
				int cameoY = backgroundY + 69;
				auto& list = dropshipBayCameLocations.emplace_back();
				list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
				list.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);
			}
			if (nStartingDropships == 2)
			{
				int cameoX = backgroundX + 55;
				int cameoY = backgroundY + 209;
				auto& list = dropshipBayCameLocations.emplace_back();
				list.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
				list.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
				list.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);
			}
			if (nStartingDropships == 3)
			{
				int cameoX = backgroundX + 55;
				int cameoY = backgroundY + 39;
				auto& list1 = dropshipBayCameLocations.emplace_back();
				list1.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				list1.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
				list1.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
				list1.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
				list1.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);

				cameoY += 120;
				auto& list2 = dropshipBayCameLocations.emplace_back();
				list2.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				list2.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
				list2.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
				list2.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
				list2.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);

				cameoY += 120;
				auto& list3 = dropshipBayCameLocations.emplace_back();
				list3.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
				list3.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
				list3.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
				list3.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
				list3.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);
			}
			// What if starting dropships is greater than 3? Or 0?
			if (dropshipBayCameLocations.size() < (size_t)nStartingDropships)
			{
				// Generate generic placements so it doesn't crash
				for (int i = (int)dropshipBayCameLocations.size(); i < nStartingDropships; i++)
				{
					int cameoX = backgroundX + 55;
					int cameoY = backgroundY + 39 + i * 120;
					auto& genericList = dropshipBayCameLocations.emplace_back();
					genericList.emplace_back(cameoX, cameoY, cameoWidth, cameoHeight);
					genericList.emplace_back(cameoX + 66, cameoY, cameoWidth, cameoHeight);
					genericList.emplace_back(cameoX, cameoY + 50, cameoWidth, cameoHeight);
					genericList.emplace_back(cameoX + 66, cameoY + 50, cameoWidth, cameoHeight);
					genericList.emplace_back(cameoX + 132, cameoY + 50, cameoWidth, cameoHeight);
				}
			}
		}
	}

	// Update nDropshipBayCameos to the maximum slot count among all dropships
	int maxCameos = 0;
	for (auto const& list : dropshipBayCameLocations)
	{
		if ((int)list.size() > maxCameos)
			maxCameos = (int)list.size();
	}
	nDropshipBayCameos = maxCameos;

	nDropshipBayTotalSlots = nStartingDropships * nDropshipBayCameos;
}

void DropshipLoadoutClass::CreateControls()
{
	const int cameoWidth = 60, cameoHeight = 48;
	buttonsList.clear();

	int btn_ScrollUp_ID = 100;
	ShapeButtonClass* btn_ScrollUp = CreateShapeButton(
		btn_ScrollUp_ID,
		0, 0,
		upArrowLocation.Width, upArrowLocation.Height,
		true
	);

	if (btn_ScrollUp)
	{
		btn_ScrollUp->SetPosition(upArrowLocation.X, upArrowLocation.Y);
		btn_ScrollUp->SetDimension(upArrowLocation.Width, upArrowLocation.Height);
		btn_ScrollUp->DrawPosition.X = upArrowX;
		btn_ScrollUp->DrawPosition.Y = upArrowY;
		buttonsList.push_back(btn_ScrollUp);
		commandManager = btn_ScrollUp;
	}

	int btn_ScrollDown_ID = 101;
	ShapeButtonClass* btn_ScrollDown = CreateShapeButton(
		btn_ScrollDown_ID,
		0, 0,
		downArrowLocation.Width, downArrowLocation.Height,
		true
	);

	if (btn_ScrollDown)
	{
		btn_ScrollDown->SetPosition(downArrowLocation.X, downArrowLocation.Y);
		btn_ScrollDown->SetDimension(downArrowLocation.Width, downArrowLocation.Height);
		btn_ScrollDown->DrawPosition.X = downArrowX;
		btn_ScrollDown->DrawPosition.Y = downArrowY;
		buttonsList.push_back(btn_ScrollDown);

		if (commandManager)
			commandManager->Add(*btn_ScrollDown);
	}

	int btn_BasicDropshipCameo_ID = 200;
	int newID = btn_BasicDropshipCameo_ID;
	dropshipBayChosenUnitsLists.clear();
	dropshipBayFixedUnitsLists.clear();
	dropshipBayChosenUnitsCount.clear();

	auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer);

	if (pSWTypeExt)
	{
		const std::vector<TechnoTypeClass*>* pFixedList = nullptr;

		if (!bIgnoreFixedUnits && !pSWTypeExt->DropshipLoadout_FixedUnits.empty())
			pFixedList = &pSWTypeExt->DropshipLoadout_FixedUnits;

		if (!pHouseExt->DropshipLoadout_SWInitialUnitsSet)
		{
			pHouseExt->DropshipLoadout_SWInitialUnits = pSWTypeExt->DropshipLoadout_InitialUnits;
			pHouseExt->DropshipLoadout_SWInitialUnitsSet = true;
		}

		const std::vector<TechnoTypeClass*>* pInitialList = nullptr;

		if (!bIgnoreFixedUnits && !pHouseExt->DropshipLoadout_SWInitialUnits.empty())
			pInitialList = &pHouseExt->DropshipLoadout_SWInitialUnits;

		bool hasSavedCargo = !pHouseExt->DropshipLoadout_SWCargo.empty();
		bool usePreload = bPreloadCargo && hasSavedCargo;

		dropshipBayChosenUnitsLists.emplace_back();
		dropshipBayFixedUnitsLists.emplace_back();

		std::vector<TechnoTypeClass*> fixedRemaining;

		if (pFixedList)
		{
			for (auto pUnit : *pFixedList)
				if (pUnit)
					fixedRemaining.push_back(pUnit);
		}

		std::vector<TechnoTypeClass*> initialUnitsRemaining;

		if (pInitialList)
		{
			for (auto pUnit : *pInitialList)
				if (pUnit)
					initialUnitsRemaining.push_back(pUnit);
		}

		for (int j = 0; j < nDropshipBayCameos; j++)
		{
			if (dropshipBayCameLocations.empty() || j >= (int)dropshipBayCameLocations[0].size())
				continue;

			ShapeButtonClass* newButton = CreateShapeButton(
				newID,
				0, 0,
				cameoWidth, cameoHeight,
				true
			);

			if (newButton)
			{
				newButton->SetPosition(dropshipBayCameLocations[0][j].X, dropshipBayCameLocations[0][j].Y);
				newButton->SetDimension(cameoWidth, cameoHeight);
				newButton->DrawPosition.X = dropshipBayCameLocations[0][j].X;
				newButton->DrawPosition.Y = dropshipBayCameLocations[0][j].Y;
				buttonsList.push_back(newButton);

				if (commandManager)
					commandManager->Add(*newButton);
			}

			TechnoTypeClass* pUnit = nullptr;
			bool isFixed = false;

			if (usePreload)
			{
				if (j < (int)pHouseExt->DropshipLoadout_SWCargo.size())
				{
					pUnit = pHouseExt->DropshipLoadout_SWCargo[j];
					if (pUnit)
					{
						auto it = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);
						if (it != fixedRemaining.end())
						{
							isFixed = true;
							fixedRemaining.erase(it);
						}
						else
						{
							isFixed = false;
							auto itInitial = std::find(initialUnitsRemaining.begin(), initialUnitsRemaining.end(), pUnit);

							if (itInitial != initialUnitsRemaining.end())
								initialUnitsRemaining.erase(itInitial);

							dropshipBayChosenUnitsCount[pUnit]++;
						}
					}
				}
			}
			else
			{
				int nFixed = pFixedList ? (int)pFixedList->size() : 0;
				int nInitial = pInitialList ? (int)pInitialList->size() : 0;

				if (pFixedList && j < nFixed)
				{
					pUnit = (*pFixedList)[j];

					if (pUnit)
						isFixed = true;
				}
				else if (pInitialList && (j - nFixed) >= 0 && (j - nFixed) < nInitial)
				{
					pUnit = (*pInitialList)[j - nFixed];

					if (pUnit)
					{
						isFixed = false;
						dropshipBayChosenUnitsCount[pUnit]++;
					}
				}
			}

			dropshipBayChosenUnitsLists[0].push_back(pUnit);
			dropshipBayFixedUnitsLists[0].push_back(isFixed);
			newID++;
		}
	}
	else
	{
		const std::vector<std::vector<TechnoTypeClass*>>* pFixedUnitsSrc = nullptr;
		if (!bIgnoreFixedUnits)
		{
			if (!pHouseTypeExt->DropshipLoadout_FixedUnits.empty())
				pFixedUnitsSrc = &pHouseTypeExt->DropshipLoadout_FixedUnits;
			else if (ScenarioExt::Global() && !ScenarioExt::Global()->DropshipLoadout_FixedUnits.empty())
				pFixedUnitsSrc = &ScenarioExt::Global()->DropshipLoadout_FixedUnits;
		}

		if (!pHouseExt->DropshipLoadout_InitialUnitsSet && !bIgnoreFixedUnits)
		{
			if (!pHouseTypeExt->DropshipLoadout_InitialUnits.empty())
				pHouseExt->DropshipLoadout_InitialUnits = pHouseTypeExt->DropshipLoadout_InitialUnits;
			else if (ScenarioExt::Global() && !ScenarioExt::Global()->DropshipLoadout_InitialUnits.empty())
				pHouseExt->DropshipLoadout_InitialUnits = ScenarioExt::Global()->DropshipLoadout_InitialUnits;

			pHouseExt->DropshipLoadout_InitialUnitsSet = true;
		}

		const std::vector<std::vector<TechnoTypeClass*>>* pInitialUnitsSrc = nullptr;

		if (!bIgnoreFixedUnits && !pHouseExt->DropshipLoadout_InitialUnits.empty())
			pInitialUnitsSrc = &pHouseExt->DropshipLoadout_InitialUnits;

		std::vector<TechnoTypeClass*> initialUnitsRemaining;

		if (!bIgnoreFixedUnits && pInitialUnitsSrc)
		{
			for (size_t i = 0; i < pInitialUnitsSrc->size() && i < (size_t)nStartingDropships; ++i)
			{
				for (auto pUnit : (*pInitialUnitsSrc)[i])
				{
					if (pUnit)
						initialUnitsRemaining.push_back(pUnit);
				}
			}
		}

		bool hasSavedCargo = (pHouseExt->DropshipLoadout_Cargo.size() > 0);

		for (int i = 0; i < nStartingDropships; i++)
		{
			dropshipBayChosenUnitsLists.emplace_back();
			dropshipBayFixedUnitsLists.emplace_back();

			if (i >= (int)dropshipBayCameLocations.size())
				continue;

			const std::vector<TechnoTypeClass*>* pFixedList = nullptr;

			if (pFixedUnitsSrc && i < (int)pFixedUnitsSrc->size())
				pFixedList = &((*pFixedUnitsSrc)[i]);

			const std::vector<TechnoTypeClass*>* pInitialList = nullptr;

			if (pInitialUnitsSrc && i < (int)pInitialUnitsSrc->size())
				pInitialList = &((*pInitialUnitsSrc)[i]);

			std::vector<TechnoTypeClass*> fixedRemaining;
			if (pFixedList)
			{
				for (auto pUnit : *pFixedList)
				{
					if (pUnit)
						fixedRemaining.push_back(pUnit);
				}
			}

			bool usePreload = bPreloadCargo && hasSavedCargo && i < (int)pHouseExt->DropshipLoadout_Cargo.size();

			for (int j = 0; j < nDropshipBayCameos; j++)
			{
				if (j >= (int)dropshipBayCameLocations[i].size())
					continue;

				int buttonID = btn_BasicDropshipCameo_ID + i * nDropshipBayCameos + j;
				ShapeButtonClass* newButton = CreateShapeButton(
					buttonID,
					0, 0,
					cameoWidth, cameoHeight,
					true
				);

				if (newButton)
				{
					newButton->SetPosition(dropshipBayCameLocations[i][j].X, dropshipBayCameLocations[i][j].Y);
					newButton->SetDimension(cameoWidth, cameoHeight);
					newButton->DrawPosition.X = dropshipBayCameLocations[i][j].X;
					newButton->DrawPosition.Y = dropshipBayCameLocations[i][j].Y;
					buttonsList.push_back(newButton);

					if (commandManager)
						commandManager->Add(*newButton);
				}

				TechnoTypeClass* pUnit = nullptr;
				bool isFixed = false;

				if (usePreload)
				{
					if (j < (int)pHouseExt->DropshipLoadout_Cargo[i].size())
					{
						pUnit = pHouseExt->DropshipLoadout_Cargo[i][j];
						if (pUnit)
						{
							auto it = std::find(fixedRemaining.begin(), fixedRemaining.end(), pUnit);
							if (it != fixedRemaining.end())
							{
								isFixed = true;
								fixedRemaining.erase(it);
							}
							else
							{
								isFixed = false;
								auto itInitial = std::find(initialUnitsRemaining.begin(), initialUnitsRemaining.end(), pUnit);

								if (itInitial != initialUnitsRemaining.end())
									initialUnitsRemaining.erase(itInitial);

								dropshipBayChosenUnitsCount[pUnit]++;
							}
						}
					}
				}
				else
				{
					int nFixed = pFixedList ? (int)pFixedList->size() : 0;
					int nInitial = pInitialList ? (int)pInitialList->size() : 0;

					if (pFixedList && j < nFixed)
					{
						pUnit = (*pFixedList)[j];

						if (pUnit)
							isFixed = true;
					}
					else if (pInitialList && (j - nFixed) >= 0 && (j - nFixed) < nInitial)
					{
						pUnit = (*pInitialList)[j - nFixed];

						if (pUnit)
						{
							isFixed = false;
							dropshipBayChosenUnitsCount[pUnit]++;
						}
					}
				}

				dropshipBayChosenUnitsLists[i].push_back(pUnit);
				dropshipBayFixedUnitsLists[i].push_back(isFixed);
			}
		}
	}

	int btn_BasicSidebarCameo_ID = 300;
	for (int i = 0; i < nSidebarCameos; i++)
	{
		if (i >= (int)sidebarCameLocations.size())
			continue;

		int sID = btn_BasicSidebarCameo_ID + i;
		ShapeButtonClass* newButton = CreateShapeButton(
			sID,
			0, 0,
			cameoWidth, cameoHeight,
			true
		);

		if (newButton)
		{
			newButton->SetPosition(sidebarCameLocations[i].X, sidebarCameLocations[i].Y);
			newButton->SetDimension(cameoWidth, cameoHeight);
			newButton->DrawPosition.X = sidebarCameLocations[i].X;
			newButton->DrawPosition.Y = sidebarCameLocations[i].Y;
			buttonsList.push_back(newButton);

			if (commandManager)
				commandManager->Add(*newButton);
		}
	}
}

void DropshipLoadoutClass::Run()
{
	DSurface* pSurface = DSurface::Hidden;
	if (!pSurface)
		return;

	pSurface->Fill(0);

	CalculateLayout(pSurface);
	CreateControls();

	int voiceEva = -1;

	if (pHouseTypeExt->DropshipLoadout_StartEVA.isset())
		voiceEva = pHouseTypeExt->DropshipLoadout_StartEVA.Get(-1);
	else if (ScenarioExt::Global())
		voiceEva = ScenarioExt::Global()->DropshipLoadout_StartEVA.Get(-1);

	if (voiceEva >= 0)
		VoxClass::PlayIndex(voiceEva);

	int themeIdx = -1;

	if (pHouseTypeExt->DropshipLoadout_Theme.isset())
		themeIdx = pHouseTypeExt->DropshipLoadout_Theme;
	else if (ScenarioExt::Global())
		themeIdx = ScenarioExt::Global()->DropshipLoadout_Theme;

	if (themeIdx == -1)
		ThemeClass::Instance.Stop(true);
	else
		ThemeClass::Instance.Play(themeIdx);

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

	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);

	if (commandManager)
		commandManager->TurnOn();

	if (dropshipLoadout_LoadoutPCX.size() > 0)
		loadoutTotalFrames = (int)dropshipLoadout_LoadoutPCX.size() - 1;
	else if (dropshipLoadout_Loadout)
		loadoutTotalFrames = dropshipLoadout_Loadout->Frames;
	else
		loadoutTotalFrames = 0;

	if (dropshipLoadout_PilotLitPCX.size() > 0)
		pilotLitTotalFrames = (int)dropshipLoadout_PilotLitPCX.size() - 1;
	else if (dropshipLoadout_PilotLit)
		pilotLitTotalFrames = dropshipLoadout_PilotLit->Frames;
	else
		pilotLitTotalFrames = 0;

	animTimer_DelayedStartValue_Loadout = ScenarioClass::Instance->Random(0, 0);
	animTimer_DelayedStartValue_PilotLit = ScenarioClass::Instance->Random(100, 300);

	animTimer_DelayedStartTimer_Loadout.Start(animTimer_DelayedStartValue_Loadout);
	animTimer_DelayedStartTimer_PilotLit.Start(animTimer_DelayedStartValue_PilotLit);
	animTimer_UpdateFrameTimer_Loadout.Start(loadoutFrameDelay);
	animTimer_UpdateFrameTimer_PilotLit.Start(pilotLitFrameDelay);

	if (sidebarRowAnimationIndex >= 0)
	{
		if (dropshipLoadout_DGreenListPCX.size() > 0)
		{
			if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenListPCX.size())
				sidebarRowAnimationTotalFrames = (int)dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex].size() - 1;
		}
		else if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenList.size() && dropshipLoadout_DGreenList[sidebarRowAnimationIndex] != nullptr)
		{
			sidebarRowAnimationTotalFrames = dropshipLoadout_DGreenList[sidebarRowAnimationIndex]->Frames;
		}
	}

	pressedSpaceKey = false;
	repaintAll = true;
	bDropshipLoadoutActive = true;
	pendingScrolls = 0;
	pHoveredUnitType = nullptr;
	hoveredDropshipIdx = -1;
	hoveredSlotIdx = -1;

	HWND hGameWnd = Game::hWnd;
	if (hGameWnd)
	{
		SetFocus(hGameWnd);
		SetActiveWindow(hGameWnd);
		SetForegroundWindow(hGameWnd);
	}

	// Flush keyboard messages from the queue to clear any leftovers
	MSG flushMsg;
	while (PeekMessage(&flushMsg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
	{
		// Discard
	}

	bool wasLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool wasRButtonDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	bool wasSpaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
	bool wasEscDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

	bool ignoreSpaceUntilReleased = wasSpaceDown;
	bool ignoreEscUntilReleased = wasEscDown;

	while (!pressedSpaceKey)
	{
		if (ignoreSpaceUntilReleased && !(GetAsyncKeyState(VK_SPACE) & 0x8000))
			ignoreSpaceUntilReleased = false;

		if (ignoreEscUntilReleased && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
			ignoreEscUntilReleased = false;

		int command = 0;

		// Check Win32 message queue directly BEFORE Game::CallBack() pumps them
		MSG msg;
		while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
		{
			bool handled = false;
			if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
			{
				if (msg.wParam == VK_SPACE)
				{
					if (!ignoreSpaceUntilReleased)
						command = VK_SPACE;

					handled = true;
				}
				else if (msg.wParam == VK_ESCAPE)
				{
					if (!ignoreEscUntilReleased)
						command = VK_ESCAPE;

					handled = true;
				}
				else if (msg.wParam == VK_UP)
				{
					command = VK_UP;
					handled = true;
				}
				else if (msg.wParam == VK_DOWN)
				{
					command = VK_DOWN;
					handled = true;
				}
			}

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Game::CallBack();
		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);

		if (command == 0 && commandManager)
			command = commandManager->Input();

		bool isLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		bool isRButtonDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		bool isSpaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
		bool isEscDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

		if (command == 0)
		{
			if (isLButtonDown && !wasLButtonDown)
				command = 1;
			else if (isRButtonDown && !wasRButtonDown)
				command = 2;
		}

		if (isSpaceDown && !wasSpaceDown)
		{
			if (!ignoreSpaceUntilReleased)
				command = VK_SPACE;
		}
		else if (isEscDown && !wasEscDown)
		{
			if (!ignoreEscUntilReleased)
				command = VK_ESCAPE;
		}

		wasLButtonDown = isLButtonDown;
		wasRButtonDown = isRButtonDown;
		wasSpaceDown = isSpaceDown;
		wasEscDown = isEscDown;

		int buttonID = -1;

		if (WWMouseClass::Instance)
		{
			RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;

			for (auto button : buttonsList)
			{
				if (button && mouseRect.X >= button->X
					&& mouseRect.X <= (button->X + button->Width)
					&& mouseRect.Y >= button->Y
					&& mouseRect.Y <= (button->Y + button->Height))
				{
					buttonID = button->ID;
					break;
				}
			}
		}

		if (bDragPending || bIsDragging)
		{
			Point2D mousePos = { 0, 0 };

			if (WWMouseClass::Instance)
			{
				mousePos.X = WWMouseClass::Instance->GetX();
				mousePos.Y = WWMouseClass::Instance->GetY();
			}

			// Check transition from pending to active drag
			if (bDragPending)
			{
				int dist = std::abs(mousePos.X - dragStartMousePos.X) + std::abs(mousePos.Y - dragStartMousePos.Y);

				if (dist >= 15)
				{
					// Transition to active drag!
					bIsDragging = true;
					bDragPending = false;

					// If dragging from a dropship slot, now temporarily remove it and refund it!
					if (nSourceDropshipIdx != -1)
					{
						bDraggedIsFixed = dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx];
						dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = nullptr;
						dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = false;

						if (!bDraggedIsFixed)
						{
							currentMoney += pDraggedUnitType->Cost;

							if (dropshipBayChosenUnitsCount.count(pDraggedUnitType) > 0)
								--dropshipBayChosenUnitsCount[pDraggedUnitType];
						}
					}
					else
					{
						bDraggedIsFixed = false;
					}

					if (startingDragDropSoundIdx >= 0)
						VocClass::PlayGlobal(startingDragDropSoundIdx, 0x2000, 1.0);

					repaintAll = true;
				}
			}

			// Check if mouse is released
			if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
			{
				// Drag finished or clicked!
				if (bDragPending)
				{
					// Quick Click (button released before moving 15 pixels)
					bDragPending = false;

					if (nSourceDropshipIdx == -1) // Clicked on sidebar
					{
						// Normal purchase to first free slot
						int maxInstances = INT_MAX;

						for (size_t idx = 0; idx < availableUnits.size(); ++idx)
						{
							if (availableUnits[idx] == pDraggedUnitType)
							{
								maxInstances = availableUnitsMaximums[idx] < 0 ? INT_MAX : availableUnitsMaximums[idx];
								break;
							}
						}

						int nInstances = dropshipBayChosenUnitsCount.count(pDraggedUnitType) > 0 ? dropshipBayChosenUnitsCount[pDraggedUnitType] : 0;

						bool hasCompatibleFreeSlot = false;

						for (int i_c = 0; i_c < (int)dropshipBayChosenUnitsLists.size() && !hasCompatibleFreeSlot; i_c++)
						{
							if (CanCarrierHoldUnit(i_c, pDraggedUnitType))
							{
								for (int j_c = 0; j_c < (int)dropshipBayChosenUnitsLists[i_c].size(); j_c++)
								{
									if (!dropshipBayChosenUnitsLists[i_c][j_c])
									{
										hasCompatibleFreeSlot = true;
										break;
									}
								}
							}
						}

						if (nInstances < maxInstances
							&& pDraggedUnitType->Cost <= currentMoney
							&& hasCompatibleFreeSlot)
						{
							bool foundFreeSlot = false;

							for (int i = 0; i < (int)dropshipBayCameLocations.size() && !foundFreeSlot; i++)
							{
								if (!CanCarrierHoldUnit(i, pDraggedUnitType))
									continue;

								for (int j = 0; j < (int)dropshipBayCameLocations[i].size() && !foundFreeSlot; j++)
								{
									if (!dropshipBayChosenUnitsLists[i][j])
									{
										dropshipBayChosenUnitsLists[i][j] = pDraggedUnitType;
										dropshipBayFixedUnitsLists[i][j] = false;
										currentMoney -= pDraggedUnitType->Cost;
										foundFreeSlot = true;
										lastSelected = pDraggedUnitType;
										++dropshipBayChosenUnitsCount[pDraggedUnitType];
										VocClass::PlayGlobal(buyClickSoundIdx, 0x2000, 1.0);
									}
								}
							}
						}
					}
					else // Clicked on dropship slot
					{
						// Do not sell on quick left-click; just select it.
						lastSelected = pDraggedUnitType;
					}

					pDraggedUnitType = nullptr;
					repaintAll = true;
				}
				else if (bIsDragging)
				{
					// Drag & Drop drop logic
					int btn_BasicDropshipCameo_ID = 200;
					int btn_BasicSidebarCameo_ID = 300;
					bool droppedOnSlot = (buttonID >= btn_BasicDropshipCameo_ID && buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots));
					bool droppedOnSidebar = (buttonID >= btn_BasicSidebarCameo_ID && buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos));

					bool droppedOnSidebarArea = false;

					if (nSourceDropshipIdx != -1 && !sidebarCameLocations.empty())
					{
						int minX = sidebarCameLocations[0].X;
						int minY = sidebarCameLocations[0].Y;
						int maxX = sidebarCameLocations[0].X + sidebarCameLocations[0].Width;
						int maxY = sidebarCameLocations[0].Y + sidebarCameLocations[0].Height;

						for (const auto& rect : sidebarCameLocations)
						{
							if (rect.X < minX) minX = rect.X;
							if (rect.Y < minY) minY = rect.Y;
							if (rect.X + rect.Width > maxX) maxX = rect.X + rect.Width;
							if (rect.Y + rect.Height > maxY) maxY = rect.Y + rect.Height;
						}

						if (upArrowLocation.Y < minY) minY = upArrowLocation.Y;
						if (downArrowLocation.Y < minY) minY = downArrowLocation.Y;
						if (upArrowLocation.Y + upArrowLocation.Height > maxY) maxY = upArrowLocation.Y + upArrowLocation.Height;
						if (downArrowLocation.Y + downArrowLocation.Height > maxY) maxY = downArrowLocation.Y + downArrowLocation.Height;

						int sidebarLeft = minX - 10;
						int sidebarTop = minY - 10;
						int sidebarRight = windowRectangle.X + windowRectangle.Width;
						int sidebarBottom = maxY + 10;

						if (mousePos.X >= sidebarLeft && mousePos.X <= sidebarRight
							&& mousePos.Y >= sidebarTop && mousePos.Y <= sidebarBottom)
						{
							droppedOnSidebarArea = true;
						}
					}

					auto ReturnToSource = [&](bool playSound = true)
						{
							if (nSourceDropshipIdx != -1)
							{
								dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = pDraggedUnitType;
								dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = bDraggedIsFixed;

								if (!bDraggedIsFixed)
								{
									currentMoney -= pDraggedUnitType->Cost;
									++dropshipBayChosenUnitsCount[pDraggedUnitType];
								}

								if (playSound)
									VocClass::PlayGlobal(buyClickSoundIdx, 0x2000, 1.0);
							}
						};

					if (droppedOnSlot)
					{
						int dropshipIndex = (buttonID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
						int slotIndex = (buttonID - btn_BasicDropshipCameo_ID) - (dropshipIndex * nDropshipBayCameos);

						if (dropshipIndex < (int)dropshipBayChosenUnitsLists.size() && slotIndex < (int)dropshipBayChosenUnitsLists[dropshipIndex].size())
						{
							if (bDraggedIsFixed && dropshipIndex != nSourceDropshipIdx)
							{
								ReturnToSource();
							}
							else if (!CanCarrierHoldUnit(dropshipIndex, pDraggedUnitType))
							{
								ReturnToSource();
							}
							else
							{
								auto pTargetUnit = dropshipBayChosenUnitsLists[dropshipIndex][slotIndex];
								bool bTargetIsFixed = dropshipBayFixedUnitsLists[dropshipIndex][slotIndex];

								int maxInstances = INT_MAX;

								for (size_t idx = 0; idx < availableUnits.size(); ++idx)
								{
									if (availableUnits[idx] == pDraggedUnitType)
									{
										maxInstances = availableUnitsMaximums[idx] < 0 ? INT_MAX : availableUnitsMaximums[idx];
										break;
									}
								}

								int nInstances = dropshipBayChosenUnitsCount.count(pDraggedUnitType) > 0 ? dropshipBayChosenUnitsCount[pDraggedUnitType] : 0;

								if (pTargetUnit == nullptr)
								{
									if (bDraggedIsFixed)
									{
										dropshipBayChosenUnitsLists[dropshipIndex][slotIndex] = pDraggedUnitType;
										dropshipBayFixedUnitsLists[dropshipIndex][slotIndex] = true;
										lastSelected = pDraggedUnitType;

										if (endingDragDropSoundIdx >= 0)
											VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
									}
									else
									{
										long targetCost = pDraggedUnitType->Cost;

										if (nInstances < maxInstances && targetCost <= currentMoney)
										{
											dropshipBayChosenUnitsLists[dropshipIndex][slotIndex] = pDraggedUnitType;
											dropshipBayFixedUnitsLists[dropshipIndex][slotIndex] = false;
											currentMoney -= targetCost;
											++dropshipBayChosenUnitsCount[pDraggedUnitType];
											lastSelected = pDraggedUnitType;

											if (endingDragDropSoundIdx >= 0)
												VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
										}
										else
										{
											ReturnToSource();
										}
									}
								}
								else
								{
									if (nSourceDropshipIdx != -1)
									{
										if (pDraggedUnitType == pTargetUnit)
										{
											ReturnToSource(false);
										}
										else
										{
											// Dragged from a dropship slot -> SWAP them!
											if ((bDraggedIsFixed || bTargetIsFixed) && dropshipIndex != nSourceDropshipIdx)
											{
												ReturnToSource();
											}
											else if (!CanCarrierHoldUnit(nSourceDropshipIdx, pTargetUnit))
											{
												ReturnToSource();
											}
											else
											{
												dropshipBayChosenUnitsLists[dropshipIndex][slotIndex] = pDraggedUnitType;
												dropshipBayFixedUnitsLists[dropshipIndex][slotIndex] = bDraggedIsFixed;
												dropshipBayChosenUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = pTargetUnit;
												dropshipBayFixedUnitsLists[nSourceDropshipIdx][nSourceSlotIdx] = bTargetIsFixed;

												if (!bDraggedIsFixed)
												{
													currentMoney -= pDraggedUnitType->Cost;
													++dropshipBayChosenUnitsCount[pDraggedUnitType];
												}

												lastSelected = pDraggedUnitType;

												if (endingDragDropSoundIdx >= 0)
													VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
											}
										}
									}
									else
									{
										if (bTargetIsFixed)
										{
											ReturnToSource();
										}
										else
										{
											bool hasFreeSlot = false;

											for (auto const pType : dropshipBayChosenUnitsLists[dropshipIndex])
											{
												if (!pType)
												{
													hasFreeSlot = true;
													break;
												}
											}

											if (hasFreeSlot)
											{
												if (nInstances < maxInstances && pDraggedUnitType->Cost <= currentMoney)
												{
													int nullIdx = -1;

													if (nSourceDropshipIdx == dropshipIndex)
													{
														nullIdx = nSourceSlotIdx;
													}
													else
													{
														for (size_t k = 0; k < dropshipBayChosenUnitsLists[dropshipIndex].size(); ++k)
														{
															if (dropshipBayChosenUnitsLists[dropshipIndex][k] == nullptr)
															{
																nullIdx = static_cast<int>(k);
																break;
															}
														}
													}
													if (nullIdx != -1)
													{
														dropshipBayChosenUnitsLists[dropshipIndex].erase(dropshipBayChosenUnitsLists[dropshipIndex].begin() + nullIdx);
														dropshipBayChosenUnitsLists[dropshipIndex].insert(dropshipBayChosenUnitsLists[dropshipIndex].begin() + slotIndex, pDraggedUnitType);
														dropshipBayFixedUnitsLists[dropshipIndex].erase(dropshipBayFixedUnitsLists[dropshipIndex].begin() + nullIdx);
														dropshipBayFixedUnitsLists[dropshipIndex].insert(dropshipBayFixedUnitsLists[dropshipIndex].begin() + slotIndex, false);
													}

													currentMoney -= pDraggedUnitType->Cost;
													++dropshipBayChosenUnitsCount[pDraggedUnitType];
													lastSelected = pDraggedUnitType;

													if (endingDragDropSoundIdx >= 0)
														VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
												}
												else
												{
													// Can't afford shift, try replacement!
													long targetRefund = pTargetUnit->Cost;
													long netCost = pDraggedUnitType->Cost - targetRefund;
													bool limitOk = (pDraggedUnitType == pTargetUnit) || (nInstances < maxInstances);

													if (limitOk && netCost <= currentMoney)
													{
														currentMoney += targetRefund;

														if (dropshipBayChosenUnitsCount.count(pTargetUnit) > 0)
															--dropshipBayChosenUnitsCount[pTargetUnit];

														dropshipBayChosenUnitsLists[dropshipIndex][slotIndex] = pDraggedUnitType;
														dropshipBayFixedUnitsLists[dropshipIndex][slotIndex] = false;
														currentMoney -= pDraggedUnitType->Cost;
														++dropshipBayChosenUnitsCount[pDraggedUnitType];
														lastSelected = pDraggedUnitType;

														if (endingDragDropSoundIdx >= 0)
															VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
													}
													else
													{
														ReturnToSource();
													}
												}
											}
											else
											{
												long targetRefund = pTargetUnit->Cost;
												long netCost = pDraggedUnitType->Cost - targetRefund;
												bool limitOk = (pDraggedUnitType == pTargetUnit) || (nInstances < maxInstances);

												if (limitOk && netCost <= currentMoney)
												{
													currentMoney += targetRefund;

													if (dropshipBayChosenUnitsCount.count(pTargetUnit) > 0)
														--dropshipBayChosenUnitsCount[pTargetUnit];

													dropshipBayChosenUnitsLists[dropshipIndex][slotIndex] = pDraggedUnitType;
													dropshipBayFixedUnitsLists[dropshipIndex][slotIndex] = false;
													currentMoney -= pDraggedUnitType->Cost;
													++dropshipBayChosenUnitsCount[pDraggedUnitType];
													lastSelected = pDraggedUnitType;

													if (endingDragDropSoundIdx >= 0)
														VocClass::PlayGlobal(endingDragDropSoundIdx, 0x2000, 1.0);
												}
												else
												{
													ReturnToSource();
												}
											}
										}
									}
								}
							}
						}
						else
						{
							ReturnToSource();
						}
					}
					else if ((droppedOnSidebar || droppedOnSidebarArea) && nSourceDropshipIdx != -1)
					{
						if (bDraggedIsFixed) // Dropped on sidebar -> permanently sold/removed.
							ReturnToSource();
						else // We already refunded the money and decremented the count when active drag started. So we just let it be.
							VocClass::PlayGlobal(sellClickSoundIdx, 0x2000, 1.0);
					}
					else
					{
						ReturnToSource();
					}

					bIsDragging = false;
					pDraggedUnitType = nullptr;
					repaintAll = true;
				}
			}
		}

		HandleInput(command, buttonID);
		UpdateAnimations();

		if (bIsDragging)
			repaintAll = true;

		if (repaintAll)
		{
			Render(pSurface);
			repaintAll = false;
		}

		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
		GScreenClass::Instance.DoBlit(true, pSurface, nullptr);

		Sleep(1);
	}

	bDropshipLoadoutActive = false;
	SaveCargo();
}

void DropshipLoadoutClass::HandleInput(int command, int buttonID)
{
	int btn_ScrollUp_ID = 100;
	int btn_ScrollDown_ID = 101;
	int btn_BasicDropshipCameo_ID = 200;
	int btn_BasicSidebarCameo_ID = 300;

	if (bIsDragging || bDragPending)
		return;

	bool pressedLeftClick = command == 1;
	if (pressedLeftClick)
	{
		Point2D mousePos = { 0, 0 };
		if (WWMouseClass::Instance)
		{
			mousePos.X = WWMouseClass::Instance->GetX();
			mousePos.Y = WWMouseClass::Instance->GetY();
		}

		if (buttonID >= btn_BasicSidebarCameo_ID && buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos))
		{
			int sidebarIndex = firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID);

			if (sidebarIndex < (int)availableUnits.size())
			{
				auto const pType = availableUnits[sidebarIndex];

				if (pType)
				{
					int maxInstances = availableUnitsMaximums[sidebarIndex] < 0 ? INT_MAX : availableUnitsMaximums[sidebarIndex];
					int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

					if (nInstances < maxInstances)
					{
						bDragPending = true;
						pDraggedUnitType = pType;
						nSourceDropshipIdx = -1;
						nSourceSlotIdx = -1;
						dragStartMousePos = mousePos;

						return;
					}
				}
			}
		}
		else if (buttonID >= btn_BasicDropshipCameo_ID && buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots))
		{
			int dropshipIndex = (buttonID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
			int slotIndex = (buttonID - btn_BasicDropshipCameo_ID) - (dropshipIndex * nDropshipBayCameos);

			if (dropshipIndex < (int)dropshipBayChosenUnitsLists.size() && slotIndex < (int)dropshipBayChosenUnitsLists[dropshipIndex].size())
			{
				auto pType = dropshipBayChosenUnitsLists[dropshipIndex][slotIndex];
				if (pType)
				{
					bDragPending = true;
					pDraggedUnitType = pType;
					nSourceDropshipIdx = dropshipIndex;
					nSourceSlotIdx = slotIndex;
					dragStartMousePos = mousePos;

					return;
				}
			}
		}
	}

	TechnoTypeClass* pPrevHovered = pHoveredUnitType;
	pHoveredUnitType = nullptr;
	hoveredDropshipIdx = -1;
	hoveredSlotIdx = -1;

	if (buttonID >= btn_BasicSidebarCameo_ID && buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos))
	{
		int sidebarIndex = firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID);

		if (sidebarIndex < (int)availableUnits.size())
			pHoveredUnitType = availableUnits[sidebarIndex];
	}
	else if (buttonID >= btn_BasicDropshipCameo_ID && buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots))
	{
		int dropshipIndex = (buttonID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
		int slotIndex = (buttonID - btn_BasicDropshipCameo_ID) - (dropshipIndex * nDropshipBayCameos);

		if (dropshipIndex < (int)dropshipBayChosenUnitsLists.size() && slotIndex < (int)dropshipBayChosenUnitsLists[dropshipIndex].size())
		{
			pHoveredUnitType = dropshipBayChosenUnitsLists[dropshipIndex][slotIndex];
			hoveredDropshipIdx = dropshipIndex;
			hoveredSlotIdx = slotIndex;
		}
	}

	if (pHoveredUnitType != pPrevHovered)
		repaintAll = true;

	pressedLeftClick = command == 1;
	bool pressedRightClick = command == 2;

	bool isAnySidebarCameo = buttonID >= btn_BasicSidebarCameo_ID && buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos);
	bool isHoveringOverSidebarCameos = command == 0 && isAnySidebarCameo;
	bool pressedAnySidebarCameo = pressedLeftClick && isAnySidebarCameo;
	bool pressedAnySidebarCameoWithRigthClick = pressedRightClick && isAnySidebarCameo;

	bool isAnyDropshipCameo = buttonID >= btn_BasicDropshipCameo_ID && buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots);
	bool isHoveringOverDropshipCameos = command == 0 && isAnyDropshipCameo;
	bool pressedAnyDropshipCameo = pressedRightClick && isAnyDropshipCameo;
	int mouseOverDropshipCameoID = isHoveringOverDropshipCameos ? buttonID : -1;

	bool isUpArrow = buttonID == btn_ScrollUp_ID;
	bool isDownArrow = buttonID == btn_ScrollDown_ID;
	bool pressedUpArrow = command == VK_UP || (pressedLeftClick && isUpArrow);
	bool pressedDownArrow = command == VK_DOWN || (pressedLeftClick && isDownArrow);

	bool isScrollFromWheel = (pendingScrolls != 0);
	if (pendingScrolls < 0)
	{
		pressedUpArrow = true;
		pendingScrolls++;
	}
	else if (pendingScrolls > 0)
	{
		pressedDownArrow = true;
		pendingScrolls--;
	}

	bool playScrollSound = !isScrollFromWheel;

	if (pressedUpArrow)
		command = btn_ScrollUp_ID;
	else if (pressedDownArrow)
		command = btn_ScrollDown_ID;
	else if (pressedAnySidebarCameo || pressedAnyDropshipCameo || pressedAnySidebarCameoWithRigthClick)
		command = buttonID;

	bool validSidebarCameoPurchase = false;
	freeDropshipSlots = false;
	Point2D mouseLocationInDropshipCameos = { 0, 0 };

	for (int i = 0; i < (int)dropshipBayCameLocations.size() && !freeDropshipSlots; i++)
	{
		if (i >= (int)dropshipBayChosenUnitsLists.size())
			continue;

		for (int j = 0; j < (int)dropshipBayCameLocations[i].size() && !freeDropshipSlots; j++)
		{
			if (j >= (int)dropshipBayChosenUnitsLists[i].size())
				continue;

			if (dropshipBayChosenUnitsLists[i][j])
				continue;

			freeDropshipSlots = true;
			break;
		}
	}

	if (isHoveringOverSidebarCameos || pressedAnySidebarCameo)
	{
		int sidebarIndex = firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID);

		if (sidebarIndex < (int)availableUnits.size())
		{
			auto const pType = availableUnits[sidebarIndex];

			if (pType)
			{
				int maxInstances = availableUnitsMaximums[sidebarIndex] < 0 ? INT_MAX : availableUnitsMaximums[sidebarIndex];
				int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

				bool hasCompatibleFreeSlot = false;

				for (int i_c = 0; i_c < (int)dropshipBayChosenUnitsLists.size() && !hasCompatibleFreeSlot; i_c++)
				{
					if (CanCarrierHoldUnit(i_c, pType))
					{
						for (int j_c = 0; j_c < (int)dropshipBayChosenUnitsLists[i_c].size(); j_c++)
						{
							if (!dropshipBayChosenUnitsLists[i_c][j_c])
							{
								hasCompatibleFreeSlot = true;
								break;
							}
						}
					}
				}

				if (nInstances < maxInstances
					&& pType->Cost <= currentMoney
					&& hasCompatibleFreeSlot)
				{
					validSidebarCameoPurchase = true;
				}
			}
		}
	}

	if (isHoveringOverDropshipCameos)
	{
		bool found = false;

		for (int i = 0; i < (int)dropshipBayCameLocations.size() && !found; i++)
		{
			for (int j = 0; j < (int)dropshipBayCameLocations[i].size() && !found; j++)
			{
				int dropshipIndex = (mouseOverDropshipCameoID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
				int slotIndex = mouseOverDropshipCameoID - btn_BasicDropshipCameo_ID - (dropshipIndex * nDropshipBayCameos);

				if (i == dropshipIndex && j == slotIndex)
				{
					mouseLocationInDropshipCameos = { i, j };
					found = true;
					break;
				}
			}
		}
	}

	if (pressedUpArrow)
	{
		if (firstBrowsableCameo >= 2)
		{
			firstBrowsableCameo -= 2;
			repaintAll = true;

			if (playScrollSound)
				VocClass::PlayGlobal(arrowsClickSoundIdx, 0x2000, 1.0);
		}
	}
	else if (pressedDownArrow)
	{
		if (availableUnits.size() > (size_t)(firstBrowsableCameo + nSidebarCameos))
		{
			firstBrowsableCameo += 2;
			repaintAll = true;

			if (playScrollSound)
				VocClass::PlayGlobal(arrowsClickSoundIdx, 0x2000, 1.0);
		}
	}
	else if (pressedAnySidebarCameoWithRigthClick)
	{
		int newIndex = firstBrowsableCameo + (command - btn_BasicSidebarCameo_ID);

		if (newIndex >= 0 && newIndex < (int)availableUnits.size())
		{
			auto const pType = availableUnits[newIndex];

			if (pType)
			{
				bool found = false;

				for (int i = (int)dropshipBayChosenUnitsLists.size() - 1; i >= 0 && !found; --i)
				{
					auto& dropshipBay = dropshipBayChosenUnitsLists[i];

					for (int j = (int)dropshipBay.size() - 1; j >= 0 && !found; --j)
					{
						if (dropshipBay[j] == pType && !dropshipBayFixedUnitsLists[i][j])
						{
							currentMoney += pType->Cost;
							dropshipBay.erase(dropshipBay.begin() + j);
							dropshipBay.push_back(nullptr);
							dropshipBayFixedUnitsLists[i].erase(dropshipBayFixedUnitsLists[i].begin() + j);
							dropshipBayFixedUnitsLists[i].push_back(false);
							found = true;
							repaintAll = true;

							if (dropshipBayChosenUnitsCount.count(pType) > 0)
								--dropshipBayChosenUnitsCount[pType];
							else
								dropshipBayChosenUnitsCount[pType] = 0;

							VocClass::PlayGlobal(sellClickSoundIdx, 0x2000, 1.0);
							break;
						}
					}
				}
			}
		}
	}
	else if (pressedAnySidebarCameo)
	{
		int newIndex = firstBrowsableCameo + (command - btn_BasicSidebarCameo_ID);

		if (newIndex >= 0 && newIndex < (int)availableUnits.size())
		{
			if (validSidebarCameoPurchase)
			{
				auto const pType = availableUnits[newIndex];

				if (pType)
				{
					bool foundFreeSlot = false;

					for (int i = 0; i < (int)dropshipBayCameLocations.size() && !foundFreeSlot; i++)
					{
						if (i >= (int)dropshipBayChosenUnitsLists.size())
							continue;

						for (int j = 0; j < (int)dropshipBayCameLocations[i].size() && !foundFreeSlot; j++)
						{
							if (j >= (int)dropshipBayChosenUnitsLists[i].size())
								continue;

							auto const pDropshipSlotType = dropshipBayChosenUnitsLists[i][j];

							if (pDropshipSlotType || !CanCarrierHoldUnit(i, pType))
								continue;

							dropshipBayChosenUnitsLists[i][j] = pType;
							currentMoney -= pType->Cost;
							foundFreeSlot = true;
							lastSelected = pType;

							++dropshipBayChosenUnitsCount[pType];
							VocClass::PlayGlobal(buyClickSoundIdx, 0x2000, 1.0);
							break;
						}
					}

					if (foundFreeSlot)
						repaintAll = true;

					if (sidebarRowAnimationIndex < 0)
					{
						sidebarRowAnimationIndex = ((command - btn_BasicSidebarCameo_ID) / 2);

						if (dropshipLoadout_DGreenListPCX.size() > 0)
						{
							if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenListPCX.size())
								animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
							else
								sidebarRowAnimationIndex = -1;

							sidebarRowAnimationTotalFrames = sidebarRowAnimationIndex >= 0 ? (int)dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex].size() - 1 : 0;
						}
						else
						{
							if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenList.size())
								animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
							else
								sidebarRowAnimationIndex = -1;

							sidebarRowAnimationTotalFrames = (sidebarRowAnimationIndex >= 0 && dropshipLoadout_DGreenList[sidebarRowAnimationIndex] != nullptr) ? dropshipLoadout_DGreenList[sidebarRowAnimationIndex]->Frames : 0;
						}
					}
				}
			}
		}
	}
	else if (pressedAnyDropshipCameo)
	{
		if (nDropshipBayCameos > 0)
		{
			int nDropship = (command - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
			int index = command - btn_BasicDropshipCameo_ID - (nDropship * nDropshipBayCameos);

			if (nDropship >= 0 && nDropship < (int)dropshipBayChosenUnitsLists.size())
			{
				if (index >= 0 && index < (int)dropshipBayChosenUnitsLists[nDropship].size())
				{
					auto pType = dropshipBayChosenUnitsLists[nDropship][index];

					if (pType && !dropshipBayFixedUnitsLists[nDropship][index])
					{
						currentMoney += pType->Cost;
						auto& affectedDropship = dropshipBayChosenUnitsLists[nDropship];
						affectedDropship.erase(affectedDropship.begin() + index);
						affectedDropship.push_back(nullptr);
						dropshipBayFixedUnitsLists[nDropship].erase(dropshipBayFixedUnitsLists[nDropship].begin() + index);
						dropshipBayFixedUnitsLists[nDropship].push_back(false);
						repaintAll = true;

						if (dropshipBayChosenUnitsCount.count(pType) > 0)
							--dropshipBayChosenUnitsCount[pType];
						else
							dropshipBayChosenUnitsCount[pType] = 0;

						VocClass::PlayGlobal(sellClickSoundIdx, 0x2000, 1.0);
					}
				}
			}
		}
	}
	else if (isHoveringOverDropshipCameos || isHoveringOverSidebarCameos)
	{
		lastTimeWasOverCameos = true;
		repaintAll = true;
	}
	else if (lastTimeWasOverCameos && !isHoveringOverDropshipCameos && !isHoveringOverSidebarCameos)
	{
		lastTimeWasOverCameos = false;
		repaintAll = true;
	}

	if (command == VK_SPACE)
		pressedSpaceKey = true;

	if (command == VK_ESCAPE)
	{
		bool soldAny = false;
		lastSelected = nullptr;
		dropshipBayChosenUnitsCount.clear();

		for (size_t i = 0; i < dropshipBayChosenUnitsLists.size(); ++i)
		{
			std::vector<TechnoTypeClass*> newUnits;
			std::vector<bool> newFixed;

			for (size_t j = 0; j < dropshipBayChosenUnitsLists[i].size(); ++j)
			{
				if (dropshipBayFixedUnitsLists[i][j])
				{
					newUnits.push_back(dropshipBayChosenUnitsLists[i][j]);
					newFixed.push_back(true);
				}
				else if (dropshipBayChosenUnitsLists[i][j] != nullptr)
				{
					soldAny = true;
				}
			}

			while (newUnits.size() < dropshipBayChosenUnitsLists[i].size())
			{
				newUnits.push_back(nullptr);
				newFixed.push_back(false);
			}

			dropshipBayChosenUnitsLists[i] = newUnits;
			dropshipBayFixedUnitsLists[i] = newFixed;
		}

		currentMoney = initialMoney;
		repaintAll = true;

		if (soldAny)
			VocClass::PlayGlobal(sellClickSoundIdx, 0x2000, 1.0);
	}
}

void DropshipLoadoutClass::UpdateAnimations()
{
	if (animTimer_DelayedStartTimer_Loadout.Completed())
	{
		if (animTimer_UpdateFrameTimer_Loadout.Completed())
		{
			if (currentLoadoutFrame < loadoutTotalFrames)
			{
				currentLoadoutFrame++;
			}
			else
			{
				currentLoadoutFrame = -1;
				animTimer_DelayedStartValue_Loadout = ScenarioClass::Instance->Random(0, 0);
				animTimer_DelayedStartTimer_Loadout.Start(animTimer_DelayedStartValue_Loadout);
			}

			animTimer_UpdateFrameTimer_Loadout.Start(loadoutFrameDelay);
			repaintAll = true;
		}
	}

	if (animTimer_DelayedStartTimer_PilotLit.Completed())
	{
		if (animTimer_UpdateFrameTimer_PilotLit.Completed())
		{
			if (currentPilotLitFrame < pilotLitTotalFrames)
				currentPilotLitFrame++;
			else
			{
				currentPilotLitFrame = -1;
				animTimer_DelayedStartValue_PilotLit = ScenarioClass::Instance->Random(100, 300);
				animTimer_DelayedStartTimer_PilotLit.Start(animTimer_DelayedStartValue_PilotLit);
			}

			animTimer_UpdateFrameTimer_PilotLit.Start(pilotLitFrameDelay);
			repaintAll = true;
		}
	}

	if (sidebarRowAnimationIndex >= 0)
	{
		if (animTimer_UpdateFrameTimer_SidebarRowAnimation.Completed())
		{
			if (currentSidebarRowAnimationFrame < sidebarRowAnimationTotalFrames)
			{
				currentSidebarRowAnimationFrame++;
				animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
			}
			else
			{
				currentSidebarRowAnimationFrame = -1;
				sidebarRowAnimationIndex = -1;
			}

			repaintAll = true;
		}
	}

	if (animTimer_UpdateFrameTimer.Completed())
		animTimer_UpdateFrameTimer.Start(animTimer_StartValue);
}

void DropshipLoadoutClass::Render(DSurface* pSurface)
{
	if (!pSurface)
		return;

	pSurface->Fill(0);
	GeneralUtils::DrawImage(
		pSurface,
		windowRectangle,
		dropshipLoadout_BackgroundPCX,
		dropshipLoadout_Background,
		dropshipLoadout_Palette
	);

	bool isHoveringSidebar = false;

	if (WWMouseClass::Instance)
	{
		RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;

		for (const auto& rect : sidebarCameLocations)
		{
			if (mouseRect.X >= rect.X && mouseRect.X <= (rect.X + rect.Width)
				&& mouseRect.Y >= rect.Y && mouseRect.Y <= (rect.Y + rect.Height))
			{
				isHoveringSidebar = true;
				break;
			}
		}
	}

	bool isMouseOverSidebarArea = false;

	if (WWMouseClass::Instance && !sidebarCameLocations.empty())
	{
		int minX = sidebarCameLocations[0].X;
		int minY = sidebarCameLocations[0].Y;
		int maxX = sidebarCameLocations[0].X + sidebarCameLocations[0].Width;
		int maxY = sidebarCameLocations[0].Y + sidebarCameLocations[0].Height;

		for (const auto& rect : sidebarCameLocations)
		{
			if (rect.X < minX) minX = rect.X;
			if (rect.Y < minY) minY = rect.Y;
			if (rect.X + rect.Width > maxX) maxX = rect.X + rect.Width;
			if (rect.Y + rect.Height > maxY) maxY = rect.Y + rect.Height;
		}

		if (upArrowLocation.Y < minY) minY = upArrowLocation.Y;
		if (downArrowLocation.Y < minY) minY = downArrowLocation.Y;
		if (upArrowLocation.Y + upArrowLocation.Height > maxY) maxY = upArrowLocation.Y + upArrowLocation.Height;
		if (downArrowLocation.Y + downArrowLocation.Height > maxY) maxY = downArrowLocation.Y + downArrowLocation.Height;

		int sidebarLeft = minX - 10;
		int sidebarTop = minY - 10;
		int sidebarRight = windowRectangle.X + windowRectangle.Width;
		int sidebarBottom = maxY + 10;

		RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;
		if (mouseRect.X >= sidebarLeft && mouseRect.X <= sidebarRight
			&& mouseRect.Y >= sidebarTop && mouseRect.Y <= sidebarBottom)
		{
			isMouseOverSidebarArea = true;
		}
	}

	for (int i = 0; i < nSidebarCameos; i++)
	{
		int newIndex = firstBrowsableCameo + i;

		if (newIndex >= (int)availableUnits.size())
			continue;

		if (i >= (int)sidebarCameLocations.size())
			continue;

		auto const pType = availableUnits[newIndex];

		if (!pType)
			continue;

		int maxInstances = availableUnitsMaximums[newIndex] < 0 ? INT_MAX : availableUnitsMaximums[newIndex];
		int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

		bool hasCompatibleFreeSlot = false;

		for (int i_c = 0; i_c < (int)dropshipBayChosenUnitsLists.size() && !hasCompatibleFreeSlot; i_c++)
		{
			if (CanCarrierHoldUnit(i_c, pType))
			{
				for (int j_c = 0; j_c < (int)dropshipBayChosenUnitsLists[i_c].size(); j_c++)
				{
					if (!dropshipBayChosenUnitsLists[i_c][j_c])
					{
						hasCompatibleFreeSlot = true;
						break;
					}
				}
			}
		}

		BlitterFlags bf = BlitterFlags::None;
		if (nInstances >= maxInstances || !hasCompatibleFreeSlot)
			bf = BlitterFlags::bf_400 | BlitterFlags::Darken;

		bool isHovering = false;

		if (!bIsDragging && !bDragPending && !pDraggedUnitType && WWMouseClass::Instance)
		{
			RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;
			isHovering = mouseRect.X >= sidebarCameLocations[i].X
				&& mouseRect.X <= (sidebarCameLocations[i].X + sidebarCameLocations[i].Width)
				&& mouseRect.Y >= sidebarCameLocations[i].Y
				&& mouseRect.Y <= (sidebarCameLocations[i].Y + sidebarCameLocations[i].Height);
		}

		ColorStruct foreColor;
		bool showHighlight = false;

		if (isHovering)
		{
			showHighlight = true;
			bool limitReached = (nInstances >= maxInstances);
			bool canBuyDirectly = (!limitReached && pType->Cost <= currentMoney && hasCompatibleFreeSlot);
			bool canReplaceAny = false;

			if (!limitReached)
			{
				for (auto const& dropship : dropshipBayChosenUnitsLists)
				{
					for (auto const pTarget : dropship)
					{
						if (pTarget && pType != pTarget)
						{
							long netCost = pType->Cost - pTarget->Cost;
							if (netCost <= currentMoney)
							{
								canReplaceAny = true;
								break;
							}
						}
					}

					if (canReplaceAny) break;
				}
			}

			if (canBuyDirectly)
				foreColor = ColorStruct { 0, 255, 0 }; // Green
			else if (canReplaceAny)
				foreColor = ColorStruct { 0, 0, 255 }; // Blue
			else
				foreColor = ColorStruct { 255, 0, 0 }; // Red
		}
		else if (pType == lastSelected)
		{
			showHighlight = true;
			foreColor = ColorStruct { 255, 239, 99 }; // Yellow
		}

		if (showHighlight)
		{
			RectangleStruct newRectangle = sidebarCameLocations[i];
			newRectangle.X -= 2;
			newRectangle.Width += 4;
			pSurface->FillRectTrans(&newRectangle, &foreColor, 255);
		}

		auto const pTypeExt = TechnoTypeExt::Fetch(pType);

		auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
		auto pFileSHP = pType->Cameo;
		auto pPalette = FileSystem::CAMEO_PAL;

		GeneralUtils::DrawImage(
			pSurface,
			sidebarCameLocations[i],
			pPCXSurface,
			pFileSHP,
			pPalette,
			0,
			-2,
			bf
		);
	}

	GeneralUtils::DrawImage(
		pSurface,
		upArrowLocation,
		dropshipLoadout_UpArrowPCX,
		dropshipLoadout_UpArrow,
		dropshipLoadout_Palette,
		0,
		-2
	);

	GeneralUtils::DrawImage(
		pSurface,
		downArrowLocation,
		dropshipLoadout_DownArrowPCX,
		dropshipLoadout_DownArrow,
		dropshipLoadout_Palette,
		0,
		-2
	);

	for (size_t i = 0; i < dropshipBayCameLocations.size(); i++)
	{
		if (i >= dropshipBayChosenUnitsLists.size())
			continue;

		for (size_t j = 0; j < dropshipBayCameLocations[i].size(); j++)
		{
			if (j >= dropshipBayChosenUnitsLists[i].size())
				continue;

			auto const pType = dropshipBayChosenUnitsLists[i][j];

			bool isHovering = false;

			if (WWMouseClass::Instance)
			{
				RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;
				isHovering = mouseRect.X >= dropshipBayCameLocations[i][j].X
					&& mouseRect.X <= (dropshipBayCameLocations[i][j].X + dropshipBayCameLocations[i][j].Width)
					&& mouseRect.Y >= dropshipBayCameLocations[i][j].Y
					&& mouseRect.Y <= (dropshipBayCameLocations[i][j].Y + dropshipBayCameLocations[i][j].Height);
			}

			ColorStruct foreColor;
			bool showHighlight = false;

			if (isHovering)
			{
				showHighlight = true;
				foreColor = ColorStruct { 255, 0, 0 };

				if (bIsDragging)
				{
					bool bTargetIsFixed = dropshipBayFixedUnitsLists[i][j];

					if (bDraggedIsFixed && static_cast<int>(i) != nSourceDropshipIdx)
					{
						showHighlight = false; // Fixed unit cannot go to other dropships
					}
					else if (!CanCarrierHoldUnit(i, pDraggedUnitType))
					{
						foreColor = ColorStruct { 170, 0, 255 }; // Violet (Too heavy)
					}
					else if (!pType)
					{
						if (bDraggedIsFixed || pDraggedUnitType->Cost <= currentMoney)
							foreColor = ColorStruct { 0, 0, 255 }; // Blue (empty slot valid drop)
						else
							showHighlight = false; // Cannot afford: no highlight
					}
					else
					{
						if (nSourceDropshipIdx != -1)
						{
							if (pDraggedUnitType == pType)
								showHighlight = false;
							else if ((bDraggedIsFixed || bTargetIsFixed) && static_cast<int>(i) != nSourceDropshipIdx)
								showHighlight = false; // Cannot swap fixed units between dropships
							else if (!CanCarrierHoldUnit(nSourceDropshipIdx, pType))
								foreColor = ColorStruct { 170, 0, 255 }; // Violet (Too heavy for source dropship)
							else
								foreColor = ColorStruct { 0, 0, 255 }; // Blue (swap)
						}
						else
						{
							// Dragged from sidebar
							if (bTargetIsFixed)
							{
								showHighlight = false; // Cannot replace fixed units from sidebar
							}
							else
							{
								bool targetDropshipHasFreeSlot = false;
								for (auto const pUnit : dropshipBayChosenUnitsLists[i])
								{
									if (!pUnit)
									{
										targetDropshipHasFreeSlot = true;
										break;
									}
								}

								// Can we afford a shift?
								bool canAffordShift = pDraggedUnitType->Cost <= currentMoney;

								// Can we afford a replacement?
								long netCost = pDraggedUnitType->Cost - pType->Cost;
								bool canAffordReplacement = netCost <= currentMoney;

								if (targetDropshipHasFreeSlot && canAffordShift)
									foreColor = ColorStruct { 0, 0, 255 }; // Blue (shift)
								else if (canAffordReplacement && pDraggedUnitType != pType)
									foreColor = ColorStruct { 255, 0, 0 }; // Red (overwrite/replace)
								else
									showHighlight = false; // Cannot afford either or redundant: no highlight
							}
						}
					}
				}
				else
				{
					if (pType && !dropshipBayFixedUnitsLists[i][j])
						foreColor = ColorStruct { 255, 0, 0 }; // Red (sellable hover)
					else
						showHighlight = false; // Don't highlight empty or fixed slot if not dragging
				}
			}
			else if (!bIsDragging && isHoveringSidebar && pHoveredUnitType && !CanCarrierHoldUnit(i, pHoveredUnitType))
			{
				showHighlight = true;
				foreColor = ColorStruct { 170, 0, 255 }; // Violet (Too heavy)
			}
			else if (!bIsDragging && pType && isHoveringSidebar && pHoveredUnitType)
			{
				// If hovering a sidebar cameo, see if this slot can be replaced by it
				int maxInstances = INT_MAX;
				for (size_t idx = 0; idx < availableUnits.size(); ++idx)
				{
					if (availableUnits[idx] == pHoveredUnitType)
					{
						maxInstances = availableUnitsMaximums[idx] < 0 ? INT_MAX : availableUnitsMaximums[idx];
						break;
					}
				}

				int nInstances = dropshipBayChosenUnitsCount.count(pHoveredUnitType) > 0 ? dropshipBayChosenUnitsCount[pHoveredUnitType] : 0;
				bool limitReached = (nInstances >= maxInstances);
				bool hasCompatibleFreeSlot = false;

				for (int i_c = 0; i_c < (int)dropshipBayChosenUnitsLists.size() && !hasCompatibleFreeSlot; i_c++)
				{
					if (CanCarrierHoldUnit(i_c, pHoveredUnitType))
					{
						for (int j_c = 0; j_c < (int)dropshipBayChosenUnitsLists[i_c].size(); j_c++)
						{
							if (!dropshipBayChosenUnitsLists[i_c][j_c])
							{
								hasCompatibleFreeSlot = true;
								break;
							}
						}
					}
				}

				bool canBuyDirectly = (!limitReached && pHoveredUnitType->Cost <= currentMoney && hasCompatibleFreeSlot);

				if (pHoveredUnitType == pType && !dropshipBayFixedUnitsLists[i][j])
				{
					// Hovering the same unit type: highlight in Red if limit is reached (to show where they are)
					if (limitReached)
					{
						showHighlight = true;
						foreColor = ColorStruct { 255, 0, 0 }; // Red
					}
				}
				else
				{
					// Only show replacement highlights on dropship cargo slots if the hovered unit CANNOT be bought normally
					if (!canBuyDirectly && !dropshipBayFixedUnitsLists[i][j])
					{
						bool limitOk = !limitReached;
						long netCost = pHoveredUnitType->Cost - pType->Cost;

						if (limitOk && netCost <= currentMoney)
						{
							showHighlight = true;
							foreColor = ColorStruct { 0, 0, 255 }; // Blue (can be replaced)
						}
					}
				}
			}

			if (showHighlight)
			{
				RectangleStruct newRectangle = dropshipBayCameLocations[i][j];
				newRectangle.X -= 2;
				newRectangle.Width += 4;
				int opacity = pType ? 255 : 76;
				FillRectTranslucent(pSurface, newRectangle, foreColor, opacity);
			}

			if (!pType)
				continue;

			auto const pTypeExt = TechnoTypeExt::Fetch(pType);

			auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
			auto pFileSHP = pType->Cameo;
			auto pPalette = FileSystem::CAMEO_PAL;

			BlitterFlags bf = BlitterFlags::None;

			if (dropshipBayFixedUnitsLists[i][j])
				bf = BlitterFlags::bf_400 | BlitterFlags::Darken;

			GeneralUtils::DrawImage(
				pSurface,
				dropshipBayCameLocations[i][j],
				pPCXSurface,
				pFileSHP,
				pPalette,
				0,
				-2,
				bf
			);
		}
	}

	if (currentLoadoutFrame >= 0)
	{
		BSurface* framePCX = nullptr;

		if (dropshipLoadout_LoadoutPCX.size() > 0)
		{
			if (currentLoadoutFrame < (int)dropshipLoadout_LoadoutPCX.size())
				framePCX = dropshipLoadout_LoadoutPCX[currentLoadoutFrame];
		}

		GeneralUtils::DrawImage(
			pSurface,
			loadoutLocation,
			framePCX,
			dropshipLoadout_Loadout,
			dropshipLoadout_Palette,
			currentLoadoutFrame,
			-2
		);
	}

	if (currentPilotLitFrame >= 0)
	{
		BSurface* framePCX = nullptr;

		if (dropshipLoadout_PilotLitPCX.size() > 0)
		{
			if (currentPilotLitFrame < (int)dropshipLoadout_PilotLitPCX.size())
				framePCX = dropshipLoadout_PilotLitPCX[currentPilotLitFrame];
		}

		GeneralUtils::DrawImage(
			pSurface,
			pilotLitLocation,
			framePCX,
			dropshipLoadout_PilotLit,
			dropshipLoadout_Palette,
			currentPilotLitFrame,
			-2
		);
	}

	if (sidebarRowAnimationIndex >= 0 && currentSidebarRowAnimationFrame >= 0)
	{
		if (sidebarRowAnimationIndex < (int)dGreenLocation.size())
		{
			BSurface* framePCX = nullptr;

			if (dropshipLoadout_DGreenListPCX.size() > 0)
			{
				if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenListPCX.size())
				{
					if (currentSidebarRowAnimationFrame < (int)dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex].size())
						framePCX = dropshipLoadout_DGreenListPCX[sidebarRowAnimationIndex][currentSidebarRowAnimationFrame];
				}
			}

			SHPStruct* fileSHP = nullptr;

			if (sidebarRowAnimationIndex < (int)dropshipLoadout_DGreenList.size())
				fileSHP = dropshipLoadout_DGreenList[sidebarRowAnimationIndex];

			GeneralUtils::DrawImage(
				pSurface,
				dGreenLocation[sidebarRowAnimationIndex],
				framePCX,
				fileSHP,
				dropshipLoadout_Palette,
				currentSidebarRowAnimationFrame,
				-2
			);
		}
	}

	wchar_t buffer[64];
	const wchar_t* csfCredits = StringTable::TryFetchString("TXT_DROPSHIP_CREDITS", L"Credits: %d");
	swprintf_s(buffer, csfCredits, currentMoney);
	COLORREF foreColor = Drawing::RGB_To_Int(255, 239, 99);
	TextPrintType style = (TextPrintType::FullShadow | TextPrintType::Point6Grad);
	Point2D creditsLabel = {
		windowRectangle.Width - 140,
		windowRectangle.Height - 15
	};

	pSurface->DrawTextA(buffer, &windowRectangle, &creditsLabel, foreColor, 0, style);

	const wchar_t* csfStartMission = StringTable::TryFetchString("TXT_DROPSHIP_START_MISSION", L"Press SPACE to continue");
	swprintf_s(buffer, csfStartMission);
	foreColor = Drawing::RGB_To_Int(255, 255, 255);
	style = (TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad);
	Point2D pressSpaceLabel = {
		(windowRectangle.Width - 175) / 2,
		windowRectangle.Height - 15
	};

	pSurface->DrawTextA(buffer, &windowRectangle, &pressSpaceLabel, foreColor, 0, style);

	// Draw Dragged Cameo
	if (bIsDragging && pDraggedUnitType)
	{
		auto const pTypeExt = TechnoTypeExt::Fetch(pDraggedUnitType);
		{
			auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
			auto pFileSHP = pDraggedUnitType->Cameo;
			auto pPalette = FileSystem::CAMEO_PAL;

			Point2D mousePos = { 0, 0 };

			if (WWMouseClass::Instance)
			{
				mousePos.X = WWMouseClass::Instance->GetX();
				mousePos.Y = WWMouseClass::Instance->GetY();
			}

			// Center the cameo on the mouse cursor
			const int cameoWidth = 60, cameoHeight = 48;
			RectangleStruct dragLoc = { mousePos.X - cameoWidth / 2, mousePos.Y - cameoHeight / 2, cameoWidth, cameoHeight };

			// Draw Highlight Border first (Blue by default, Red if dragged from dropship and hovering sidebar)
			RectangleStruct newRectangle = dragLoc;
			newRectangle.X -= 2;
			newRectangle.Width += 4;
			ColorStruct dragBorderColor = ColorStruct { 0, 0, 255 }; // Blue

			if (nSourceDropshipIdx != -1 && isMouseOverSidebarArea)
				dragBorderColor = ColorStruct { 255, 0, 0 }; // Red (sell indicator)

			pSurface->FillRectTrans(&newRectangle, &dragBorderColor, 255);

			// Draw the cameo with half transparency to make it look like a drag shadow
			GeneralUtils::DrawImage(
				pSurface,
				dragLoc,
				pPCXSurface,
				pFileSHP,
				pPalette,
				0,
				-2,
				BlitterFlags::bf_400 | BlitterFlags::Darken
			);
		}
	}

	this->DrawTooltip(pSurface);
}

void DropshipLoadoutClass::DrawTooltip(DSurface* pSurface)
{
	if (bIsDragging)
		return;

	if (!pHoveredUnitType)
		return;

	if (!BitFont::Instance || !BitText::Instance)
		return;

	int maxToolTipWidth = Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : 200;

	// Calculate maxLimit
	int maxLimit = -1;

	for (size_t idx = 0; idx < availableUnits.size(); ++idx)
	{
		if (availableUnits[idx] == pHoveredUnitType)
		{
			maxLimit = availableUnitsMaximums[idx];
			break;
		}
	}

	// Calculate currentCount
	int currentCount = 0;

	if (dropshipBayChosenUnitsCount.count(pHoveredUnitType) > 0)
		currentCount = dropshipBayChosenUnitsCount[pHoveredUnitType];

	// Determine dimensions of each line to compute the total box size
	int textWidth = 0;
	int textHeight = 0;

	// Name line
	int nameWidth = 0, nameHeight = 0;
	std::wstring nameStr = pHoveredUnitType->UIName;
	BitFont::Instance->GetTextDimension(nameStr.c_str(), &nameWidth, &nameHeight, maxToolTipWidth);
	textWidth = std::max(textWidth, nameWidth);
	textHeight += nameHeight;

	bool isHoveredInDropship = (hoveredDropshipIdx != -1);
	bool isHoveredFixed = (isHoveredInDropship && hoveredSlotIdx < (int)dropshipBayFixedUnitsLists[hoveredDropshipIdx].size() && dropshipBayFixedUnitsLists[hoveredDropshipIdx][hoveredSlotIdx]);

	// Availability line (if limit exists)
	int availWidth = 0, availHeight = 0;
	std::wstring availLabel = StringTable::TryFetchString("TXT_DROPSHIP_AVAILABLE", L"Available: ");
	std::wstring availValueStr;
	int availLabelWidth = 0, availLabelHeight = 0;
	int availValueWidth = 0, availValueHeight = 0;

	if (maxLimit > 0 && !isHoveredFixed)
	{
		BitFont::Instance->GetTextDimension(availLabel.c_str(), &availLabelWidth, &availLabelHeight, maxToolTipWidth);

		std::wostringstream availValueOss;
		availValueOss << (maxLimit - currentCount) << L"/" << maxLimit;
		availValueStr = availValueOss.str();
		BitFont::Instance->GetTextDimension(availValueStr.c_str(), &availValueWidth, &availValueHeight, maxToolTipWidth);

		availWidth = availLabelWidth + availValueWidth;
		availHeight = std::max(availLabelHeight, availValueHeight);
		textWidth = std::max(textWidth, availWidth);
		textHeight += availHeight + 2; // +2 line spacing
	}

	// Cost line
	std::wstring costLabelStr = StringTable::TryFetchString("TXT_DROPSHIP_COST", L"Cost: ");
	int costLabelWidth = 0, costLabelHeight = 0;
	int fullCostWidth = 0;
	int fullCostHeight = 0;
	std::wstring costValStr;
	int costValWidth = 0, costValHeight = 0;
	int cost = 0;

	if (!isHoveredFixed)
	{
		BitFont::Instance->GetTextDimension(costLabelStr.c_str(), &costLabelWidth, &costLabelHeight, maxToolTipWidth);

		cost = pHoveredUnitType->GetActualCost(HouseClass::CurrentPlayer);
		std::wostringstream costValOss;
		costValOss << Phobos::UI::CostLabel << std::abs(cost);
		costValStr = costValOss.str();
		BitFont::Instance->GetTextDimension(costValStr.c_str(), &costValWidth, &costValHeight, maxToolTipWidth);

		fullCostWidth = costLabelWidth + costValWidth;
		fullCostHeight = std::max(costLabelHeight, costValHeight);
		textWidth = std::max(textWidth, fullCostWidth);
		textHeight += fullCostHeight + 2; // +2 line spacing
	}

	// Description
	std::wstring descStr;
	int descWidth = 0, descHeight = 0;
	auto const pTypeExt = TechnoTypeExt::Fetch(pHoveredUnitType);

	if (Phobos::Config::ToolTipDescriptions && !pTypeExt->UIDescription.Get().empty())
	{
		descStr = pTypeExt->UIDescription.Get().Text;
		BitFont::Instance->GetTextDimension(descStr.c_str(), &descWidth, &descHeight, maxToolTipWidth);
		textWidth = std::max(textWidth, descWidth);
		textHeight += descHeight + 4; // +4 for extra paragraph gap
	}

	// Calculate final box bounds
	int boxPadding = 5;
	int boxWidth = textWidth + boxPadding * 2;
	int boxHeight = textHeight + boxPadding * 2;

	Point2D mousePos = { 0, 0 };
	if (WWMouseClass::Instance)
	{
		mousePos.X = WWMouseClass::Instance->GetX();
		mousePos.Y = WWMouseClass::Instance->GetY();
	}

	int minX = windowRectangle.X;
	int maxX = windowRectangle.X + windowRectangle.Width;
	int minY = windowRectangle.Y;
	int maxY = windowRectangle.Y + windowRectangle.Height;

	int boxX = mousePos.X + 15;
	int boxY = mousePos.Y + 15;

	if (boxX + boxWidth > maxX) boxX = mousePos.X - boxWidth - 5;
	if (boxY + boxHeight > maxY) boxY = maxY - boxHeight - 5;

	if (boxX < minX) boxX = minX;
	if (boxY < minY) boxY = minY;

	RectangleStruct boxRect = { boxX, boxY, boxWidth, boxHeight };

	// Draw translucent black background
	ColorStruct bgColor(0, 0, 0);
	pSurface->FillRectTrans(&boxRect, &bgColor, 180);

	// Draw border outline
	pSurface->DrawRect(&boxRect, Drawing::RGB_To_Int(120, 120, 120));

	// Save BitFont state to prevent side effects on other parts of UI
	LTRBStruct oldBounds = BitFont::Instance->Bounds;
	WORD oldColor = BitFont::Instance->Color;
	bool oldField41 = BitFont::Instance->field_41;

	// Set shared BitFont properties
	LTRBStruct ltrbBounds = { boxRect.X, boxRect.Y, boxRect.X + boxRect.Width, boxRect.Y + boxRect.Height };
	BitFont::Instance->field_41 = 1;
	BitFont::Instance->SetBounds(&ltrbBounds);

	int currentY = boxRect.Y + boxPadding;

	// Draw Name (Yellow/Gold)
	BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB_To_Int(255, 239, 99));
	BitText::Instance->DrawText(
		BitFont::Instance,
		pSurface,
		nameStr.c_str(),
		boxRect.X + boxPadding,
		currentY,
		nameWidth,
		nameHeight,
		0, 0, 0
	);
	currentY += nameHeight + 2;

	// Draw Availability (if limit exists)
	if (maxLimit > 0 && !isHoveredFixed)
	{
		// Draw label "Available: " (White)
		BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB_To_Int(255, 255, 255));
		BitText::Instance->DrawText(
			BitFont::Instance,
			pSurface,
			availLabel.c_str(),
			boxRect.X + boxPadding,
			currentY,
			availLabelWidth,
			availLabelHeight,
			0, 0, 0
		);

		// Draw value (Red / Yellow / White)
		int availableCount = maxLimit - currentCount;
		COLORREF availColor = Drawing::RGB_To_Int(255, 255, 255); // White

		if (availableCount == 0)
			availColor = Drawing::RGB_To_Int(255, 0, 0); // Red
		else if (availableCount * 2 <= maxLimit)
			availColor = Drawing::RGB_To_Int(255, 255, 0); // Yellow

		BitFont::Instance->Color = static_cast<WORD>(availColor);
		BitText::Instance->DrawText(
			BitFont::Instance,
			pSurface,
			availValueStr.c_str(),
			boxRect.X + boxPadding + availLabelWidth,
			currentY,
			availValueWidth,
			availValueHeight,
			0, 0, 0
		);

		currentY += availHeight + 2;
	}

	// Draw Cost
	if (!isHoveredFixed)
	{
		// Draw label "Cost: " (White)
		BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB_To_Int(255, 255, 255));
		BitText::Instance->DrawText(
			BitFont::Instance,
			pSurface,
			costLabelStr.c_str(),
			boxRect.X + boxPadding,
			currentY,
			costLabelWidth,
			costLabelHeight,
			0, 0, 0
		);

		// Draw value (Red / Yellow / White)
		COLORREF costColor = Drawing::RGB_To_Int(255, 255, 255); // White

		if (!isHoveredInDropship)
		{
			if (currentMoney < cost)
				costColor = Drawing::RGB_To_Int(255, 0, 0); // Red
			else if (currentMoney < cost * 2)
				costColor = Drawing::RGB_To_Int(255, 255, 0); // Yellow
		}

		BitFont::Instance->Color = static_cast<WORD>(costColor);
		BitText::Instance->DrawText(
			BitFont::Instance,
			pSurface,
			costValStr.c_str(),
			boxRect.X + boxPadding + costLabelWidth,
			currentY,
			costValWidth,
			costValHeight,
			0, 0, 0
		);

		currentY += fullCostHeight + 2;
	}

	// Draw Description (if exists)
	if (!descStr.empty())
	{
		currentY += 2; // Small gap before description paragraph
		BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB_To_Int(200, 200, 200)); // Light Gray
		BitText::Instance->DrawText(
			BitFont::Instance,
			pSurface,
			descStr.c_str(),
			boxRect.X + boxPadding,
			currentY,
			descWidth,
			descHeight,
			0, 0, 0
		);
	}

	// Restore BitFont state
	BitFont::Instance->Bounds = oldBounds;
	BitFont::Instance->Color = oldColor;
	BitFont::Instance->field_41 = oldField41;
}

int DropshipLoadoutClass::GetCarrierSizeLimit(int carrierIdx)
{
	if (carrierIdx < 0 || carrierIdx >= nStartingDropships)
		return -1;

	if (pSWTypeExt)
	{
		if (pSWTypeExt->DropshipLoadout_SizeLimit.isset())
			return pSWTypeExt->DropshipLoadout_SizeLimit;

		if (pSWTypeExt->DropshipLoadout_Carrier.isset())
			return static_cast<int>(pSWTypeExt->DropshipLoadout_Carrier.Get()->SizeLimit);

		if (auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer))
		{
			if (pHouseExt->DropshipLoadout_SWCarrier)
				return static_cast<int>(pHouseExt->DropshipLoadout_SWCarrier->SizeLimit);
		}

		return -1;
	}

	std::vector<int> sizeLimits;

	if (pHouseTypeExt->DropshipLoadout_Carriers_SizeLimit.size() > 0)
	{
		for (int limit : pHouseTypeExt->DropshipLoadout_Carriers_SizeLimit)
		{
			sizeLimits.push_back(limit);
		}
	}
	else if (ScenarioExt::Global())
	{
		for (int limit : ScenarioExt::Global()->DropshipLoadout_Carriers_SizeLimit)
		{
			sizeLimits.push_back(limit);
		}
	}

	int configuredLimit = -1;

	if (carrierIdx < (int)sizeLimits.size())
		configuredLimit = sizeLimits[carrierIdx];

	if (configuredLimit == 0)
	{
		std::vector<TechnoTypeClass*> carriers;

		if (pHouseTypeExt->DropshipLoadout_Carriers.size() > 0)
		{
			for (auto carrier : pHouseTypeExt->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}
		else if (ScenarioExt::Global())
		{
			for (auto carrier : ScenarioExt::Global()->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}

		if (carrierIdx < (int)carriers.size() && carriers[carrierIdx])
			return static_cast<int>(carriers[carrierIdx]->SizeLimit);
	}

	return configuredLimit;
}

bool DropshipLoadoutClass::CanCarrierHoldUnit(int carrierIdx, TechnoTypeClass* pUnitType)
{
	if (!pUnitType)
		return true;

	int limit = GetCarrierSizeLimit(carrierIdx);

	if (limit == -1)
		return true;

	return static_cast<int>(pUnitType->Size) <= limit;
}

void DropshipLoadoutClass::SaveCargo()
{
	if (!HouseClass::CurrentPlayer)
		return;

	auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer);

	if (!pHouseExt)
		return;

	if (pSWTypeExt)
	{
		std::vector<TechnoTypeClass*> unitsList;

		if (!dropshipBayChosenUnitsLists.empty())
		{
			for (auto const pTechno : dropshipBayChosenUnitsLists[0])
			{
				if (pTechno)
					unitsList.push_back(pTechno);
			}
		}

		pHouseExt->DropshipLoadout_SWCargo = unitsList;

		TechnoTypeClass* pCarrier = nullptr;

		if (pSWTypeExt->DropshipLoadout_Carrier.isset())
		{
			pCarrier = pSWTypeExt->DropshipLoadout_Carrier;
		}
		else
		{
			std::vector<TechnoTypeClass*> carriers;

			if (pHouseTypeExt->DropshipLoadout_Carriers.size() > 0)
			{
				for (auto carrier : pHouseTypeExt->DropshipLoadout_Carriers)
				{
					carriers.push_back(carrier);
				}
			}
			else if (ScenarioExt::Global())
			{
				for (auto carrier : ScenarioExt::Global()->DropshipLoadout_Carriers)
				{
					carriers.push_back(carrier);
				}
			}
			if (!carriers.empty())
				pCarrier = carriers[0];
		}

		pHouseExt->DropshipLoadout_SWCarrier = pCarrier;

		// Consume/remove initial units that were not kept in cargo (SW-specific pool)
		std::vector<TechnoTypeClass*> newInitialUnits;
		std::vector<TechnoTypeClass*> cargoCopy = unitsList;

		for (auto pUnit : pHouseExt->DropshipLoadout_SWInitialUnits)
		{
			if (pUnit)
			{
				auto it = std::find(cargoCopy.begin(), cargoCopy.end(), pUnit);

				if (it != cargoCopy.end())
				{
					newInitialUnits.push_back(pUnit);
					cargoCopy.erase(it);
				}
			}
		}

		pHouseExt->DropshipLoadout_SWInitialUnits = newInitialUnits;

		bool addUnusedMoneyToPlayer = this->bAddUnusedMoneyToPlayer.Get(false);

		if (this->startingMoney == -1)
		{
			long spent = HouseClass::CurrentPlayer->Available_Money() - currentMoney;
			HouseClass::CurrentPlayer->TransactMoney(-spent);
		}
		else
		{
			if (addUnusedMoneyToPlayer)
			{
				HouseClass::CurrentPlayer->TransactMoney(currentMoney);
			}
			else
			{
				if (this->startingMoney == 0 && !pSWTypeExt->DropshipLoadout_Money.isset())
				{
					long spent = HouseClass::CurrentPlayer->Available_Money() - currentMoney;
					HouseClass::CurrentPlayer->TransactMoney(-spent);
				}
			}
		}
	}
	else
	{
		pHouseExt->DropshipLoadout_Cargo.clear();
		pHouseExt->DropshipLoadout_Carriers.clear();

		std::vector<TechnoTypeClass*> carriers;

		if (pHouseTypeExt->DropshipLoadout_Carriers.size() > 0)
		{
			for (auto carrier : pHouseTypeExt->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}
		else if (ScenarioExt::Global())
		{
			for (auto carrier : ScenarioExt::Global()->DropshipLoadout_Carriers)
			{
				carriers.push_back(carrier);
			}
		}

		int nCarriers = (int)carriers.size();

		for (int i = 0; i < nStartingDropships && i < nCarriers; i++)
		{
			pHouseExt->DropshipLoadout_Carriers.push_back(carriers[i]);

			if (i >= (int)dropshipBayChosenUnitsLists.size())
				continue;

			auto& unitsList = pHouseExt->DropshipLoadout_Cargo.emplace_back();

			for (auto const pTechno : dropshipBayChosenUnitsLists[i])
			{
				if (pTechno)
					unitsList.push_back(pTechno);
			}
		}

		// Consume/remove initial units that were not kept in cargo
		if (!pHouseExt->DropshipLoadout_InitialUnits.empty())
		{
			for (size_t i = 0; i < pHouseExt->DropshipLoadout_InitialUnits.size() && i < (size_t)nStartingDropships; ++i)
			{
				std::vector<TechnoTypeClass*> newInitialList;
				std::vector<TechnoTypeClass*> cargoCopy;

				if (i < pHouseExt->DropshipLoadout_Cargo.size())
					cargoCopy = pHouseExt->DropshipLoadout_Cargo[i];

				for (auto pUnit : pHouseExt->DropshipLoadout_InitialUnits[i])
				{
					if (pUnit)
					{
						auto it = std::find(cargoCopy.begin(), cargoCopy.end(), pUnit);

						if (it != cargoCopy.end())
						{
							newInitialList.push_back(pUnit);
							cargoCopy.erase(it);
						}
					}
				}

				pHouseExt->DropshipLoadout_InitialUnits[i] = std::move(newInitialList);
			}
		}

		bool addUnusedMoneyToPlayer = false;

		if (this->bAddUnusedMoneyToPlayer.isset())
			addUnusedMoneyToPlayer = this->bAddUnusedMoneyToPlayer;
		else if (pHouseTypeExt->DropshipLoadout_AddUnusedMoneyToPlayer.isset())
			addUnusedMoneyToPlayer = pHouseTypeExt->DropshipLoadout_AddUnusedMoneyToPlayer;
		else if (ScenarioExt::Global())
			addUnusedMoneyToPlayer = ScenarioExt::Global()->DropshipLoadout_AddUnusedMoneyToPlayer;

		if (this->startingMoney == -1)
		{
			// Using player's own money: just deduct what was spent
			long spent = HouseClass::CurrentPlayer->Available_Money() - currentMoney;
			HouseClass::CurrentPlayer->TransactMoney(-spent);
		}
		else
		{
			// Using a separate budget (either from rules, global, or a custom amount > 0)
			if (addUnusedMoneyToPlayer)
			{
				HouseClass::CurrentPlayer->TransactMoney(currentMoney);
			}
			else
			{
				long dropshipLoadout_InitialMoney = -1;

				if (pHouseTypeExt->DropshipLoadout_Money.isset())
					dropshipLoadout_InitialMoney = pHouseTypeExt->DropshipLoadout_Money;
				else if (ScenarioExt::Global())
					dropshipLoadout_InitialMoney = ScenarioExt::Global()->DropshipLoadout_Money;

				if (this->startingMoney == 0 && dropshipLoadout_InitialMoney < 0)
				{
					// Fallback to player's own money in default rules
					long spent = HouseClass::CurrentPlayer->Available_Money() - currentMoney;
					HouseClass::CurrentPlayer->TransactMoney(-spent);
				}
			}
		}
	}
}

DEFINE_HOOK(0x683D89, Dropship_Loadout_Remake, 0x6)
{
	enum { EndFunction = 0x683D9C };

	if (!HouseClass::CurrentPlayer || !ScenarioClass::Instance)
		return EndFunction;

	auto const pHouseTypeExt = HouseTypeExt::Fetch(HouseClass::CurrentPlayer->Type);

	if (!pHouseTypeExt)
		return EndFunction;

	int nStartingDropships = pHouseTypeExt->DropshipLoadout_StartingDropships.isset() ? pHouseTypeExt->DropshipLoadout_StartingDropships.Get() : ScenarioExt::Global()->DropshipLoadout_StartingDropships;

	if (nStartingDropships <= 0)
		return EndFunction;

	DropshipLoadoutClass loadout;

	if (loadout.Initialize())
		loadout.Run();

	return EndFunction;
}

void DropshipLoadoutClass::OpenInGameWindow(bool bIgnoreFixedUnits, bool bPreloadCargo, int allowableUnitsIndex, int startingMoney, Nullable<bool> bAddUnusedMoneyToPlayer, Nullable<bool> bRememberPurchasedCargo, SuperWeaponTypeClass* pSWType)
{
	if (!ScenarioClass::Instance)
		return;

	ScenarioClass::Instance->PauseGame();

	const bool oldLocked = ScenarioClass::Instance->UserInputLocked;
	const bool oldPaused = ScenarioClass::Instance->IsGamePaused;

	ScenarioClass::Instance->UserInputLocked = false;
	ScenarioClass::Instance->IsGamePaused = false;

	DropshipLoadoutClass loadout;

	if (loadout.Initialize(bIgnoreFixedUnits, bPreloadCargo, allowableUnitsIndex, startingMoney, bAddUnusedMoneyToPlayer, bRememberPurchasedCargo, pSWType))
		loadout.Run();

	ScenarioClass::Instance->IsGamePaused = oldPaused;
	ScenarioClass::Instance->UserInputLocked = oldLocked;

	ScenarioClass::Instance->ResumeGame();
}

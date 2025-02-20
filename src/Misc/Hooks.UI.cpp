#include <Phobos.h>

#include <Helpers/Macro.h>
#include <PreviewClass.h>
#include <Surface.h>
#include <ThemeClass.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x777C41, UI_ApplyAppIcon, 0x9)
{
	if (Phobos::AppIconPath != nullptr && strlen(Phobos::AppIconPath))
	{
		Debug::Log("Applying AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(0x640B8D, LoadingScreen_DisableEmptySpawnPositions, 0x6)
{
	GET(bool, esi, ESI);
	if (Phobos::UI::DisableEmptySpawnPositions || !esi)
	{
		return 0x640CE2;
	}
	return 0x640B93;
}

//DEFINE_HOOK(0x640E78, LoadingScreen_DisableColorPoints, 0x6)
//{
//	return 0x641071;
//}

// Allow size = 0 for map previews
DEFINE_HOOK(0x641B41, LoadingScreen_SkipPreview, 0x8)
{
	GET(RectangleStruct*, pRect, EAX);
	if (pRect->Width > 0 && pRect->Height > 0)
	{
		return 0;
	}
	return 0x641D4E;
}

DEFINE_HOOK(0x641EE0, PreviewClass_ReadPreview, 0x6)
{
	GET(PreviewClass*, pThis, ECX);
	GET_STACK(const char*, lpMapFile, 0x4);

	CCFileClass file(lpMapFile);
	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		CCINIClass ini;
		ini.ReadCCFile(&file, true);
		ini.CurrentSection = nullptr;
		ini.CurrentSectionName = nullptr;

		ScenarioClass::Instance->ReadStartPoints(ini);

		R->EAX(pThis->ReadPreviewPack(ini));
	}
	else
		R->EAX(false);

	return 0x64203D;
}

DEFINE_HOOK(0x4A25E0, CreditsClass_GraphicLogic_HarvesterCounter, 0x7)
{
	auto const pPlayer = HouseClass::CurrentPlayer();
	if (pPlayer->Defeated)
		return 0;

	RectangleStruct vRect = DSurface::Sidebar->GetRect();

	if (Phobos::UI::HarvesterCounter_Show && Phobos::Config::ShowHarvesterCounter)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];
		auto nActive = HouseExt::ActiveHarvesterCount(pPlayer);
		auto nTotal = HouseExt::TotalHarvesterCount(pPlayer);
		auto nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;

		ColorStruct clrToolTip = nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow
			? Drawing::TooltipColor() : nPercentage > Phobos::UI::HarvesterCounter_ConditionRed
			? pSideExt->Sidebar_HarvesterCounter_Yellow : pSideExt->Sidebar_HarvesterCounter_Red;

		swprintf_s(counter, L"%ls%d/%d", Phobos::UI::HarvesterLabel, nActive, nTotal);

		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
		};

		DSurface::Sidebar->DrawText(counter, &vRect, &vPos, Drawing::RGB_To_Int(clrToolTip), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	}

	if (Phobos::UI::PowerDelta_Show && Phobos::Config::ShowPowerDelta && pPlayer->Buildings.Count)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];

		ColorStruct clrToolTip;

		if (pPlayer->PowerBlackoutTimer.InProgress())
		{
			clrToolTip = pSideExt->Sidebar_PowerDelta_Grey;
			swprintf_s(counter, L"%ls", Phobos::UI::PowerBlackoutLabel);
		}
		else
		{
			int delta = pPlayer->PowerOutput - pPlayer->PowerDrain;

			double percent = pPlayer->PowerOutput != 0
				? (double)pPlayer->PowerDrain / (double)pPlayer->PowerOutput : pPlayer->PowerDrain != 0
				? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow;

			clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
				? pSideExt->Sidebar_PowerDelta_Green : LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
				? pSideExt->Sidebar_PowerDelta_Yellow : pSideExt->Sidebar_PowerDelta_Red;

			swprintf_s(counter, L"%ls%+d", Phobos::UI::PowerLabel, delta);
		}

		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 - 70 + pSideExt->Sidebar_PowerDelta_Offset.Get().X,
			2 + pSideExt->Sidebar_PowerDelta_Offset.Get().Y
		};

		auto const TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_PowerDelta_Align.Get()));

		DSurface::Sidebar->DrawText(counter, &vRect, &vPos, Drawing::RGB_To_Int(clrToolTip), 0, TextFlags);
	}

	if (Phobos::UI::WeedsCounter_Show && Phobos::Config::ShowWeedsCounter)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];
		ColorStruct clrToolTip = pSideExt->Sidebar_WeedsCounter_Color.Get(Drawing::TooltipColor());

		swprintf_s(counter, L"%d", static_cast<int>(pPlayer->OwnedWeed.GetTotalAmount()));

		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 + 50 + pSideExt->Sidebar_WeedsCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_WeedsCounter_Offset.Get().Y
		};

		DSurface::Sidebar->DrawText(counter, &vRect, &vPos, Drawing::RGB_To_Int(clrToolTip), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8AA, Replace_XXICON_With_New, 0x7)   //SWTypeClass::Load
DEFINE_HOOK_AGAIN(0x6CEE31, Replace_XXICON_With_New, 0x7)   //SWTypeClass::ReadINI
DEFINE_HOOK_AGAIN(0x716D13, Replace_XXICON_With_New, 0x7)   //TechnoTypeClass::Load
DEFINE_HOOK(0x715A4D, Replace_XXICON_With_New, 0x7)         //TechnoTypeClass::ReadINI
{
	char pFilename[0x20];
	strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
	_strlwr_s(pFilename);

	if (_stricmp(pFilename, GameStrings::XXICON_SHP)
		&& strstr(pFilename, ".shp"))
	{
		if (auto pFile = FileSystem::LoadFile(RulesExt::Global()->MissingCameo, false))
		{
			R->EAX(pFile);
			return R->Origin() + 0xC;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A8463, StripClass_OperatorLessThan_CameoPriority, 0x5)
{
	GET_STACK(TechnoTypeClass*, pLeft, STACK_OFFSET(0x1C, -0x8));
	GET_STACK(TechnoTypeClass*, pRight, STACK_OFFSET(0x1C, -0x4));
	GET_STACK(int, idxLeft, STACK_OFFSET(0x1C, 0x8));
	GET_STACK(int, idxRight, STACK_OFFSET(0x1C, 0x10));
	GET_STACK(AbstractType, rttiLeft, STACK_OFFSET(0x1C, 0x4));
	GET_STACK(AbstractType, rttiRight, STACK_OFFSET(0x1C, 0xC));
	auto pLeftTechnoExt = pLeft ? TechnoTypeExt::ExtMap.Find(pLeft) : nullptr;
	auto pRightTechnoExt = pRight ? TechnoTypeExt::ExtMap.Find(pRight) : nullptr;
	auto pLeftSWExt = (rttiLeft == AbstractType::Special || rttiLeft == AbstractType::Super || rttiLeft == AbstractType::SuperWeaponType)
		? SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->GetItem(idxLeft)) : nullptr;
	auto pRightSWExt = (rttiRight == AbstractType::Special || rttiRight == AbstractType::Super || rttiRight == AbstractType::SuperWeaponType)
		? SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->GetItem(idxRight)) : nullptr;

	if ((pLeftTechnoExt || pLeftSWExt) && (pRightTechnoExt || pRightSWExt))
	{
		auto leftPriority = pLeftTechnoExt ? pLeftTechnoExt->CameoPriority : pLeftSWExt->CameoPriority;
		auto rightPriority = pRightTechnoExt ? pRightTechnoExt->CameoPriority : pRightSWExt->CameoPriority;
		enum { rTrue = 0x6A8692, rFalse = 0x6A86A0 };

		if (leftPriority > rightPriority)
			return rTrue;
		else if (rightPriority > leftPriority)
			return rFalse;
	}

	// Restore overridden instructions
	GET(AbstractType, rtti1, ESI);
	return rtti1 == AbstractType::Special ? 0x6A8477 : 0x6A8468;
}

DEFINE_HOOK(0x6D4684, TacticalClass_Draw_FlyingStrings, 0x6)
{
	FlyingStrings::UpdateAll();
	return 0;
}

DEFINE_HOOK(0x456776, BuildingClass_DrawRadialIndicator_Visibility, 0x6)
{
	enum { ContinueDraw = 0x456789, DoNotDraw = 0x456962 };
	GET(BuildingClass* const, pThis, ESI);

	if (HouseClass::IsCurrentPlayerObserver() || pThis->Owner->IsControlledByCurrentPlayer())
		return ContinueDraw;

	AffectedHouse const canSee = RulesExt::Global()->RadialIndicatorVisibility.Get();
	if (pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer) ? canSee & AffectedHouse::Allies : canSee & AffectedHouse::Enemies)
		return ContinueDraw;

	return DoNotDraw;
}

#pragma region ShowBriefing

namespace BriefingTemp
{
	bool ShowBriefing = false;
}

__forceinline void ShowBriefing()
{
	if (BriefingTemp::ShowBriefing)
	{
		// Show briefing dialog.
		Game::SpecialDialog = 9;
		Game::ShowSpecialDialog();
		BriefingTemp::ShowBriefing = false;

		// Play scenario theme.
		int theme = ScenarioClass::Instance->ThemeIndex;

		if (theme == -1)
			ThemeClass::Instance->Stop(true);
		else
			ThemeClass::Instance->Queue(theme);
	}
}

// Check if briefing dialog should be played before starting scenario.
DEFINE_HOOK(0x683E41, ScenarioClass_Start_ShowBriefing, 0x6)
{
	enum { SkipGameCode = 0x683E6B };

	GET_STACK(bool, showBriefing, STACK_OFFSET(0xFC, -0xE9));

	// Don't show briefing dialog for non-campaign games etc.
	if (!Phobos::Config::ShowBriefing || !ScenarioExt::Global()->ShowBriefing || !showBriefing || !SessionClass::IsCampaign())
		return 0;

	BriefingTemp::ShowBriefing = true;

	int theme = ScenarioExt::Global()->BriefingTheme;

	if (theme == -1)
	{
		SideClass* pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex);

		if (const auto pSideExt = SideExt::ExtMap.Find(pSide))
			theme = pSideExt->BriefingTheme;
	}

	if (theme != -1)
		ThemeClass::Instance->Queue(theme);

	// Skip over playing scenario theme.
	return SkipGameCode;
}

// Show the briefing dialog before entering game loop.
DEFINE_HOOK(0x48CE85, MainGame_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x48CE8A };

	// Restore overridden instructions.
	SessionClass::Instance->Resume();

	ShowBriefing();

	return SkipGameCode;
}

// Show the briefing dialog on starting a new scenario after clearing another.
DEFINE_HOOK(0x55D14F, AuxLoop_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x55D159 };

	// Restore overridden instructions.
	SessionClass::Instance->Resume();

	ShowBriefing();

	return SkipGameCode;
}

// Skip redrawing the screen if we're gonna show the briefing screen immediately after loading screen finishes on initially launched mission.
DEFINE_HOOK(0x683F66, PauseGame_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x683FAA };

	if (BriefingTemp::ShowBriefing)
		return SkipGameCode;

	return 0;
}

// Skip redrawing the screen if we're gonna show the briefing screen immediately after loading screen finishes on succeeding missions.
DEFINE_HOOK(0x685D95, DoWin_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x685D9F };

	if (BriefingTemp::ShowBriefing)
		return SkipGameCode;

	return 0;
}

// Set briefing dialog resume button text.
DEFINE_HOOK(0x65F764, BriefingDialog_ShowBriefing, 0x5)
{
	if (BriefingTemp::ShowBriefing)
	{
		GET(HWND, hDlg, ESI);

		auto const hResumeBtn = GetDlgItem(hDlg, 1059);
		SendMessageA(hResumeBtn, 1202, 0, reinterpret_cast<LPARAM>(Phobos::UI::ShowBriefingResumeButtonLabel));
	}

	return 0;
}

// Set briefing dialog resume button status bar label.
DEFINE_HOOK(0x604985, GetDialogUIStatusLabels_ShowBriefing, 0x5)
{
	if (BriefingTemp::ShowBriefing)
	{
		enum { SkipGameCode = 0x60498A };

		R->EAX(Phobos::UI::ShowBriefingResumeButtonStatusLabel);

		return SkipGameCode;
	}

	return 0;
}

#pragma endregion

bool __fastcall Fake_HouseIsAlliedWith(HouseClass* pThis, void*, HouseClass* CurrentPlayer)
{
	return (Phobos::Config::ShowPlanningPath && SessionClass::IsSingleplayer())
		|| pThis->IsControlledByCurrentPlayer()
		|| pThis->IsAlliedWith(CurrentPlayer);
}

DEFINE_JUMP(CALL, 0x63B136, GET_OFFSET(Fake_HouseIsAlliedWith));
DEFINE_JUMP(CALL, 0x63B100, GET_OFFSET(Fake_HouseIsAlliedWith));
DEFINE_JUMP(CALL, 0x63B17F, GET_OFFSET(Fake_HouseIsAlliedWith));
DEFINE_JUMP(CALL, 0x63B1BA, GET_OFFSET(Fake_HouseIsAlliedWith));
DEFINE_JUMP(CALL, 0x63B2CE, GET_OFFSET(Fake_HouseIsAlliedWith));

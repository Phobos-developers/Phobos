#include <Phobos.h>

#include <Helpers/Macro.h>
#include <PreviewClass.h>
#include <Surface.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x777C41, UI_ApplyAppIcon, 0x9)
{
	if (Phobos::AppIconPath != nullptr)
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
	auto const pPlayer = HouseClass::Player();
	if (!pPlayer || pPlayer->Defeated)
		return 0;

	if (Phobos::UI::ShowHarvesterCounter)
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

		RectangleStruct vRect = { 0, 0, 0, 0 };
		DSurface::Sidebar->GetRect(&vRect);

		DSurface::Sidebar->DrawText(counter, &vRect, &vPos, Drawing::RGB2DWORD(clrToolTip), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	}

	if (Phobos::UI::ShowPowerDelta)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];
		auto delta = pPlayer->PowerOutput - pPlayer->PowerDrain;

		double percent = pPlayer->PowerOutput != 0
			? (double)pPlayer->PowerDrain / (double)pPlayer->PowerOutput : pPlayer->PowerDrain != 0
			? Phobos::UI::PowerDelta_ConditionRed*2.f : Phobos::UI::PowerDelta_ConditionYellow;

		ColorStruct clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
			? pSideExt->Sidebar_PowerDelta_Green : LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
			? pSideExt->Sidebar_PowerDelta_Yellow : pSideExt->Sidebar_PowerDelta_Red;

		auto TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_PowerDelta_Align.Get()));

		swprintf_s(counter, L"%ls%+d", Phobos::UI::PowerLabel, delta);

		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 - 70 + pSideExt->Sidebar_PowerDelta_Offset.Get().X,
			2 + pSideExt->Sidebar_PowerDelta_Offset.Get().Y
		};

		RectangleStruct vRect = { 0, 0, 0, 0 };
		DSurface::Sidebar->GetRect(&vRect);

		DSurface::Sidebar->DrawText(counter, &vRect, &vPos, Drawing::RGB2DWORD(clrToolTip), 0, TextFlags);
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

	if (_stricmp(pFilename, "xxicon.shp")
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
	GET_STACK(TechnoTypeClass*, pLeft, STACK_OFFS(0x1C, 0x8));
	GET_STACK(TechnoTypeClass*, pRight, STACK_OFFS(0x1C, 0x4));
	GET_STACK(int, idxLeft, STACK_OFFS(0x1C, -0x8));
	GET_STACK(int, idxRight, STACK_OFFS(0x1C, -0x10));
	GET_STACK(AbstractType, rttiLeft, STACK_OFFS(0x1C, -0x4));
	GET_STACK(AbstractType, rttiRight, STACK_OFFS(0x1C, -0xC));
	auto pLeftTechnoExt = TechnoTypeExt::ExtMap.Find(pLeft);
	auto pRightTechnoExt = TechnoTypeExt::ExtMap.Find(pRight);
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

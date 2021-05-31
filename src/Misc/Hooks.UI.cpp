#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Surface.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Misc/Debug.h>

DEFINE_HOOK(777C41, UI_ApplyAppIcon, 9)
{
	if (Phobos::AppIconPath != nullptr) {
		Debug::Log("Applying AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(640B8D, LoadingScreen_DisableEmptySpawnPositions, 6)
{
	GET(bool, esi, ESI);
	if (Phobos::UI::DisableEmptySpawnPositions || !esi) {
		return 0x640CE2;
	}
	return 0x640B93;
}

//DEFINE_HOOK(640E78, LoadingScreen_DisableColorPoints, 6)
//{
//	return 0x641071;
//}

// Allow size = 0 for map previews
DEFINE_HOOK(641B41, LoadingScreen_SkipPreview, 8)
{
	GET(RectangleStruct*, pRect, EAX);
	if (pRect->Width > 0 && pRect->Height > 0) {
		return 0;
	}
	return 0x641D4E;
}

DEFINE_HOOK(4A25E0, CreditsClass_GraphicLogic_HarvesterCounter, 7)
{
	if (Phobos::UI::ShowHarvesterCounter) {
		auto pPlayer = HouseClass::Player;
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::Player->SideIndex));
		wchar_t counter[0x20];
		auto nActive = HouseExt::ActiveHarvesterCount(pPlayer);
		auto nTotal = HouseExt::TotalHarvesterCount(pPlayer);
		auto nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;

		ColorStruct clrToolTip = nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow
			? Drawing::TooltipColor : nPercentage > Phobos::UI::HarvesterCounter_ConditionRed
			? pSideExt->Sidebar_HarvesterCounter_Yellow : pSideExt->Sidebar_HarvesterCounter_Red;

		swprintf_s(counter, L"%ls%d/%d", Phobos::UI::HarvesterLabel, nActive, nTotal);
		
		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
		};

		RectangleStruct vRect = { 0, 0, 0, 0 };
		DSurface::Sidebar->GetRect(&vRect);

		DSurface::Sidebar->DrawTextA(counter, &vRect, &vPos,
			Drawing::RGB2DWORD(clrToolTip), 0, 0x4108);
	}

	return 0;
}
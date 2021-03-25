#include <Helpers/Macro.h>
#include <Surface.h>
#include <HouseClass.h>
#include <SideClass.h>
#include "../Phobos.h"
#include "../Utilities/Debug.h"
#include "../Ext/House/Body.h"
#include "../Ext/Side/Body.h"

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

DEFINE_HOOK(4A25E0, CreditsClass_GraphicLogic_HarvesterCounter, 7)
{
	if (Phobos::UI::ShowHarvesterCounter) {
		auto pPlayer = HouseClass::Player;
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::Player->SideIndex));
		wchar_t counter[0x20];
		ColorStruct& clrToolTip = *reinterpret_cast<ColorStruct*>(0xB0FA1C);

		swprintf_s(counter, L"%ls%d/%d", Phobos::UI::HarvesterLabel,
			HouseExt::ActiveHarvesterCount(pPlayer), HouseExt::TotalHarvesterCount(pPlayer));
		
		Point2D vPos = {
			DSurface::Sidebar->GetWidth() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
		};

		RectangleStruct vRect = { 0, 0, 0, 0 };
		DSurface::Sidebar->GetRect(&vRect);

		DSurface::Sidebar->DrawTextA(counter, &vRect, &vPos,
			((clrToolTip.R >> 3) << 11) + ((clrToolTip.G >> 2) << 5) + (clrToolTip.B >> 3), 0, 0x4108);
	}

	return 0;
}
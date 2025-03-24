#include "Body.h"
#include <HouseClass.h>
#include <ThemeClass.h>

// Ingame music switch when defeated
DEFINE_HOOK_AGAIN(0x4FCB7D, HouseClass_WinLoseTheme, 0x5)  // HouseClass::Flag_To_Win
DEFINE_HOOK(0x4FCD66, HouseClass_WinLoseTheme, 0x5)        // HouseClass::Flag_To_Lose
{
	HouseClass* pThis = HouseClass::CurrentPlayer;
	SideClass* pSide = SideClass::Array.GetItemOrDefault(pThis->SideIndex);
	auto pSideExt = SideExt::ExtMap.Find(pSide);
	int themeIndex = (pThis->IsWinner) ? pSideExt->IngameScore_WinTheme : pSideExt->IngameScore_LoseTheme;

	if (themeIndex >= 0)
			ThemeClass::Instance.Play(themeIndex);

	return 0;
}

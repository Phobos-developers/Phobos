#include <ScenarioClass.h>
#include <HouseClass.h>
#include <ThemeClass.h>

#include "Body.h"

// ingame music switch when defeated
DEFINE_HOOK_AGAIN(0x4FCB7D, HouseClass_WinLose_Theme, 0x5)
DEFINE_HOOK(0x4FCD66, HouseClass_WinLose_Theme, 0x5)
{
	HouseClass* pThis = HouseClass::CurrentPlayer;

	int PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (auto const pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		if (auto const pData = SideExt::ExtMap.Find(pSide)) {
			auto themeIndex = (pThis->IsWinner) ? pData->IngameScore_WinTheme : pData->IngameScore_LoseTheme;

			if (themeIndex >= 0) {
				ThemeClass::Instance->Play(themeIndex);
			}
		}
	}
	return 0;
}

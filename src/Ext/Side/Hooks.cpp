#include <ScenarioClass.h>
#include <HouseClass.h>
#include <Themes.h>

#include "Body.h"

// ingame music switch when defeated
DEFINE_HOOK_AGAIN(4FCB7D, HouseClass_WinLose_Theme, 5)
DEFINE_HOOK(4FCD66, HouseClass_WinLose_Theme, 5)
{
	HouseClass* pThis = HouseClass::Player;

	int PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (auto const pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		if (auto const pData = SideExt::ExtMap.Find(pSide)) {
			auto themeIndex = (pThis->IsWinner) ? pData->IngameScore_WinTheme : pData->IngameScore_LoseTheme;

			if (themeIndex >= 0) {
				ThemePlayer::Instance->Play(themeIndex);
			}
		}
	}
	return 0;
}

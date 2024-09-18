#include <Utilities/Macro.h>
#include <HouseClass.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x68804A, AssignHouses_PlayerHouses, 0x5)
{
	GET(HouseClass*, pPlayerHouse, EBP);

	HouseExt::SetSkirmishHouseName(pPlayerHouse);

	return 0x68808E;
}

DEFINE_HOOK(0x688210, AssignHouses_ComputerHouses, 0x5)
{
	GET(HouseClass*, pAiHouse, EBP);

	HouseExt::SetSkirmishHouseName(pAiHouse);

	return 0x688252;
}

// Skips checking the gamemode or who the player is when assigning houses
DEFINE_JUMP(LJMP, 0x44F8CB, 0x44F8E1)

#include <Helpers/Macro.h>
#include <DisplayClass.h>
#include <FootClass.h>
#include <SessionClass.h>
#include <HouseClass.h>

// ;; reveal
// 4ADFF0 = MapClass_RevealShroud, 5
// 577EBF = MapClass_Reveal, 6
// 586683 = CellClass_DiscoverTechno, 5
// 4FC1FF = HouseClass_AcceptDefeat_CleanShroudFog, 6

// MapClass_RevealShroud as Xkein said
DEFINE_HOOK(0x4ADFF0, DisplayClass_All_To_Look_Ground, 0x5)
{
	// GET(DisplayClass*, pDisplay, ECX);
	GET_STACK(DWORD, dwUnk, 0x4);
	GET_STACK(DWORD, dwUnk2, 0x8);

	for (auto pTechno : *TechnoClass::Array)
		if (pTechno && !dwUnk || pTechno->WhatAmI() != AbstractType::Building)
			if (pTechno->GetTechnoType()->RevealToAll ||
				(
					pTechno->DiscoveredByPlayer &&
					pTechno->Owner == HouseClass::Player() ||
					pTechno->Owner->CurrentPlayer ||
					pTechno->Owner->PlayerControl
					) ||
				(
					pTechno->Owner == HouseClass::Player() ||
					HouseClass::Player()->ArrayIndex != -1 &&
					pTechno->Owner->IsAlliedWith(HouseClass::Player())
					))
			{
				pTechno->See(0, dwUnk2);
				if (pTechno->IsInAir())
				{
					MapClass::Instance->RevealArea3(
						&pTechno->GetCoords(),
						pTechno->LastSightRange - 3,
						pTechno->LastSightRange + 3,
						false
					);
				}
			}


	return 0x4AE0A5;
}

DEFINE_HOOK(0x577EBF, MapClass_Reveal, 0x6)
{
	GET(CellClass*, pCell, EAX);

	pCell->ShroudCounter = 0;
	pCell->GapsCoveringThisCell = 0;
	pCell->CopyFlags |= 0x18u;
	pCell->Flags |= 0x3u;
	pCell->CleanFog();

	return 0x577EE9;
}

DEFINE_HOOK(0x586683, CellClass_DiscoverTechno, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);
	GET(CellClass*, pCell, ESI);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x8));

	bool bDiscovered = false;
	if (pTechno)
		bDiscovered = pTechno->DiscoveredBy(pHouse);
	if (auto const pJumpjet = pCell->Jumpjet)
		bDiscovered |= pJumpjet->DiscoveredBy(pHouse);
	R->EAX(bDiscovered);

	return 0x586696;
}

DEFINE_HOOK(0x4FC1FF, HouseClass_AcceptDefeat_CleanShroudFog, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	MapClass::Instance->Reveal(pHouse);

	return 0x4FC214;
}
#include "FogOfWar.h"

// ;; reveal
// 4ADFF0 = MapClass_RevealShroud, 5
// 577EBF = MapClass_Reveal, 6
// 586683 = CellClass_DiscoverTechno, 5
// 4FC1FF = HouseClass_AcceptDefeat_CleanShroudFog, 6



// MapClass_RevealShroud as Xkein said
DEFINE_HOOK(4ADFF0, DisplayClass_All_To_Look_Ground, 5) 
{
	// GET(DisplayClass*, pDisplay, ECX);
	GET_STACK(DWORD, dwUnk, 0x4);
	GET_STACK(DWORD, dwUnk2, 0x8);

	for (auto pTechno : *TechnoClass::Array)
		if (pTechno)
			FogOfWar::Reveal_DisplayClass_All_To_Look_Ground(pTechno, dwUnk, dwUnk2);

	return 0x4AE0A5;
}

DEFINE_HOOK(577EBF, MapClass_Reveal, 6)
{
	GET(CellClass*, pCell, EAX);

	pCell->ShroudCounter = 0;
	pCell->GapsCoveringThisCell = 0;
	pCell->CopyFlags |= 0x18u;
	pCell->Flags |= 3u;
	pCell->CellClass_CleanFog_CellClass_bitclear_0x400000();

	return 0x577EE9;
}

DEFINE_HOOK(586683, CellClass_DiscoverTechno, 5)
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

DEFINE_HOOK(4FC1FF, HouseClass_AcceptDefeat_CleanShroudFog, 6)
{
	GET(HouseClass*, pHouse, ESI);

	MapClass::Global()->Reveal(pHouse);

	return 0x4FC214;
}
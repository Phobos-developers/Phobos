#include <Helpers/Macro.h>

#include "Body.h"

#include <MapClass.h>

DEFINE_HOOK(6E9443, TeamClass_AI, 8) {
	GET(TeamClass *, pTeam, ESI);

	ScriptExt::ProcessAction(pTeam);
	
	return 0;
}

// Idea from E1Elite - secsome
DEFINE_HOOK(6E95B3, TeamClass_AI_MoveToCell, 6)
{
	if (!R->BL())
		return 0x6E95A4;

	GET(int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFS(0x38, 0x28));

	cell.X = static_cast<short>(nCoord % 1000);
	cell.Y = static_cast<short>(nCoord / 1000);

	R->EAX(MapClass::Instance->GetCellAt(cell));
	return 0x6E959C;
}
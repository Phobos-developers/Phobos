#include <Helpers/Macro.h>

#include "Body.h"

DEFINE_HOOK(6E9443, TeamClass_AI, 8) {
	GET(TeamClass *, pTeam, ESI);

	ScriptExt::ProcessAction(pTeam);
	
	return 0;
}

DEFINE_HOOK(6E95B7, TeamClass_AI_MoveToCell, 8)
{
	GET(int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFS(0x38, 0x28));

	cell.X = static_cast<short>(nCoord % 1000);
	cell.Y = static_cast<short>(nCoord / 1000);

	return 0x6E95CD;
}
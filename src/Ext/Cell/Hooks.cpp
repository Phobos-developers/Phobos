#include "Body.h"

#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x480E27, CellClass_DamageWall_DamageWallRecursivly, 0x5)
{
	enum { SkipGameCode = 0x480EBC };
	return RulesExt::Global()->DamageWallRecursivly ? 0 : SkipGameCode;
}

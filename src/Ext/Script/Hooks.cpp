#include <Helpers/Macro.h>

#include "Body.h"

DEFINE_HOOK(6E9443, TeamClass_Update, 8) {
	GET(TeamClass *, pTeam, ESI);

	ScriptExt::ProcessAction(pTeam);
	

	return 0;
}

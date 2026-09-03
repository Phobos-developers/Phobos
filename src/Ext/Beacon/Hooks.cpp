#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x5370A0, BeaconPlacementCommandClass_ExecuteSub_Start, 0x5)
{
	if (RulesExt::Global()->AllowBeaconHotKeyInSinglePlayer)
		return 0x5370AE;

	return 0;
}

DEFINE_JUMP(LJMP, 0x430CD4, 0x430CEA);

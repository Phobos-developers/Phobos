#include <BuildingClass.h>
#include <FootClass.h>

#include <Utilities/Macro.h>

// In vanilla YR, game destroys building animations directly by calling constructor.
// Ares changed this to call UnInit() which has a consequence of doing pointer invalidation on the AnimClass pointer.
// This notably causes an issue with Grinder that restores ActiveAnim if the building is sold/destroyed while SpecialAnim is playing even if the building is gone or in limbo.
// Now it does not do this if the building is in limbo, which covers all cases from being destroyed, sold, to erased by Temporal weapons.
// There is another potential case for this with ProductionAnim & IdleAnim which is also patched here just in case.
DEFINE_HOOK_AGAIN(0x44E997, BuildingClass_Detach_RestoreAnims, 0x6)
DEFINE_HOOK(0x44E9FA, BuildingClass_Detach_RestoreAnims, 0x6)
{
	enum { SkipAnimOne = 0x44E9A4, SkipAnimTwo = 0x44EA07 };

	GET(BuildingClass*, pThis, ESI);

	if (pThis->InLimbo)
		return R->Origin() == 0x44E997 ? SkipAnimOne : SkipAnimTwo;

	return 0;
}

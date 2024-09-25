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

// Ares make chrono miner cannot teleport again after repaired from repair bay for unknown reason
// This hook trying to fix it
DEFINE_HOOK(0x741B30, UnitClass_SetDestination_HarvesterFix, 0x7)
{
	GET(UnitClass*, pThis, EBP);

	if (pThis->GetCurrentMission() != Mission::Sleep)
		return 0x741BAA;

	if (const auto pLink = pThis->GetNthLink())
	{
		const auto pBuilding = abstract_cast<BuildingClass*>(pLink);

		if (pBuilding && pBuilding->Type->UnitRepair && pThis->Locomotor.GetInterfacePtr())
		{
			pThis->Locomotor->Power_On();
			pThis->QueueMission(Mission::Harvest, false);
		}

		return 0x741B4A;
	}
	else if (const auto pCell = pThis->GetCell())
	{
		const auto pBuilding = pCell->GetBuilding();

		if (pBuilding && pBuilding->Type->UnitRepair && pThis->Locomotor.GetInterfacePtr())
			pThis->Locomotor->Power_On();
	}

	return 0x741BAA;
}

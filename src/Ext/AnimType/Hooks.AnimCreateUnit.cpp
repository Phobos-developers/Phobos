#include <HouseClass.h>
#include <ScenarioClass.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

#include "Body.h"


// Anim Create Unit 
// Author : Otamaa


DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFS(0x44, -0x4));

	R->ECX(R->ESI());
	TechnoExt::ExtMap.Find(pThis)->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
	pThis->Destroy();

	return 0x737F74;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DesytroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();
		Location.Z = 0;
		pThis->MarkAllOccupationBits(Location);
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (auto Unit = pTypeExt->CreateUnit.Get())
	{
		auto DecidedOwner = HouseClass::FindCivilianSide();
		auto Owner = pThis->Owner;

		if (Owner && !Owner->Defeated)
			DecidedOwner = Owner;
		
		auto Location = pThis->GetCoords();
		Location.Z = 0;

		pThis->UnmarkAllOccupationBits(Location);

		if (auto pTechno = static_cast<TechnoClass*>(Unit->CreateObject(DecidedOwner)))
		{
			bool success = false;

			auto const AnimExt = AnimExt::ExtMap.Find(pThis);

			auto aFacing = pTypeExt->CreateUnit_Facing.Get();
				 aFacing = aFacing > 255 ? 255 : aFacing;
				 aFacing = aFacing <= -1 ? ScenarioClass::Instance->Random.RandomRanged(0, 255) : aFacing;

			auto FacingResult = (pTypeExt->CreateUnit_UseDeathFacings.Get() && AnimExt->Fromdeathunit) ? AnimExt->DeathUnitFacing : static_cast<short>(aFacing);

			if (!MapClass::Instance->TryGetCellAt(pThis->GetCoords())->GetBuilding())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pTechno->Put(Location, FacingResult);
				--Unsorted::IKnowWhatImDoing;
			}
			else
			{
				success = pTechno->Put(Location, FacingResult);
			}

			if (success)
			{
				if (pTechno->HasTurret() && AnimExt->Fromdeathunit && AnimExt->DeathUnitHasTurrent && pTypeExt->CreateUnit_useDeathTurrentFacings.Get())
					pTechno->SecondaryFacing.set(AnimExt->DeathUnitTurretFacing);

				Debug::Log("[" __FUNCTION__ "] Stored Turret Facing %d \n",AnimExt->DeathUnitTurretFacing.value256());

				if (!pTechno->InLimbo)
					pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);

				if (!DecidedOwner->Type->MultiplayPassive)
					DecidedOwner->RecheckTechTree = true;
			}
			else
			{
				if (pTechno)
					pTechno->UnInit();
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}
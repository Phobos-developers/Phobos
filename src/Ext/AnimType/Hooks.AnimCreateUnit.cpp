// Anim-to--Unit 
// Author: Otamaa

#include "Body.h"

#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

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

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
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

	if (auto unit = pTypeExt->CreateUnit.Get())
	{
		HouseClass* decidedOwner = (pThis->Owner)
			? pThis->Owner : HouseClass::FindCivilianSide();

		CoordStruct location = pThis->GetCoords();
		location.Z = 0;

		pThis->UnmarkAllOccupationBits(location);

		if (auto pTechno = static_cast<TechnoClass*>(unit->CreateObject(decidedOwner)))
		{
			bool success = false;
			auto const pExt = AnimExt::ExtMap.Find(pThis);

			auto aFacing = pTypeExt->CreateUnit_RandomFacing.Get() 
				? static_cast<unsigned short>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pTypeExt->CreateUnit_Facing.Get();

			short resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() && pExt->FromDeathUnit)
				? pExt->DeathUnitFacing : aFacing;

			if (!MapClass::Instance->TryGetCellAt(pThis->GetCoords())->GetBuilding())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pTechno->Unlimbo(location, resultingFacing);
				--Unsorted::IKnowWhatImDoing;
			}
			else
			{
				success = pTechno->Unlimbo(location, resultingFacing);
			}

			if (success)
			{
				if (pTechno->HasTurret() && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings.Get())
					pTechno->SecondaryFacing.set(pExt->DeathUnitTurretFacing);

				Debug::Log("[" __FUNCTION__ "] Stored Turret Facing %d \n",pExt->DeathUnitTurretFacing.value256());

				if (!pTechno->InLimbo)
					pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);

				if (!decidedOwner->Type->MultiplayPassive)
					decidedOwner->RecheckTechTree = true;
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
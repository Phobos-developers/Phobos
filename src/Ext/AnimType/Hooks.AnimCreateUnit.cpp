// Anim-to--Unit
// Author: Otamaa

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
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

		if (auto pCell = pThis->GetCell())
			Location = pCell->GetCoordsWithBridge();
		else
			Location.Z = MapClass::Instance->GetCellFloorHeight(Location);

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

		auto pCell = pThis->GetCell();
		CoordStruct location = pThis->GetCoords();

		if (pCell)
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);

		pThis->UnmarkAllOccupationBits(location);

		if (pTypeExt->CreateUnit_ConsiderPathfinding)
		{
			bool allowBridges = unit->SpeedType != SpeedType::Float;

			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
				unit->SpeedType, -1, unit->MovementZone, false, 1, 1, true,
				false, false, allowBridges, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			location = pThis->GetCoords();

			if (pCell)
				location = pCell->GetCoordsWithBridge();
			else
				location.Z = MapClass::Instance->GetCellFloorHeight(location);
		}

		if (auto pTechno = static_cast<TechnoClass*>(unit->CreateObject(decidedOwner)))
		{
			bool success = false;
			auto const pExt = AnimExt::ExtMap.Find(pThis);

			auto aFacing = pTypeExt->CreateUnit_RandomFacing.Get()
				? static_cast<unsigned short>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pTypeExt->CreateUnit_Facing.Get();

			short resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() && pExt->FromDeathUnit)
				? pExt->DeathUnitFacing : aFacing;

			if (pCell)
				pTechno->OnBridge = pCell->ContainsBridge();

			BuildingClass* pBuilding = pCell ? pCell->GetBuilding() : MapClass::Instance->TryGetCellAt(location)->GetBuilding();

			if (!pBuilding)
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

				Debug::Log("[" __FUNCTION__ "] Stored Turret Facing %d \n", pExt->DeathUnitTurretFacing.value256());

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

DEFINE_HOOK(0x469C98, BulletClass_DetonateAt_DamageAnimSelected, 0x0)
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	if (pAnim)
	{
		auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

		HouseClass* pInvoker = (pThis->Owner) ? pThis->Owner->Owner : nullptr;
		HouseClass* pVictim = nullptr;

		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
			pVictim = Target->Owner;

		if (auto unit = pTypeExt->CreateUnit.Get())
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);
		}
		else if (!pAnim->Owner)
		{
			auto const pExt = BulletExt::ExtMap.Find(pThis);
			pAnim->Owner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;
		}

		if (pThis->Owner)
		{
			auto pExt = AnimExt::ExtMap.Find(pAnim);
			pExt->Invoker = pThis->Owner;
		}
	}
	else if (pThis->WH == RulesClass::Instance->NukeWarhead)
	{
		return NukeWarheadExtras;
	}

	return Continue;
}

DEFINE_HOOK(0x6E2368, ActionClass_PlayAnimAt, 0x7)
{
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x4));

	if (pAnim)
	{
		auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

		if (auto unit = pTypeExt->CreateUnit.Get())
			AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pHouse, pHouse);
		else if (!pAnim->Owner && pHouse)
			pAnim->Owner = pHouse;
	}

	return 0;
}

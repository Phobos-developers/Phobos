#include <HouseClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <Ext/TechnoType/Body.h>

#include "Body.h"

DEFINE_HOOK(73880D, UnitClass_Destroy_DesytroyAnim, 6)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pType = pThis->Type;
	if (pType->DestroyAnim.Count > 0)
	{
		/* TODO:
			- Play anim based how much animation is entry and calculate it to select correct facings
			- Support for BuildingType (Possible patfind issues) and AircraftType (adding more tags make it bit messy)
			- Transfering passangers (General bus Logic)
		*/

		if (auto const AnimC = GameCreate<AnimClass>(pType->DestroyAnim[ScenarioClass::Instance->Random.Random() % pType->DestroyAnim.Count], pThis->Location, 0, 1, 0x600, 0, false))
		{
			AnimC->Owner = pThis->Owner;
			if (auto const TypeExt = AnimTypeExt::ExtMap.Find(AnimC->Type))
			{
				// TypeExt->FromDeathUnit = true;

				// Dont remap if owner defeated (prevent desyncs)
				if (TypeExt->CreateUnit_RemapAnim && !pThis->Owner->Defeated)
					AnimC->LightConvert = pThis->GetRemapColour();

				if (TechnoTypeExt::ExtMap.Find(pType)->StoreDeathFacings)
				{
					TypeExt->SavedData.FromDeathUnit = true;
					TypeExt->SavedData.UnitDeathFacing = pThis->Facing.current().value256();
					TypeExt->SavedData.SourceHasTurret = pThis->HasTurret();

					if (pThis->HasTurret())
						TypeExt->SavedData.UnitDeathTurretFacing = pThis->TurretFacing.current();
				}
			}
		}
	}

	return 0x73887E;
}

DEFINE_HOOK(423BC8, AnimClass_Update_CreateUnit_MarkOccBits, 6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
		pThis->MarkAllOccupationBits(pThis->Location + pTypeExt->CreateUnit_Offset);

	return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(424932, AnimClass_Update_CreateUnit_ActualAffects, 6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (auto Unit = pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->Location + pTypeExt->CreateUnit_Offset.Get();
		auto const Owner = pThis->Owner;
		pThis->UnmarkAllOccupationBits(Location);
		auto DecidedOwner = (!Owner || Owner->Defeated) ? HouseClass::FindCivilianSide() : Owner;

		if (auto const pTechno = static_cast<TechnoClass*>(Unit->CreateObject(DecidedOwner)))
		{
			auto BCell = MapClass::Instance->TryGetCellAt(Location);
			bool IsInAir = (Location.Z - BCell->GetCoordsWithBridge().Z > 2 * Unsorted::LevelHeight);
			auto Random = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
			auto aFacing = static_cast<short>(pTypeExt->CreateUnit_Facing.Get());
			aFacing = aFacing > 255 ? 255 : aFacing;
			aFacing = aFacing == -1 || aFacing < 0 ? Random : aFacing;
			auto Facing = (pTypeExt->CreateUnit_UseDeathFacings && pTypeExt->SavedData.FromDeathUnit) ? pTypeExt->SavedData.UnitDeathFacing : aFacing;
			bool isFlyingStuffs = pTechno->GetTechnoType()->BalloonHover || pTechno->GetTechnoType()->JumpJet;

			// Preset Properties
			pTechno->IsFallingDown = IsInAir && !isFlyingStuffs;
			pTechno->OnBridge = BCell->ContainsBridge();

			bool success = false;

			// dont allow to Force create when buildings below
			// may cause Unit become Invulnerable bug
			// TODO: Passable=yes building check
			if (pTypeExt->CreateUnit_Force.Get() && !BCell->GetBuilding())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pTechno->Put(Location, Facing);
				--Unsorted::IKnowWhatImDoing;
			}
			else
			{
				success = pTechno->Put(Location, Facing);
			}

			if (success)
			{
				if (pTechno->HasTurret() && pTypeExt->CreateUnit_UseDeathFacings &&
					pTypeExt->SavedData.FromDeathUnit && pTypeExt->SavedData.SourceHasTurret)
				{
					pTechno->TurretFacing.set(pTypeExt->SavedData.UnitDeathTurretFacing);
				}

				Debug::Log("[" __FUNCTION__ "] Stored Turret Facing %d \n",
					pTypeExt->SavedData.UnitDeathTurretFacing.value256());

				if (!pTechno->InLimbo)
				{
					pTechno->NeedsRedraw = true;
					pTechno->QueueMission(pTypeExt->CreateUnit_Mission, 0);
				}

				if (!DecidedOwner->Type->MultiplayPassive)
					DecidedOwner->RecheckTechTree = true;
			}
			else
			{
				if (pTechno)
					pTechno->UnInit();
				// GameDelete(pUnit);
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}

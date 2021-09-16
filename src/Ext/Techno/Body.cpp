#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>

#include <Ext/Team/Body.h>
#include <Ext/BulletType/Body.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (auto pVictimTechno = static_cast<TechnoClass*>(pVictim))
	{
		auto pVictimTechnoData = TechnoExt::ExtMap.Find(pVictim);

		if (pVictimTechnoData && pKiller)
		{
			TechnoClass* pObjectKiller;
			if ((pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner)
			{
				pObjectKiller = pKiller->SpawnOwner;
			}
			else
			{
				pObjectKiller = pKiller;
			}

			if (pObjectKiller && pObjectKiller->BelongsToATeam())
			{
				auto pKillerTechnoData = TechnoExt::ExtMap.Find(pObjectKiller);
				auto pFootKiller = abstract_cast<FootClass*>(pObjectKiller);
				auto pFocus = abstract_cast<TechnoClass*>(pFootKiller->Team->Focus);
				Debug::Log("DEBUG: pObjectKiller -> [%s] [%s] registered a kill of the type [%s]\n", pFootKiller->Team->Type->ID, pObjectKiller->get_ID(), pVictim->get_ID());

				pKillerTechnoData->LastKillWasTeamTarget = false;
				if (pFocus == pVictim)
				{
					pKillerTechnoData->LastKillWasTeamTarget = true;
				}

				// Conditional Jump Script Action stuff
				auto pKillerTeamData = TeamExt::ExtMap.Find(pFootKiller->Team);
				if (pKillerTeamData)
				{
					if (pKillerTeamData->KillsCountLimit >= 0)
						pKillerTeamData->KillsCounter++;

					if (pKillerTeamData && pKillerTeamData->ConditionalEvaluationType >= 0)
					{
						// Evaluate

						if (pKillerTeamData->ConditionalEvaluationType == 0 && pKillerTeamData->KillsCountLimit >= 0)
						{
							pKillerTeamData->ConditionalJumpEvaluation = false;

							// Comparators are like in [AITriggerTypes]
							switch (pKillerTeamData->ConditionalComparatorType)
							{
							case 0:
								// <
								if (pKillerTeamData->KillsCounter < pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							case 1:
								// <=
								if (pKillerTeamData->KillsCounter <= pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							case 2:
								// =
								if (pKillerTeamData->KillsCounter = pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							case 3:
								// >=
								if (pKillerTeamData->KillsCounter >= pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							case 4:
								// >
								if (pKillerTeamData->KillsCounter > pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							case 5:
								// !=
								if (pKillerTeamData->KillsCounter != pKillerTeamData->KillsCountLimit)
									pKillerTeamData->ConditionalJumpEvaluation = true;
								break;
							default:
								break;
							}
						}
					}
				}
			}
		}
	}
}

void TechnoExt::ApplyMindControlRangeLimit(TechnoClass* pThis)
{
	if (auto Capturer = pThis->MindControlledBy)
	{
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(Capturer->GetTechnoType());
		if (pCapturerExt && pCapturerExt->MindControlRangeLimit > 0
			&& pThis->DistanceFrom(Capturer) > pCapturerExt->MindControlRangeLimit * 256.0)
		{
			Capturer->CaptureManager->FreeUnit(pThis);
		}
	}
}

void TechnoExt::ApplyInterceptor(TechnoClass* pThis)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pData && pTypeData && pTypeData->Interceptor && !pThis->Target &&
		!(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
	{
		for (auto const& pBullet : *BulletClass::Array)
		{
			if (auto pBulletTypeData = BulletTypeExt::ExtMap.Find(pBullet->Type))
			{
				if (!pBulletTypeData->Interceptable)
					continue;
			}

			const double guardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteGuardRange * 256 : pTypeData->Interceptor_GuardRange * 256;
			const double minguardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteMinimumGuardRange * 256 : pTypeData->Interceptor_MinimumGuardRange * 256;

			double distance = pBullet->Location.DistanceFrom(pThis->Location);
			if (distance > guardRange || distance < minguardRange)
				continue;

			/*
			if (pBullet->Location.DistanceFrom(pBullet->TargetCoords) >
				double(ScenarioClass::Instance->Random.RandomRanged(128, (int)guardRange / 10)) * 10)
			{
				continue;
			}
			*/

			if (!pThis->Owner->IsAlliedWith(pBullet->Owner))
			{
				pThis->SetTarget(pBullet);
				pData->InterceptedBullet = pBullet;
				break;
			}
		}
	}
}

void TechnoExt::ApplyPowered_KillSpawns(TechnoClass* pThis)
{
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pThis->WhatAmI() == AbstractType::Building)
	{
		auto pBuilding = abstract_cast<BuildingClass*>(pThis);
		if (pTypeData->Powered_KillSpawns && pBuilding->Type->Powered && !pBuilding->IsPowerOnline())
		{
			if (auto pManager = pBuilding->SpawnManager)
			{
				pManager->ResetTarget();
				for (auto pItem : pManager->SpawnedNodes)
				{
					if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
					{
						pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
							RulesClass::Instance()->C4Warhead, nullptr, false, false, nullptr);
					}
				}
			}
		}
	}
}

void TechnoExt::ApplySpawn_LimitRange(TechnoClass* pThis)
{
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->Spawn_LimitedRange)
	{
		if (auto pManager = pThis->SpawnManager)
		{
			auto pTechnoType = pThis->GetTechnoType();
			int weaponRange = 0;
			int weaponRangeExtra = pTypeData->Spawn_LimitedExtraRange * 256;

			auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
			{
				if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
					weaponRange = pWeaponType->Range;
			};

			setWeaponRange(pTechnoType->Weapon[0].WeaponType);
			setWeaponRange(pTechnoType->Weapon[1].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[0].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[1].WeaponType);

			weaponRange += weaponRangeExtra;

			if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
				pManager->ResetTarget();
		}
	}
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!pThis || pThis->InLimbo)
		return false;

	auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExt::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld))
			return true;
	}

	return false;
}

void TechnoExt::InitializeLaserTrails(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->LaserTrails.size())
		return;

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		for (auto const& entry: pTypeExt->LaserTrailData)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[entry.idxType].get())
			{
				pExt->LaserTrails.push_back(std::make_unique<LaserTrailClass>(
					pLaserType, pThis->Owner, entry.FLH, entry.IsOnTurret));
			}
		}
	}
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();
	auto const pFoot = abstract_cast<FootClass*>(pThis);
	Matrix3D mtx;

	// Step 1: get body transform matrix
	if (pFoot && pFoot->Locomotor)
		mtx = pFoot->Locomotor->Draw_Matrix(nullptr);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx);

		double turretRad = (pThis->TurretFacing().value32() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.current().value32() - 8) * -(Math::Pi / 16);
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate((float)pCoord.X, (float)pCoord.Y, (float)pCoord.Z);

	Vector3D<float> result = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { (int)result.X, (int)result.Y, (int)result.Z };

	return location;
}

CoordStruct TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (!pThis || weaponIndex < 0)
		return FLH;

	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pThis->Veterancy.IsElite())
	{
		if (pExt->EliteWeaponBurstFLHs[weaponIndex].Count > pThis->CurrentBurstIndex)
		{
			FLHFound = true;
			FLH = pExt->EliteWeaponBurstFLHs[weaponIndex][pThis->CurrentBurstIndex];
		}
	}
	else
	{
		if (pExt->WeaponBurstFLHs[weaponIndex].Count > pThis->CurrentBurstIndex)
		{
			FLHFound = true;
			FLH = pExt->WeaponBurstFLHs[weaponIndex][pThis->CurrentBurstIndex];
		}
	}

	return FLH;
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->InterceptedBullet)
		.Process(this->Shield)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->LastKillWasTeamTarget)
		;
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") { }

TechnoExt::ExtContainer::~ExtContainer() = default;

void TechnoExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);

	TechnoExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExt::ExtMap.SaveStatic();

	return 0;
}
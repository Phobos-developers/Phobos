#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>

#include <AircraftClass.h>
#include <TacticalClass.h>

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);

	R->AL(CaptureManagerExt::CaptureUnit(pThis->Owner->CaptureManager, pThis->Target, pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4690D4, BulletClass_Logics_ScreenShake, 0x6)
{
	enum { SkipShaking = 0x469130 };

	GET(WarheadTypeClass*, pWarhead, EAX);
	GET_BASE(CoordStruct*, pCoords, 0x8);

	if (auto const pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		auto&& [_, visible] = TacticalClass::Instance->CoordsToClient(*pCoords);

		if (pExt->ShakeIsLocal && !visible)
			return SkipShaking;
	}

	return 0;
}

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (!pHouse)
			R->ECX(pExt->FirerHouse);
	}

	return 0;
}

#pragma region DetonateOnAllMapObjects

static __forceinline void TryDetonateFull(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExt::ExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
	{
		pThis->Target = pTechno;
		pThis->Location = pTechno->GetCoords();
		pThis->Detonate(pTechno->GetCoords());
	}
}

static __forceinline void TryDetonateDamageArea(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExt::ExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
	{
		int damage = (pThis->Health * pThis->DamageMultiplier) >> 8;
		pWHExt->DamageAreaWithTarget(pTechno->GetCoords(), damage, pThis->Owner, pThis->WH, true, pOwner, pTechno);
	}
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects &&
		pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
		pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
	{
		pWHExt->WasDetonatedOnAllMapObjects = true;
		auto const originalLocation = pThis->Location;
		auto const pOriginalTarget = pThis->Target;
		auto const pExt = BulletExt::ExtMap.Find(pThis);
		auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;

		auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
		{
			std::vector<T> vec(dvc.Count);
			std::copy(dvc.begin(), dvc.end(), vec.begin());
			return vec;
		};

		void (*tryDetonate)(BulletClass*, TechnoClass*, WarheadTypeExt::ExtData*, HouseClass*) = TryDetonateFull;

		if (!pWHExt->DetonateOnAllMapObjects_Full)
			tryDetonate = TryDetonateDamageArea;

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
		{
			auto const aircraft = copy_dvc(*AircraftClass::Array);

			for (auto pAircraft : aircraft)
				tryDetonate(pThis, pAircraft, pWHExt, pOwner);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
		{
			auto const buildings = copy_dvc(*BuildingClass::Array);

			for (auto pBuilding : buildings)
				tryDetonate(pThis, pBuilding, pWHExt, pOwner);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
		{
			auto const infantry = copy_dvc(*InfantryClass::Array);

			for (auto pInf : infantry)
				tryDetonate(pThis, pInf, pWHExt, pOwner);
		}

		if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
		{
			auto const units = copy_dvc(*UnitClass::Array);

			for (auto const pUnit : units)
				tryDetonate(pThis, pUnit, pWHExt, pOwner);
		}

		pThis->Target = pOriginalTarget;
		pThis->Location = originalLocation;
		pWHExt->WasDetonatedOnAllMapObjects = false;

		return ReturnFromFunction;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	bool isLand = pThis->GetCell()->LandType != LandType::Water;

	if (!isLand && pWHExt->Debris_Conventional)
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

DEFINE_HOOK(0x469E34, BulletClass_Logics_DebrisAnims, 0x5)
{
	enum { SkipGameCode = 0x469EBA };

	GET(BulletClass*, pThis, ESI);
	GET(int, debrisCount, EBX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto debrisAnims = pWHExt->DebrisAnims.GetElements(RulesClass::Instance->MetallicDebris);

	if (debrisAnims.size() < 1)
		return SkipGameCode;

	while (debrisCount > 0)
	{
		int debrisIndex = ScenarioClass::Instance->Random.RandomRanged(0, debrisAnims.size() - 1);

		auto anim = GameCreate<AnimClass>(debrisAnims[debrisIndex], pThis->GetCoords());

		if (anim && pThis->Owner)
			anim->Owner = pThis->Owner->Owner;

		debrisCount--;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x469B44, BulletClass_Logics_LandTypeCheck, 0x6)
{
	enum { SkipChecks = 0x469BA2 };

	GET(BulletClass*, pThis, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pWHExt->Conventional_IgnoreUnits)
		return SkipChecks;

	return 0;
}

DEFINE_HOOK(0x469C46, BulletClass_Logics_DamageAnimSelected, 0x8)
{
	enum { SkipGameCode = 0x469C98 };

	GET(BulletClass*, pThis, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);
	LEA_STACK(CoordStruct*, coords, STACK_OFFSET(0xA4, -0x40));

	bool createdAnim = false;

	if (pAnimType)
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
		int creationInterval = pWHExt->Splashed ? pWHExt->SplashList_CreationInterval : pWHExt->AnimList_CreationInterval;
		int* remainingInterval = &pWHExt->RemainingAnimCreationInterval;
		int scatterMin = pWHExt->Splashed ? pWHExt->SplashList_ScatterMin.Get() : pWHExt->AnimList_ScatterMin.Get();
		int scatterMax = pWHExt->Splashed ? pWHExt->SplashList_ScatterMax.Get() : pWHExt->AnimList_ScatterMax.Get();
		bool allowScatter = scatterMax != 0 || scatterMin != 0;

		if (creationInterval > 0 && pThis->Owner)
			remainingInterval = &TechnoExt::ExtMap.Find(pThis->Owner)->WHAnimRemainingCreationInterval;

		if (creationInterval < 1 || *remainingInterval <= 0)
		{
			*remainingInterval = creationInterval;

			HouseClass* pInvoker = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			HouseClass* pVictim = nullptr;

			if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
				pVictim = Target->Owner;

			auto types = make_iterator_single(pAnimType);

			if (pWHExt->SplashList_CreateAll && pWHExt->Splashed)
				types = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList);
			else if (pWHExt->AnimList_CreateAll && !pWHExt->Splashed)
				types = pWHExt->OwnerObject()->AnimList;

			for (auto const& pType : types)
			{
				if (!pType)
					continue;

				auto animCoords = *coords;

				if (allowScatter)
				{
					int distance = ScenarioClass::Instance->Random.RandomRanged(scatterMin, scatterMax);
					animCoords = MapClass::GetRandomCoordsNear(animCoords, distance, false);
				}

				if (auto const pAnim = GameCreate<AnimClass>(pType, animCoords, 0, 1, 0x2600, -15, false))
				{
					createdAnim = true;

					AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

					if (!pAnim->Owner)
						pAnim->Owner = pInvoker;

					if (pThis->Owner)
					{
						auto pExt = AnimExt::ExtMap.Find(pAnim);
						pExt->SetInvoker(pThis->Owner);
					}
				}
			}
		}
		else
		{
			(*remainingInterval)--;
		}
	}

	R->EAX(createdAnim);
	return SkipGameCode;
}

DEFINE_HOOK(0x469AA4, BulletClass_Logics_Extras, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct*, coords, 0x8);

	// Extra warheads
	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			int damage = defaultDamage;
			size_t size = pWeaponExt->ExtraWarheads_DamageOverrides.size();

			if (size > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];
			else if (size > 0)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[size - 1];

			bool detonate = true;
			size = pWeaponExt->ExtraWarheads_DetonationChances.size();

			if (size > i)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[i] >= ScenarioClass::Instance->Random.RandomDouble();
			if (size > 0)
				detonate = pWeaponExt->ExtraWarheads_DetonationChances[size - 1] >= ScenarioClass::Instance->Random.RandomDouble();

			bool isFull = true;
			size = pWeaponExt->ExtraWarheads_FullDetonation.size();

			if (size > i)
				isFull = pWeaponExt->ExtraWarheads_FullDetonation[i];
			if (size > 0)
				isFull = pWeaponExt->ExtraWarheads_FullDetonation[size - 1];

			if (detonate)
			{
				if (isFull)
				{
					WarheadTypeExt::DetonateAt(pWH, *coords, pThis->Owner, damage, pOwner, pThis->Target);
				}
				else
				{
					auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
					pWHExt->DamageAreaWithTarget(*coords, damage, pThis->Owner, pWH, true, pOwner, abstract_cast<TechnoClass*>(pThis->Target));
				}
			}
		}
	}

	// Return to sender
	if (pThis->Type && pThis->Owner)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

		if (auto const pWeapon = pTypeExt->ReturnWeapon)
		{
			if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pThis->Owner, pThis->Owner,
				pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
			{
				pBullet->WeaponType = pWeapon;
				auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
				pBulletExt->FirerHouse = pBulletExt->FirerHouse;

				pBullet->MoveTo(pThis->Location, BulletVelocity::Empty);
			}
		}
	}

	WarheadTypeExt::ExtMap.Find(pThis->WH)->InDamageArea = true;

	return 0;
}

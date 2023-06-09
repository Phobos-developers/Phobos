#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
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
		Point2D screenCoords;

		if (pExt->ShakeIsLocal && !TacticalClass::Instance->CoordsToClient(*pCoords, &screenCoords))
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

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects &&
			pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
			pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			auto const pExt = BulletExt::ExtMap.Find(pThis);
			auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;

			auto tryDetonate = [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					pThis->Target = pTechno;
					pThis->Detonate(pTechno->GetCoords());
				}
			};

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				for (auto pTechno : *AircraftClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
			{
				for (auto pTechno : *BuildingClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				for (auto pTechno : *InfantryClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
			{
				for (auto pTechno : *UnitClass::Array)
					tryDetonate(pTechno);
			}

			pWHExt->WasDetonatedOnAllMapObjects = false;

			return ReturnFromFunction;
		}
	}

	return 0;
}

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

DEFINE_HOOK(0x469C98, BulletClass_DetonateAt_DamageAnimSelected, 0x0)
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	if (pAnim)
	{
		auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

		HouseClass* pInvoker = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
		HouseClass* pVictim = nullptr;

		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
			pVictim = Target->Owner;

		AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

		if (!pAnim->Owner)
			pAnim->Owner = pInvoker;

		if (pThis->Owner)
		{
			auto pExt = AnimExt::ExtMap.Find(pAnim);
			pExt->SetInvoker(pThis->Owner);
		}
	}
	else if (pThis->WH == RulesClass::Instance->NukeWarhead)
	{
		return NukeWarheadExtras;
	}

	return Continue;
}


DEFINE_HOOK(0x46A290, BulletClass_Logics_ExtraWarheads, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct*, coords, 0x8);

	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			int damage = defaultDamage;

			if (pWeaponExt->ExtraWarheads_DamageOverrides.size() > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];

			WarheadTypeExt::DetonateAt(pWH, *coords, pThis->Owner, damage, pOwner);
		}
	}

	return 0;
}

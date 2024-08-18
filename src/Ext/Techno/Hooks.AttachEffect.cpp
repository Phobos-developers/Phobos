#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x4DB218, FootClass_GetMovementSpeed_SpeedMultiplier, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(int, speed, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	speed = static_cast<int>(speed * pExt->AE.SpeedMultiplier);
	R->EAX(speed);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FDC87, TechnoClass_ArmorMultiplier, 0x6) // TechnoClass_AdjustDamage
DEFINE_HOOK(0x701966, TechnoClass_ArmorMultiplier, 0x6)       // TechnoClass_ReceiveDamage
{
	TechnoClass* pThis = R->Origin() == 0x701966 ? R->ESI<TechnoClass*>() : R->EDI<TechnoClass*>();
	GET(int, damage, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	damage = static_cast<int>(damage / pExt->AE.ArmorMultiplier);
	R->EAX(damage);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FDBE2, TechnoClass_FirepowerMultiplier, 0x6) // TechnoClass_AdjustDamage
DEFINE_HOOK(0x6FE352, TechnoClass_FirepowerMultiplier, 0x8)       // TechnoClass_FireAt
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, damage, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	damage = static_cast<int>(damage * pExt->AE.FirepowerMultiplier);
	R->EAX(damage);

	return 0;
}

bool __fastcall TechnoClass_Limbo_Wrapper(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	bool markForRedraw = false;
	bool altered = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;

	// Do not remove attached effects from undeploying buildings.
	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		if (pBuilding->Type->UndeploysInto && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus == 2)
			return pThis->TechnoClass::Limbo();
	}

	for (it = pExt->AttachedEffects.begin(); it != pExt->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();

		if ((attachEffect->GetType()->DiscardOn & DiscardCondition::Entry) != DiscardCondition::None)
		{
			altered = true;

			if (attachEffect->GetType()->HasTint())
				markForRedraw = true;

			if (attachEffect->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = pExt->AttachedEffects.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (altered)
		pExt->RecalculateStatMultipliers();

	if (markForRedraw)
		pExt->OwnerObject()->MarkForRedraw();

	return pThis->TechnoClass::Limbo();
}

DEFINE_JUMP(VTABLE, 0x7F4A34, GET_OFFSET(TechnoClass_Limbo_Wrapper)); // TechnoClass
DEFINE_JUMP(CALL, 0x4DB3B1, GET_OFFSET(TechnoClass_Limbo_Wrapper));   // FootClass
DEFINE_JUMP(CALL, 0x445DDA, GET_OFFSET(TechnoClass_Limbo_Wrapper))    // BuildingClass

DEFINE_HOOK(0x702050, TechnoClass_ReceiveDamage_AttachEffectExpireWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	std::set<AttachEffectTypeClass*> cumulativeTypes;
	std::vector<WeaponTypeClass*> expireWeapons;

	for (auto const& attachEffect : pExt->AttachedEffects)
	{
		auto const pType = attachEffect->GetType();

		if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Death) != ExpireWeaponCondition::None)
		{
			if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || !cumulativeTypes.contains(pType))
			{
				if (pType->Cumulative && pType->ExpireWeapon_CumulativeOnlyOnce)
					cumulativeTypes.insert(pType);

				expireWeapons.push_back(pType->ExpireWeapon);
			}
		}
	}

	auto const coords = pThis->GetCoords();
	auto const pOwner = pThis->Owner;

	for (auto const& pWeapon : expireWeapons)
	{
		WeaponTypeExt::DetonateAt(pWeapon, coords, pThis, pOwner, pThis);
	}

	return 0;
}

DEFINE_HOOK(0x701E18, TechnoClass_ReceiveDamage_ReflectDamage, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int*, pDamage, EBX);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));
	GET_STACK(HouseClass*, pSourceHouse, STACK_OFFSET(0xC4, 0x1C));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (pWHExt->Reflected)
		return 0;

	if (pExt->AE.ReflectDamage && *pDamage > 0 && (!pWHExt->SuppressReflectDamage || pWHExt->SuppressReflectDamage_Types.size() > 0))
	{
		for (auto& attachEffect : pExt->AttachedEffects)
		{
			if (!attachEffect->IsActive())
				continue;

			auto const pType = attachEffect->GetType();

			if (!pType->ReflectDamage)
				continue;

			if (pWHExt->SuppressReflectDamage && pWHExt->SuppressReflectDamage_Types.Contains(pType))
				continue;

			auto const pWH = pType->ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
			int damage = static_cast<int>(*pDamage * pType->ReflectDamage_Multiplier);

			if (EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pThis->Owner, pSourceHouse))
			{
				auto const pWHExtRef = WarheadTypeExt::ExtMap.Find(pWH);
				pWHExtRef->Reflected = true;

				if (pType->ReflectDamage_Warhead_Detonate)
					WarheadTypeExt::DetonateAt(pWH, pSource, pThis, damage, pThis->Owner);
				else
					pSource->ReceiveDamage(&damage, 0, pWH, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}
		}
	}

	return 0;
}

#include "Body.h"

DEFINE_HOOK(0x4DB218, FootClass_GetMovementSpeed_SpeedMultiplier, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(int, speed, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	speed = static_cast<int>(speed * pExt->AE_SpeedMultiplier);
	R->EAX(speed);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FDC87, TechnoClass_ArmorMultiplier, 0x6) // TechnoClass_AdjustDamage
DEFINE_HOOK(0x701966, TechnoClass_ArmorMultiplier, 0x6)       // TechnoClass_ReceiveDamage
{
	TechnoClass* pThis = R->Origin() == 0x701966 ? R->ESI<TechnoClass*>() : R->EDI<TechnoClass*>();
	GET(int, damage, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	damage = static_cast<int>(damage / pExt->AE_ArmorMultiplier);
	R->EAX(damage);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FDBE2, TechnoClass_FirepowerMultiplier, 0x6) // TechnoClass_AdjustDamage
DEFINE_HOOK(0x6FE352, TechnoClass_FirepowerMultiplier, 0x8)       // TechnoClass_FireAt
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, damage, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	damage = static_cast<int>(damage * pExt->AE_FirepowerMultiplier);
	R->EAX(damage);

	return 0;
}

bool __fastcall TechnoClass_IsReadyToCloak_Wrapper(TechnoClass* pThis)
{
	bool cloak = pThis->Cloakable;
	TechnoExt::ExtData* pExt = nullptr;

	if (!pThis->Cloakable)
	{
		pExt = TechnoExt::ExtMap.Find(pThis);
		pThis->Cloakable = pExt->AE_Cloakable;
	}

	bool retVal = pThis->TechnoClass::IsReadyToCloak();
	pThis->Cloakable = cloak;

	if (retVal)
	{
		if (!pExt)
			pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->AE_ForceDecloak)
			return false;
	}

	return retVal;
}

bool __fastcall TechnoClass_ShouldNotCloak_Wrapper(TechnoClass* pThis)
{
	bool cloak = pThis->Cloakable;
	TechnoExt::ExtData* pExt = nullptr;

	if (!pThis->Cloakable)
	{
		pExt = TechnoExt::ExtMap.Find(pThis);
		pThis->Cloakable = pExt->AE_Cloakable;
	}

	bool retVal = pThis->TechnoClass::ShouldNotBeCloaked();
	pThis->Cloakable = cloak;

	if (retVal)
	{
		if (!pExt)
			pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->AE_ForceDecloak)
			return true;
	}

	return retVal;
}

DEFINE_JUMP(VTABLE, 0x7E2544, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // AircraftClass
DEFINE_JUMP(VTABLE, 0x7E8F34, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // FootClass
DEFINE_JUMP(VTABLE, 0x7EB2F8, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4C00, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5F10, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // UnitClass
DEFINE_JUMP(CALL, 0x457779, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper))    // BuildingClass

DEFINE_JUMP(VTABLE, 0x7E2548, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // AircraftClass
DEFINE_JUMP(VTABLE, 0x7E8F38, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // FootClass
DEFINE_JUMP(VTABLE, 0x7EB2FC, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4C04, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5F14, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper)); // UnitClass
DEFINE_JUMP(CALL, 0x4578C9, GET_OFFSET(TechnoClass_ShouldNotCloak_Wrapper));   // BuildingClass

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

DEFINE_HOOK(0x702050, TechnoClass_TakeDamage_AttachEffectExpireWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	std::set<AttachEffectTypeClass*> cumulativeTypes;

	for (auto const& attachEffect : pExt->AttachedEffects)
	{
		auto const pType = attachEffect->GetType();

		if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Death) != ExpireWeaponCondition::None)
		{
			if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || !cumulativeTypes.contains(pType))
			{
				if (pType->Cumulative && pType->ExpireWeapon_CumulativeOnlyOnce)
					cumulativeTypes.insert(pType);

				attachEffect->ExpireWeapon();
			}
		}
	}

	return 0;
}

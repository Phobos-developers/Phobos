#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x8)
{
	GET(AnimClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pType, EDX);

	if (pType->HideIfNoOre)
	{
		auto nThreshold = abs(AnimTypeExt::ExtMap.Find(pThis->Type)->HideIfNoOre_Threshold.Get());
		auto pCell = pThis->GetCell();

		pThis->Invisible = !pCell || pCell->GetContainedTiberiumValue() <= nThreshold;
	}

	return 0x423BBF;
}

// Goes before and replaces Ares animation damage / weapon hook at 0x424538.
DEFINE_HOOK(0x424513, AnimClass_AI_Damage, 0x6)
{
	enum { SkipDamage = 0x42465D, Continue = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	if (pThis->Type->Damage <= 0.0 || pThis->HasExtras)
		return SkipDamage;

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int delay = pTypeExt->Damage_Delay.Get();
	int damageMultiplier = 1;
	bool adjustAccum = false;
	double damage = 0;
	int appliedDamage = 0;

	if (pThis->OwnerObject && pThis->OwnerObject->WhatAmI() == AbstractType::Terrain)
		damageMultiplier = 5;

	if (pTypeExt->Damage_ApplyOncePerLoop) // If damage is to be applied only once per animation loop
	{
		if (pThis->Animation.Value == std::max(delay - 1, 1))
			appliedDamage = static_cast<int>(std::round(pThis->Type->Damage)) * damageMultiplier;
		else
			return SkipDamage;
	}
	else if (delay <= 0 || pThis->Type->Damage < 1.0) // If Damage.Delay is less than 1 or Damage is a fraction.
	{
		adjustAccum = true;
		damage = damageMultiplier * pThis->Type->Damage + pThis->Accum;
		pThis->Accum = damage;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (damage >= 1.0)
			appliedDamage = static_cast<int>(std::round(damage));
		else
			return SkipDamage;
	}
	else
	{
		// Accum here is used as a counter for Damage.Delay, which cannot deal fractional damage.
		damage = pThis->Accum + 1.0;
		pThis->Accum = damage;

		if (damage < delay)
			return SkipDamage;

		// Use Type->Damage as the actually dealt damage.
		appliedDamage = static_cast<int>(std::round(pThis->Type->Damage)) * damageMultiplier;
	}

	if (appliedDamage <= 0 || pThis->IsPlaying)
		return SkipDamage;

	// Store fractional damage if needed, or reset the accum if hit the Damage.Delay counter.
	if (adjustAccum)
		pThis->Accum = damage - appliedDamage;
	else
		pThis->Accum = 0.0;

	TechnoClass* pInvoker = nullptr;
	HouseClass* pInvokerHouse = nullptr;

	if (pTypeExt->Damage_DealtByInvoker)
	{
		auto const pExt = AnimExt::ExtMap.Find(pThis);
		pInvoker = pExt->Invoker;

		if (!pInvoker)
		{
			pInvoker = pThis->OwnerObject ? abstract_cast<TechnoClass*>(pThis->OwnerObject) : nullptr;
			pInvokerHouse = !pInvoker ? pExt->InvokerHouse : nullptr;
		}
	}

	if (pTypeExt->Weapon.isset())
	{
		WeaponTypeExt::DetonateAt(pTypeExt->Weapon.Get(), pThis->GetCoords(), pInvoker, appliedDamage, pInvokerHouse);
	}
	else
	{
		auto pWarhead = pThis->Type->Warhead;

		if (!pWarhead)
			pWarhead = strcmp(pThis->Type->get_ID(), "INVISO") ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		auto pOwner = pInvoker ? pInvoker->Owner : nullptr;

		if (!pOwner)
		{
			if (pThis->Owner)
				pOwner = pThis->Owner;
			else if (pThis->OwnerObject)
				pOwner = pThis->OwnerObject->GetOwningHouse();
		}

		MapClass::DamageArea(pThis->GetCoords(), appliedDamage, pInvoker, pWarhead, true, pOwner);
	}

	return Continue;
}

DEFINE_HOOK(0x424322, AnimClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(AnimClass*, pTrailerAnim, EAX);

	if (pThis->Type->TrailerAnim && pThis->Type->TrailerSeperation > 0 &&
		Unsorted::CurrentFrame % pThis->Type->TrailerSeperation == 0)
	{
		if (auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim))
		{
			auto pExt = AnimExt::ExtMap.Find(pThis);
			pTrailerAnim->Owner = pThis->Owner;
			pTrailerAnimExt->Invoker = pExt->Invoker;
			pTrailerAnimExt->InvokerHouse = pExt->InvokerHouse;
		}
	}

	return 0;
}

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFSET(0x100, 0x4));

	if (auto const pThisTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		pCoord->X += pThisTypeExt->XDrawOffset;

	return 0;
}

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(AnimClass*, pThis, ECX);

	auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->OwnerObject && pExt->Layer_UseObjectLayer.isset())
	{
		Layer layer = pThis->Type->Layer;

		if (pExt->Layer_UseObjectLayer.Get())
			layer = pThis->OwnerObject->InWhichLayer();

		R->EAX(layer);

		return ReturnValue;
	}

	return 0;
}

DEFINE_HOOK(0x424C3D, AnimClass_AttachTo_CenterCoords, 0x6)
{
	enum { SkipGameCode = 0x424C76 };

	GET(AnimClass*, pThis, ESI);

	auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->UseCenterCoordsIfAttached)
	{
		pThis->SetLocation(CoordStruct::Empty);

		return SkipGameCode;
	}

	return 0;
}

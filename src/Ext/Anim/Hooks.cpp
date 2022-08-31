#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/WeaponType/Body.h>

// Set in AnimClass::AI and guaranteed to be valid within it.
namespace AnimAITemp
{
	AnimExt::ExtData* ExtData;
	AnimTypeExt::ExtData* TypeExtData;
}

DEFINE_HOOK(0x423AD2, AnimClass_AI_SetContext, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pExt = AnimExt::ExtMap.Find(pThis);

	AnimAITemp::ExtData = pExt;
	AnimAITemp::TypeExtData = pExt->TypeExtData;

	return 0;
}

DEFINE_HOOK(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x8)
{
	GET(AnimClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pType, EDX);

	if (pType->HideIfNoOre)
	{
		auto nThreshold = abs(AnimAITemp::TypeExtData->HideIfNoOre_Threshold.Get());
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

	auto const pTypeExt = AnimAITemp::TypeExtData;
	int delay = pTypeExt->Damage_Delay.Get();

	int damageMultiplier = 1;

	if (pThis->OwnerObject && pThis->OwnerObject->WhatAmI() == AbstractType::Terrain)
		damageMultiplier = 5;

	bool adjustAccum = false;
	double damage = 0;
	int appliedDamage = 0;

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

	auto const pExt = AnimAITemp::ExtData;
	TechnoClass* pInvoker = nullptr;

	if (pTypeExt->Damage_DealtByInvoker)
	{
		pInvoker = pExt->Invoker;

		if (!pInvoker)
			pInvoker = pThis->OwnerObject ? abstract_cast<TechnoClass*>(pThis->OwnerObject) : nullptr;
	}

	if (pTypeExt->Weapon.isset())
	{
		WeaponTypeExt::DetonateAt(pTypeExt->Weapon.Get(), pThis->GetCoords(), pInvoker, appliedDamage);
	}
	else
	{
		auto pWarhead = pThis->Type->Warhead;

		if (!pWarhead)
			pWarhead = strcmp(pThis->Type->get_ID(), "INVISO") ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		auto pOwner = pInvoker ? pInvoker->Owner : nullptr;

		if (!pOwner)
			pOwner = pThis->OwnerObject ? pThis->OwnerObject->GetOwningHouse() : nullptr;

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
			pTrailerAnim->Owner = pThis->Owner;
			pTrailerAnimExt->Invoker = AnimAITemp::ExtData->Invoker;
		}
	}

	return 0;
}

DEFINE_HOOK(0x424807, AnimClass_AI_Next, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	const auto pExt = AnimAITemp::ExtData;

	// Update type data after anim type changes
	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pThis->Type)
	{
		pExt->TypeExtData = AnimTypeExt::ExtMap.Find(pThis->Type);
		AnimAITemp::TypeExtData = pExt->TypeExtData;
	}

	return 0;
}

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));

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

DEFINE_HOOK(0x424C49, AnimClass_AttachTo_BuildingCoords, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);
	GET(CoordStruct*, pCoords, EAX);

	auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->UseCenterCoordsIfAttached)
	{
		pCoords = pObject->GetCenterCoord(pCoords);
		pCoords->X += 128;
		pCoords->Y += 128;
	}

	return 0;
}

#include "Body.h"

#include <ScenarioClass.h>
#include <WarheadTypeClass.h>

#include <Ext/AnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

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

// Nuke Ares' animation damage hook at 0x424538.
DEFINE_PATCH(0x424538, 0x8B, 0x8E, 0xCC, 0x00, 0x00, 0x00);

// And add the new one after that.
DEFINE_HOOK(0x42453E, AnimClass_AI_Damage, 0x6)
{
	enum { SkipDamage = 0x42465D, Continue = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int delay = pTypeExt->Damage_Delay.Get();
	int damageMultiplier = 1;
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
		damage = damageMultiplier * pThis->Type->Damage + pThis->Accum;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (damage >= 1.0)
		{
			appliedDamage = static_cast<int>(std::round(damage));
			pThis->Accum = damage - appliedDamage;
		}
		else
		{
			pThis->Accum = damage;
			return SkipDamage;
		}
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
		pThis->Accum = 0.0;
	}

	if (appliedDamage <= 0 || pThis->IsInert)
		return SkipDamage;

	TechnoClass* pInvoker = nullptr;
	HouseClass* pOwner = nullptr;

	if (pTypeExt->Damage_DealtByInvoker)
	{
		auto const pExt = AnimExt::ExtMap.Find(pThis);
		pInvoker = pExt->Invoker;
		pOwner = pExt->InvokerHouse;

		if (!pInvoker)
		{
			pInvoker = pThis->OwnerObject ? abstract_cast<TechnoClass*>(pThis->OwnerObject) : nullptr;

			if (pInvoker)
				pOwner = pInvoker->Owner;
		}
	}

	if (pTypeExt->Weapon)
	{
		WeaponTypeExt::DetonateAt(pTypeExt->Weapon, pThis->GetCoords(), pInvoker, appliedDamage, pOwner);
	}
	else
	{
		if (!pOwner)
		{
			if (pThis->Owner)
			{
				pOwner = pThis->Owner;
			}
			else if (pInvoker)
			{
				pOwner = pInvoker->Owner;
			}
			else if (pThis->OwnerObject)
			{
				pOwner = pThis->OwnerObject->GetOwningHouse();
			}
			else if (pThis->IsBuildingAnim)
			{
				auto const pBuilding = AnimExt::ExtMap.Find(pThis)->ParentBuilding;
				pOwner = pBuilding ? pBuilding->Owner : nullptr;
			}
		}

		auto pWarhead = pThis->Type->Warhead;

		if (!pWarhead)
			pWarhead = strcmp(pThis->Type->get_ID(), "INVISO") ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		MapClass::DamageArea(pThis->GetCoords(), appliedDamage, pInvoker, pWarhead, true, pOwner);
	}

	return Continue;
}

DEFINE_HOOK(0x4242E1, AnimClass_AI_TrailerAnim, 0x5)
{
	enum { SkipGameCode = 0x424322 };

	GET(AnimClass*, pThis, ESI);

	if (auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->TrailerAnim, pThis->GetCoords(), 1, 1))
	{
		auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
		auto const pExt = AnimExt::ExtMap.Find(pThis);
		AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->Owner, nullptr, false, true);
		pTrailerAnimExt->SetInvoker(pExt->Invoker, pExt->InvokerHouse);
	}

	return SkipGameCode;
}

// Deferred creation of attached particle systems for debris anims.
DEFINE_HOOK(0x423939, AnimClass_BounceAI_AttachedSystem, 0x6)
{
	GET(AnimClass*, pThis, EBP);

	AnimExt::ExtMap.Find(pThis)->CreateAttachedSystem();

	return 0;
}

DEFINE_HOOK(0x62E08B, ParticleSystemClass_DTOR_DetachAttachedSystem, 0x7)
{
	GET(ParticleSystemClass*, pParticleSystem, EDI);

	AnimExt::InvalidateParticleSystemPointers(pParticleSystem);

	return 0;
}

DEFINE_HOOK(0x423CC7, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);
	GET(bool const, heightFlag, EAX);

	if (!pThis || !pThis->Type)
		return SkipGameCode;

	auto const pType = pThis->Type;
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pType);
	auto const splashAnims = pTypeExt->SplashAnims.GetElements(RulesClass::Instance->SplashList);
	auto const nDamage = Game::F2I(pType->Damage);
	auto const pOwner = AnimExt::GetOwnerHouse(pThis);

	AnimExt::HandleDebrisImpact(pType->ExpireAnim, pTypeExt->WakeAnim, splashAnims, pOwner, pType->Warhead, nDamage,
		pThis->GetCell(), pThis->Location, heightFlag, pType->IsMeteor, pTypeExt->Warhead_Detonate, pTypeExt->ExplodeOnWater, pTypeExt->SplashAnims_PickRandom);

	return SkipGameCode;
}

DEFINE_HOOK(0x424807, AnimClass_AI_Next, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	const auto pExt = AnimExt::ExtMap.Find(pThis);
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
		pExt->DeleteAttachedSystem();

	if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
		pExt->CreateAttachedSystem();

	return 0;
}

DEFINE_HOOK(0x424CF1, AnimClass_Start_DetachedReport, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->DetachedReport >= 0)
		VocClass::PlayAt(pTypeExt->DetachedReport.Get(), pThis->GetCoords());

	return 0;
}

// 0x422CD8 is in an alternate code path only used by anims with ID RING1, unused normally but covering it just because
DEFINE_HOOK_AGAIN(0x422CD8, AnimClass_DrawIt_XDrawOffset, 0x6)
DEFINE_HOOK(0x423122, AnimClass_DrawIt_XDrawOffset, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x110, 0x4));

	if (auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		pLocation->X += pTypeExt->XDrawOffset;

	return 0;
}

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(AnimClass*, pThis, ECX);

	if (pThis->OwnerObject)
	{
		auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

		if (pTypeExt->Layer_UseObjectLayer.isset())
		{
			Layer layer = pThis->Type->Layer;

			if (pTypeExt->Layer_UseObjectLayer.Get())
				layer = pThis->OwnerObject->InWhichLayer();

			R->EAX(layer);
			return ReturnValue;
		}
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

DEFINE_HOOK(0x4236F0, AnimClass_DrawIt_Tiled_Palette, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	R->EDX(pTypeExt->Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL));

	return 0x4236F6;
}

DEFINE_HOOK(0x423365, AnimClass_DrawIt_ExtraShadow, 0x8)
{
	enum { DrawExtraShadow = 0x42336D, SkipExtraShadow = 0x4233EE };

	GET(AnimClass*, pThis, ESI);

	if (pThis->HasExtras)
	{
		const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

		if (!pTypeExt->ExtraShadow)
			return SkipExtraShadow;

		return DrawExtraShadow;
	}

	return SkipExtraShadow;
}

#pragma region AltPalette

// Fix AltPalette anims not using owner color scheme.
DEFINE_HOOK(0x4232E2, AnimClass_DrawIt_AltPalette, 0x6)
{
	enum { SkipGameCode = 0x4232EA };

	GET(AnimClass*, pThis, ESI);

	int schemeIndex = pThis->Owner ? pThis->Owner->ColorSchemeIndex - 1 : RulesExt::Global()->AnimRemapDefaultColorScheme;
	schemeIndex += AnimTypeExt::ExtMap.Find(pThis->Type)->AltPalette_ApplyLighting ? 1 : 0;
	auto const scheme = ColorScheme::Array->Items[schemeIndex];

	R->ECX(scheme);
	return SkipGameCode;
}

// Set ShadeCount to 53 to initialize the palette fully shaded - this is required to make it not draw over shroud for some reason.
DEFINE_HOOK(0x68C4C4, GenerateColorSpread_ShadeCountSet, 0x5)
{
	GET(int, shadeCount, EDX);

	if (shadeCount == 1)
		R->EDX(53);

	return 0;
}

#pragma endregion

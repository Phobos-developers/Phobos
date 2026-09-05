#include "Body.h"

#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/AresFunctions.h>

namespace AnimLoggingTemp
{
	DWORD UniqueID = 0;
	AnimTypeClass* pType = nullptr;
}

DEFINE_HOOK(0x423B95, AnimClass_AI_Early, 0x8)
{
	GET(AnimClass* const, pThis, ESI);

	if (ScenarioExt::Global()->FiringAnimUpdateCount > 0)
		AnimExt::Fetch(pThis)->UpdateAsFiringAnim();

	auto const pType = pThis->Type;

	AnimLoggingTemp::UniqueID = pThis->UniqueID;
	AnimLoggingTemp::pType = pType;

	// Replace vanilla HideIfNoOre check.
	if (pType->HideIfNoOre)
	{
		const int nThreshold = abs(AnimTypeExt::Fetch(pType)->HideIfNoOre_Threshold.Get());
		pThis->Invisible = pThis->GetCell()->GetContainedTiberiumValue() <= nThreshold;
	}

	return 0x423BC8;
}

// Nuke Ares' animation damage hook at 0x424538.
DEFINE_PATCH(0x424538, 0x8B, 0x8E, 0xCC, 0x00, 0x00, 0x00);

// And add the new one after that.
DEFINE_HOOK(0x42453E, AnimClass_AI_Damage, 0x6)
{
	enum { SkipDamage = 0x42465D, Continue = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	if (pThis->IsInert)
		return SkipDamage;

	const auto pType = pThis->Type;
	const auto pTypeExt = AnimTypeExt::Fetch(pType);
	const auto pOwnerObject = pThis->OwnerObject;
	const int delay = pTypeExt->Damage_Delay.Get();
	const bool isTerrain = pOwnerObject && pOwnerObject->WhatAmI() == AbstractType::Terrain;
	const int damageMultiplier = isTerrain ? 5 : 1;
	const double baseDamage = pType->Damage;
	const bool firstDamage = pThis->Animation.Value == std::max(delay - 1, 1);

	int appliedDamage = 0;

	if (pTypeExt->Damage_ApplyOncePerLoop) // If damage is to be applied only once per animation loop
	{
		if (firstDamage)
			appliedDamage = static_cast<int>(std::round(baseDamage)) * damageMultiplier;
		else
			return SkipDamage;
	}
	else if (delay <= 0 || baseDamage < 1.0) // If Damage.Delay is less than 1 or Damage is a fraction.
	{
		const double totalDamage = damageMultiplier * baseDamage + pThis->Accum;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (totalDamage >= 1.0)
		{
			appliedDamage = static_cast<int>(std::round(totalDamage));
			pThis->Accum = totalDamage - appliedDamage;
		}
		else
		{
			pThis->Accum = totalDamage;
			return SkipDamage;
		}
	}
	else
	{
		// Accum here is used as a counter for Damage.Delay, which cannot deal fractional damage.
		pThis->Accum += 1.0;

		if (pThis->Accum < delay)
			return SkipDamage;

		// Use Type->Damage as the actually dealt damage.
		appliedDamage = static_cast<int>(std::round(baseDamage)) * damageMultiplier;
		pThis->Accum = 0.0;
	}

	if (appliedDamage <= 0)
		return SkipDamage;

	TechnoClass* pInvoker = nullptr;
	HouseClass* pOwner = pThis->Owner;

	if (pTypeExt->Damage_DealtByInvoker.Get(RulesExt::Global()->AnimDamage_DealtByInvoker))
	{
		const auto pExt = AnimExt::Fetch(pThis);
		pInvoker = pExt->Invoker;

		if (!pInvoker)
		{
			if (pOwnerObject)
				pInvoker = abstract_cast<TechnoClass*, true>(pOwnerObject);
			else if (pThis->IsBuildingAnim)
				pInvoker = pExt->ParentBuilding;
		}

		if (pExt->InvokerHouse)
			pOwner = pExt->InvokerHouse;

		if (pInvoker)
		{
			if (!pExt->InvokerHouse)
				pOwner = pInvoker->Owner;

			// only calculate firepower multiplier in the first round
			if (firstDamage && pTypeExt->Damage_ApplyFirepowerMult.Get(RulesExt::Global()->AnimDamage_ApplyFirepowerMult))
				pExt->FirepowerMult = TechnoExt::GetCurrentFirepowerMultiplier(pInvoker);
		}

		if (pTypeExt->Damage_ApplyFirepowerMult.Get(RulesExt::Global()->AnimDamage_ApplyFirepowerMult))
			appliedDamage = static_cast<int>(appliedDamage * pExt->FirepowerMult);
	}

	// Jun 29, 2025 - Starkku: Owner != Invoker. Previously OwnerObject / ParentBuilding fallback only existed for Warheads
	// but if we are unifying the approaches it needs to be available even without and separately from invoker.
	if (!pOwner)
	{
		if (pOwnerObject)
			pOwner = pOwnerObject->GetOwningHouse();
		else if (pThis->IsBuildingAnim)
			pOwner = AnimExt::Fetch(pThis)->ParentBuilding->Owner;
	}

	if (pTypeExt->Weapon)
	{
		WeaponTypeExt::DetonateAt(pTypeExt->Weapon, pThis->GetCoords(), pInvoker, appliedDamage, pOwner, pOwnerObject);
	}
	else
	{
		auto pWarhead = pType->Warhead;

		if (!pWarhead)
			pWarhead = strcmp(pType->get_ID(), "INVISO") ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		MapClass::DamageArea(pThis->GetCoords(), appliedDamage, pInvoker, pWarhead, true, pOwner);
	}

	return Continue;
}

DEFINE_HOOK(0x42465D, AnimClass_AI_NullTypeCheck, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (!pThis->Type)
	{
		char buffer[28];

		if (AnimLoggingTemp::UniqueID == pThis->UniqueID && AnimLoggingTemp::pType)
			sprintf_s(buffer, sizeof(buffer), " [%s]", AnimLoggingTemp::pType->get_ID());
		else
			sprintf_s(buffer, sizeof(buffer), "");

		auto coords = pThis->Location;
		auto mapCoords = pThis->GetMapCoords();
		Debug::FatalErrorAndExit("AnimClass_AI_NullTypeCheck: Animation%s has null type. Active: %d | Inert: %d | Coords: %d,%d,%d | Cell: %d,%d\n",
			buffer, pThis->IsAlive, pThis->IsInert, coords.X, coords.Y, coords.Z, mapCoords.X, mapCoords.Y);
	}

	AnimLoggingTemp::UniqueID = 0;
	AnimLoggingTemp::pType = nullptr;

	return 0;
}

DEFINE_HOOK(0x4242E1, AnimClass_AI_TrailerAnim, 0x5)
{
	enum { SkipGameCode = 0x424322 };

	GET(AnimClass*, pThis, ESI);

	auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->TrailerAnim, pThis->GetCoords(), 1, 1);

	auto const pTrailerAnimExt = AnimExt::Fetch(pTrailerAnim);
	auto const pExt = AnimExt::Fetch(pThis);
	AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->Owner, nullptr, false, true);
	pTrailerAnimExt->SetInvoker(pExt->Invoker, pExt->InvokerHouse);

	return SkipGameCode;
}

// Deferred creation of attached particle systems for debris anims.
DEFINE_HOOK(0x423939, AnimClass_BounceAI_AttachedSystem, 0x6)
{
	GET(AnimClass*, pThis, EBP);

	AnimExt::Fetch(pThis)->CreateAttachedSystem();

	return 0;
}

DEFINE_HOOK(0x62E08B, ParticleSystemClass_DTOR_DetachAttachedSystem, 0x7)
{
	GET(ParticleSystemClass*, pParticleSystem, EDI);

	if (pParticleSystem->Owner && pParticleSystem->Owner->WhatAmI() == AbstractType::Anim)
		AnimExt::InvalidateParticleSystemPointers(pParticleSystem);

	return 0;
}

DEFINE_HOOK(0x423CC7, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);
	GET(bool const, heightFlag, EAX);

	if (!pThis)
		return SkipGameCode;

	auto const pType = pThis->Type;

	if (!pType)
		return SkipGameCode;

	auto const pTypeExt = AnimTypeExt::Fetch(pType);
	auto const splashAnims = pTypeExt->SplashAnims.GetElements(RulesClass::Instance->SplashList);
	auto const nDamage = static_cast<int>(pType->Damage);
	auto const pOwner = AnimExt::GetOwnerHouse(pThis);

	AnimExt::HandleDebrisImpact(pType->ExpireAnim, pTypeExt->WakeAnim, splashAnims, pOwner, pType->Warhead, nDamage,
		pThis->GetCell(), pThis->Location, heightFlag, pType->IsMeteor, pTypeExt->Warhead_Detonate, pTypeExt->ExplodeOnWater, pTypeExt->SplashAnims_PickRandom);

	return SkipGameCode;
}

DEFINE_HOOK(0x424807, AnimClass_AI_Next, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	const auto pExt = AnimExt::Fetch(pThis);
	const auto pTypeExt = AnimTypeExt::Fetch(pThis->Type);
	pThis->UseCellLightConvert = pTypeExt->TheaterPalette.Get(pThis->UseCellLightConvert);

	if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
		pExt->DeleteAttachedSystem();

	if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
		pExt->CreateAttachedSystem();

	if (const auto pAlphaMap = AresFunctions::AlphaExtMap)
	{
		if (const auto pAlpha = pAlphaMap->get_or_default(pThis))
			GameDelete(pAlpha);
	}

	return 0;
}

DEFINE_HOOK(0x424CF1, AnimClass_Start_DetachedReport, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);

	if (pTypeExt->DetachedReport >= 0)
		VocClass::PlayAt(pTypeExt->DetachedReport.Get(), pThis->GetCoords());

	return 0;
}

// 0x422CD8 is in an alternate code path only used by anims with ID RING1, unused normally but covering it just because
DEFINE_HOOK_AGAIN(0x422CD8, AnimClass_DrawIt_DrawOffset, 0x6)
DEFINE_HOOK(0x423122, AnimClass_DrawIt_DrawOffset, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x110, 0x4));

	auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);
	pLocation->X += pTypeExt->XDrawOffset;

	bool const applyX = pTypeExt->XDrawOffset_ApplyBracketWidth;
	bool const applyY = pTypeExt->YDrawOffset_ApplyBracketHeight;

	if ((applyX || applyY) && pThis->OwnerObject && pThis->OwnerObject->AbstractFlags & AbstractFlags::Techno)
	{
		// Hardcoded in shield healthbar code as well.
		constexpr int SHIELD_HEALTHBAR_OFFSET = -3;
		auto const pTechno = static_cast<TechnoClass*>(pThis->OwnerObject);
		bool const invertX = pTypeExt->XDrawOffset_InvertBracketShift;
		bool const invertY = pTypeExt->YDrawOffset_InvertBracketShift;

		if (auto const pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			auto const pType = pBuilding->Type;
			auto const pos = TechnoExt::GetBuildingSelectBracketPosition(pBuilding, pBuilding->Type, BuildingSelectBracketPosition::Top);

			if (applyY && ((pType->Height >= 0 && !invertY) || (pType->Height < 0 && invertY)))
				pLocation->Y = pos.Y + pTypeExt->YDrawOffset_BracketAdjust_Buildings.Get(pTypeExt->YDrawOffset_BracketAdjust);

			if (applyX)
			{
				int const width = pBuilding->Type->GetFoundationWidth();
				int const shift = static_cast<int>(Unsorted::CellWidthInPixels * (width / 2.0) * (invertX ? -1 : 1));
				pLocation->X = pos.X + shift + pTypeExt->XDrawOffset_BracketAdjust_Buildings.Get(pTypeExt->XDrawOffset_BracketAdjust);
			}
		}
		else
		{
			auto const pType = pTechno->GetTechnoType();
			auto const horizontalPos = invertX ? HorizontalPosition::Left : HorizontalPosition::Right;
			auto const pos = TechnoExt::GetFootSelectBracketPosition(pTechno, Anchor(horizontalPos, VerticalPosition::Top), pTechno->WhatAmI() == AbstractType::Infantry);

			if (applyY && ((pType->PixelSelectionBracketDelta <= 0 && !invertY) || (pType->PixelSelectionBracketDelta > 0 && invertY)))
				pLocation->Y = pos.Y + pType->PixelSelectionBracketDelta + pTypeExt->YDrawOffset_BracketAdjust;

			if (applyX)
				pLocation->X = pos.X + pTypeExt->XDrawOffset_BracketAdjust;
		}

		if (applyY)
		{
			if (auto const pShield = TechnoExt::Fetch(pTechno)->Shield.get())
			{
				auto const pShieldType = pShield->GetType();

				if (pShield->IsAvailable() && !pShield->IsBrokenAndNonRespawning() && (pShield->GetHealthRatio() > 0.0 || !pShieldType->Pips_HideIfNoStrength))
				{
					if ((pShieldType->BracketDelta <= 0 && !invertY) || (pShieldType->BracketDelta > 0 && invertY))
						pLocation->Y += pShieldType->BracketDelta + SHIELD_HEALTHBAR_OFFSET;
				}
			}
		}
	}

	*pLocation += AnimExt::Fetch(pThis)->AEDrawOffset;

	return 0;
}

#pragma region AttachedAnims

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(AnimClass*, pThis, ECX);

	if (pThis->OwnerObject)
	{
		auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);

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

DEFINE_HOOK(0x424C3D, AnimClass_AttachTo_AttachedAnimPosition, 0x6)
{
	enum { SkipGameCode = 0x424C76 };

	GET(AnimClass*, pThis, ESI);

	auto const pExt = AnimTypeExt::Fetch(pThis->Type);

	if (pExt->AttachedAnimPosition != AttachedAnimPosition::Default)
	{
		pThis->SetLocation(CoordStruct::Empty);
		return SkipGameCode;
	}

	return 0;
}


class AnimClassFake final : public AnimClass
{
	CoordStruct* _GetCenterCoords(CoordStruct* pCrd) const;
};

CoordStruct* AnimClassFake::_GetCenterCoords(CoordStruct* pCrd) const
{
	CoordStruct* coords = pCrd;
	*coords = this->Location;

	if (auto const pObject = this->OwnerObject)
	{
		*coords += pObject->GetCoords();

		if (AnimTypeExt::Fetch(this->Type)->AttachedAnimPosition == AttachedAnimPosition::Ground)
			coords->Z = MapClass::Instance.GetCellFloorHeight(*coords);
	}

	return coords;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E339C, AnimClassFake::_GetCenterCoords);

#pragma endregion

DEFINE_HOOK(0x4236F0, AnimClass_DrawIt_Tiled_Palette, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);

	R->EDX(pTypeExt->Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL));

	return 0x4236F6;
}

DEFINE_HOOK(0x423654, AnimClass_DrawIt_Tiled_Interval, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET(RectangleStruct*, pBounds, EAX);
	GET(const int*, pValue, EDI);

	int height = pBounds->Height;

	auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);
	if (pTypeExt->Tiled_Interval > 0)
		height = pTypeExt->Tiled_Interval;

	R->EAX(height);
	R->ECX(*pValue);
	return 0x423659;
}

DEFINE_HOOK(0x423660, AnimClass_DrawIt_Tiled_Center, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET(const int, height, EAX);
	R->EDX(VTable::Get(pThis)); // Restore overriden instruction

	const auto pTypeExt = AnimTypeExt::Fetch(pThis->Type);
	if (pTypeExt->Tiled_AlignToCenter)
		R->EAX(0);
	else
		R->EAX(height / 2);

	return 0x423667;
}

DEFINE_HOOK(0x423365, AnimClass_DrawIt_ExtraShadow, 0x8)
{
	enum { DrawExtraShadow = 0x42336D, SkipExtraShadow = 0x4233EE };

	GET(AnimClass*, pThis, ESI);

	if (pThis->HasExtras)
	{
		auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);

		if (!pTypeExt->ExtraShadow)
			return SkipExtraShadow;

		return DrawExtraShadow;
	}

	return SkipExtraShadow;
}

DEFINE_HOOK(0x423855, AnimClass_DrawIt_ShadowLocation, 0x7)
{
	enum { SkipGameCode = 0x42385D };

	GET(AnimClass*, pThis, ESI);
	GET(Point2D*, pLocation, EDI);

	int zCoord = pThis->GetZ();

	if (auto const pUnit = abstract_cast<UnitClass*>(pThis->OwnerObject))
	{
		// If deploying anim is played in air, cast shadow on ground.
		if (pUnit->DeployAnim == pThis && pUnit->GetHeight() > 0)
		{
			auto const pCell = pUnit->GetCell();
			auto const coords = pCell->GetCenterCoords();
			*pLocation = TacticalClass::Instance->CoordsToClient(coords).first;
			zCoord = coords.Z;
		}
	}

	R->EAX(zCoord);
	return SkipGameCode;
}

// Apply cell lighting on UseNormalLight=no MakeInfantry anims.
DEFINE_HOOK(0x4232BF, AnimClass_DrawIt_MakeInfantry, 0x6)
{
	enum { SkipGameCode = 0x4232C5 };

	GET(AnimClass*, pThis, ESI);

	if (pThis->Type->MakeInfantry != -1)
	{
		auto const pCell = pThis->GetCell();
		R->EAX(pCell->Intensity_Normal);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x423061, AnimClass_DrawIt_Visibility, 0x6)
{
	enum { SkipDrawing = 0x4238A3 };

	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::Fetch(pThis->Type);

	if (!pTypeExt->RestrictVisibilityIfCloaked && pTypeExt->VisibleTo == AffectedHouse::All)
		return 0;

	auto pTechno = abstract_cast<TechnoClass*>(pThis->OwnerObject);
	HouseClass* const pCurrentHouse = HouseClass::CurrentPlayer;

	if (!pTechno)
	{
		auto const pExt = AnimExt::Fetch(pThis);

		if (pExt->IsTechnoTrailerAnim)
			pTechno = pExt->Invoker;
	}

	if (pTypeExt->RestrictVisibilityIfCloaked && pTechno && !HouseClass::IsCurrentPlayerObserver()
		&& (pTechno->CloakState == CloakState::Cloaked || pTechno->CloakState == CloakState::Cloaking)
		&& !pTechno->Owner->IsAlliedWith(pCurrentHouse)
		&& !pTechno->GetCell()->Sensors_InclHouse(pCurrentHouse->ArrayIndex))
	{
		return SkipDrawing;
	}

	auto pOwner = pThis->OwnerObject ? pThis->OwnerObject->GetOwningHouse() : pThis->Owner;

	if (pTypeExt->VisibleTo_ConsiderInvokerAsOwner)
	{
		auto const pExt = AnimExt::Fetch(pThis);

		if (pExt->Invoker)
			pOwner = pExt->Invoker->Owner;
		else if (pExt->InvokerHouse)
			pOwner = pExt->InvokerHouse;
	}

	if (!HouseClass::IsCurrentPlayerObserver() && !EnumFunctions::CanTargetHouse(pTypeExt->VisibleTo, pCurrentHouse, pOwner))
		return SkipDrawing;

	return 0;
}

// Reverse-engineered from YR for Translucent=no code path only with exception of new additions.
DEFINE_HOOK(0x423183, AnimClass_DrawIt_Translucency, 0x6)
{
	enum { SkipGameCode = 0x4230FE, ReturnFromFunction = 0x4238A3 };

	GET(AnimClass*, pThis, ESI);
	GET(BlitterFlags, flags, EBX);

	const auto pType = pThis->Type;
	const auto pTypeExt = AnimTypeExt::Fetch(pType);
	const int translucencyLevel = pThis->TranslucencyLevel; // Used by building animations when building needs to be drawn partially transparent. >= 15 means animation skips drawing.
	const int currentFrame = pThis->Animation.Value;
	const int frames = pType->End;
	TranslucencyLevel level;
	bool hasValue = false;

	if (pTypeExt->Translucency_Cloaked.HasValues())
	{
		// New addition: Different Translucency animation for attached animations on cloaked objects. Also keyframeable.
		if (const auto pTechno = abstract_cast<TechnoClass*>(pThis->OwnerObject))
		{
			if (pTechno->CloakState == CloakState::Cloaked || pTechno->CloakState == CloakState::Cloaking)
			{
				level = pTypeExt->Translucency_Cloaked.Get(static_cast<double>(currentFrame) / frames);
				hasValue = true;
			}
		}
	}

	if (!hasValue && pTypeExt->Translucency.HasValues())
	{
		// New addition: Keyframeable Translucency, replaces game Translucency setting.
		level = pTypeExt->Translucency.Get(static_cast<double>(currentFrame) / frames);
	}

	if (level == BlitterFlags::None)
	{
		// Translucency <= 0, map translucencyLevel to transparency blitter flags
		if (translucencyLevel)
		{
			if (translucencyLevel > 15)
				return ReturnFromFunction;
			else if (translucencyLevel > 5)
				flags |= BlitterFlags::TransLucent50;
			else
				flags |= BlitterFlags::TransLucent25;
		}
	}
	else
	{
		// Translucency > 0, Translucency directly maps to blitter flags.
		if (translucencyLevel >= 15)
			return ReturnFromFunction;

		flags |= level;
	}

	R->EBX(flags);
	return SkipGameCode;
}

#pragma region AltPalette

// Fix AltPalette anims not using owner color scheme.
DEFINE_HOOK(0x4232E2, AnimClass_DrawIt_AltPalette, 0x6)
{
	enum { SkipGameCode = 0x4232EA };

	GET(AnimClass*, pThis, ESI);

	int schemeIndex = pThis->Owner ? pThis->Owner->ColorSchemeIndex - 1 : RulesExt::Global()->AnimRemapDefaultColorScheme;
	schemeIndex += AnimTypeExt::Fetch(pThis->Type)->AltPalette_ApplyLighting ? 1 : 0;
	auto const scheme = ColorScheme::Array[schemeIndex];

	R->ECX(scheme);
	return SkipGameCode;
}

// Set ShadeCount to 53 to initialize the palette fully shaded - this is required to make it not draw over shroud for some reason.
DEFINE_HOOK(0x68C4C4, GenerateColorSpread_ShadeCountSet, 0x5)
{
	GET(const int, shadeCount, EDX);

	if (shadeCount == 1)
		R->EDX(53);

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x425174, AnimClass_Detach_Cloak, 0x6)
{
	enum { SkipDetaching = 0x4251A3 };

	GET(AnimClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	auto const pTypeExt = AnimTypeExt::TryFetch(pThis->Type);

	if (pTypeExt && !pTypeExt->DetachOnCloak)
	{
		if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			auto const pTechnoExt = TechnoExt::Fetch(pTechno);

			if (pTechnoExt->IsDetachingForCloak)
				return SkipDetaching;
		}
	}

	return 0;
}

#pragma region ScorchFlamer

// Disable Ares' implementation.
DEFINE_PATCH(0x42511B, 0x5F, 0x5E, 0x5D, 0x5B, 0x83, 0xC4, 0x20);
DEFINE_PATCH(0x4250C9, 0x5F, 0x5E, 0x5D, 0x5B, 0x83, 0xC4, 0x20);
DEFINE_PATCH(0x42513F, 0x5F, 0x5E, 0x5D, 0x5B, 0x83, 0xC4, 0x20);

DEFINE_HOOK(0x425060, AnimClass_Expire_ScorchFlamer, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if (pType->Flamer || pType->Scorch)
		AnimExt::SpawnFireAnims(pThis);

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x4250E1, AnimClass_Middle_CraterDestroyTiberium, 0x6)
{
	enum { SkipDestroyTiberium = 0x4250EC };
	GET(AnimTypeClass*, pType, EDX);
	return AnimTypeExt::Fetch(pType)->Crater_DestroyTiberium.Get(RulesExt::Global()->AnimCraterDestroyTiberium) ? 0 : SkipDestroyTiberium;
}

#pragma region FiringAnimUpdate

DEFINE_HOOK(0x6FF42B, TechnoClass_Fire_Anim, 0x7)
{
	enum { SkipBuildingCheck = 0x6FF437 };

	GET(TechnoClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_BASE(const int, wpIdx, 0xC);

	if (pWeapon->Anim.Count > 0 && WeaponTypeExt::Fetch(pWeapon)->Anim_Update.Get(RulesExt::Global()->FiringAnim_Update))
	{
		const auto pAnimExt = AnimExt::Fetch(pAnim);
		pAnimExt->FiringAnim_Weapon = pWeapon;
		pAnimExt->FiringAnim_WeaponIndex = wpIdx;
		pAnimExt->FiringAnim_BurstIndex = pThis->CurrentBurstIndex;
		ScenarioExt::Global()->FiringAnimUpdateCount++;
		return SkipBuildingCheck;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x47DA74, CellClass_RecalcAttributes_TileAnimDrawer, 0x7)
{
	enum { SkipGameCode = 0x47DA7B };

	GET(AnimClass*, pAnim, EAX);

	pAnim->UseCellLightConvert = AnimTypeExt::Fetch(pAnim->Type)->TheaterPalette.Get(true);

	return SkipGameCode;
}

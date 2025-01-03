#include <AircraftClass.h>
#include "Body.h"

#include <ScenarioClass.h>
#include <TunnelLocomotionClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

#pragma region Update

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	TechnoExt::ExtMap.Find(pThis)->OnEarlyUpdate();
	TechnoExt::ApplyMindControlRangeLimit(pThis);

	return 0;
}

// Ares-hook jmp to this offset
DEFINE_HOOK(0x71A88D, TemporalClass_AI, 0x0)
{
	GET(TemporalClass*, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		pTarget->IsMouseHovering = false;

		const auto pExt = TechnoExt::ExtMap.Find(pTarget);
		pExt->UpdateTemporal();
	}

	// Recovering vanilla instructions that were broken by a hook call
	return R->EAX<int>() <= 0 ? 0x71A895 : 0x71AB08;
}

DEFINE_HOOK_AGAIN(0x51BAC7, FootClass_AI_Tunnel, 0x6)//InfantryClass_AI_Tunnel
DEFINE_HOOK(0x7363B5, FootClass_AI_Tunnel, 0x6)//UnitClass_AI_Tunnel
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipGameSelfHeal;
}

// Can't hook where unit promotion happens in vanilla because of Ares - Fryone, Kerbiter
DEFINE_HOOK(0x6F9FA9, TechnoClass_AI_PromoteAnim, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	auto aresProcess = [pThis]() { return (pThis->GetTechnoType()->Turret) ? 0x6F9FB7 : 0x6FA054; };

	if (!RulesExt::Global()->Promote_VeteranAnimation && !RulesExt::Global()->Promote_EliteAnimation)
		return aresProcess();

	if (pThis->CurrentRanking != pThis->Veterancy.GetRemainingLevel() && pThis->CurrentRanking != Rank::Invalid && (pThis->Veterancy.GetRemainingLevel() != Rank::Rookie))
	{
		AnimClass* promAnim = nullptr;
		if (pThis->Veterancy.GetRemainingLevel() == Rank::Veteran && RulesExt::Global()->Promote_VeteranAnimation)
			promAnim = GameCreate<AnimClass>(RulesExt::Global()->Promote_VeteranAnimation, pThis->GetCenterCoords());
		else if (RulesExt::Global()->Promote_EliteAnimation)
			promAnim = GameCreate<AnimClass>(RulesExt::Global()->Promote_EliteAnimation, pThis->GetCenterCoords());

		if(promAnim)
			promAnim->SetOwnerObject(pThis);
	}

	return aresProcess();
}

DEFINE_HOOK(0x6FA540, TechnoClass_AI_ChargeTurret, 0x6)
{
	enum { SkipGameCode = 0x6FA5BE };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->ChargeTurretDelay <= 0)
	{
		pThis->CurrentTurretNumber = 0;
		return SkipGameCode;
	}

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	int timeLeft = pThis->RearmTimer.GetTimeLeft();

	if (pExt->ChargeTurretTimer.HasStarted())
		timeLeft = pExt->ChargeTurretTimer.GetTimeLeft();
	else if (pExt->ChargeTurretTimer.Expired())
		pExt->ChargeTurretTimer.Stop();

	int turretCount = pType->TurretCount;
	int turretIndex = Math::max(0, timeLeft * turretCount / pThis->ChargeTurretDelay);

	if (turretIndex >= turretCount)
		turretIndex = turretCount - 1;

	pThis->CurrentTurretNumber = turretIndex;

	return SkipGameCode;
}

#pragma endregion

#pragma region Init

DEFINE_HOOK(0x6F42F7, TechnoClass_Init, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!pType)
		return 0;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pType);

	pExt->CurrentShieldType = pExt->TypeExtData->ShieldType;
	pExt->InitializeLaserTrails();
	pExt->InitializeAttachEffects();

	return 0;
}

DEFINE_HOOK(0x6F421C, TechnoClass_Init_DefaultDisguise, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	// mirage is not here yet
	if (pThis->WhatAmI() == AbstractType::Infantry && pExt->DefaultDisguise)
	{
		pThis->Disguise = pExt->DefaultDisguise;
		pThis->Disguised = true;
		return 0x6F4277;
	}

	pThis->Disguised = false;

	return 0;
}

DEFINE_HOOK_AGAIN(0x7355C0, TechnoClass_Init_InitialStrength, 0x6) // UnitClass_Init
DEFINE_HOOK_AGAIN(0x517D69, TechnoClass_Init_InitialStrength, 0x6) // InfantryClass_Init
DEFINE_HOOK_AGAIN(0x442C7B, TechnoClass_Init_InitialStrength, 0x6) // BuildingClass_Init
DEFINE_HOOK(0x414057, TechnoClass_Init_InitialStrength, 0x6)       // AircraftClass_Init
{
	GET(TechnoClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (R->Origin() != 0x517D69)
	{
		if (R->Origin() != 0x442C7B)
			R->EAX(pTypeExt->InitialStrength.Get(R->EAX<int>()));
		else
			R->ECX(pTypeExt->InitialStrength.Get(R->ECX<int>()));
	}
	else
	{
		auto strength = pTypeExt->InitialStrength.Get(R->EDX<int>());
		pThis->Health = strength;
		pThis->EstimatedHealth = strength;
	}

	return 0;
}

#pragma endregion

#pragma region Limbo

DEFINE_HOOK(0x6F6AC4, TechnoClass_Limbo, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->Shield)
		pExt->Shield->KillAnim();

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

#pragma endregion

DEFINE_HOOK_AGAIN(0x70CF39, TechnoClass_ReplaceArmorWithShields, 0x6) //TechnoClass_EvalThreatRating_Shield
DEFINE_HOOK_AGAIN(0x6F7D31, TechnoClass_ReplaceArmorWithShields, 0x6) //TechnoClass_CanAutoTargetObject_Shield
DEFINE_HOOK_AGAIN(0x6FCB64, TechnoClass_ReplaceArmorWithShields, 0x6) //TechnoClass_CanFire_Shield
DEFINE_HOOK(0x708AEB, TechnoClass_ReplaceArmorWithShields, 0x6) //TechnoClass_ShouldRetaliate_Shield
{
	WeaponTypeClass* pWeapon = nullptr;
	if (R->Origin() == 0x708AEB)
		pWeapon = R->ESI<WeaponTypeClass*>();
	else if (R->Origin() == 0x6F7D31)
		pWeapon = R->EBP<WeaponTypeClass*>();
	else
		pWeapon = R->EBX<WeaponTypeClass*>();

	ObjectClass* pTarget = nullptr;
	if (R->Origin() == 0x6F7D31 || R->Origin() == 0x70CF39)
		pTarget = R->ESI<ObjectClass*>();
	else
		pTarget = R->EBP<ObjectClass*>();

	if (const auto pExt = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget)))
	{
		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->CanBePenetrated(pWeapon->Warhead))
				return 0;

			if (pShieldData->IsActive())
			{
				R->EAX(pShieldData->GetArmorType());
				return R->Origin() + 6;
			}
		}
	}

	return 0;
}

#pragma region StatMultipliers

DEFINE_HOOK(0x4DB218, FootClass_GetMovementSpeed_SpeedMultiplier, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(int, speed, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	speed = static_cast<int>(speed * pExt->AE.SpeedMultiplier);
	R->EAX(speed);

	return 0;
}

static int CalculateArmorMultipliers(TechnoClass* pThis, int damage, WarheadTypeClass* pWarhead)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	double mult = pExt->AE.ArmorMultiplier;

	if (pExt->AE.HasRestrictedArmorMultipliers)
	{
		for (auto const& attachEffect : pExt->AttachedEffects)
		{
			if (!attachEffect->IsActive())
				continue;

			auto const type = attachEffect->GetType();

			if (type->ArmorMultiplier_DisallowWarheads.Contains(pWarhead))
				continue;

			if (type->ArmorMultiplier_AllowWarheads.size() > 0 && !type->ArmorMultiplier_AllowWarheads.Contains(pWarhead))
				continue;

			mult *= type->ArmorMultiplier;
		}
	}

	return static_cast<int>(damage / mult);
}

DEFINE_HOOK(0x6FDC87, TechnoClass_AdjustDamage_ArmorMultiplier, 0x6)
{
	GET(TechnoClass*, pTarget, EDI);
	GET(int, damage, EAX);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x18, 0x8));

	R->EAX(CalculateArmorMultipliers(pTarget, damage, pWeapon->Warhead));

	return 0;
}

DEFINE_HOOK(0x701966, TechnoClass_ReceiveDamage_ArmorMultiplier, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, damage, EAX);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	R->EAX(CalculateArmorMultipliers(pThis, damage, pWarhead));

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

#pragma endregion

#pragma region Disguise

bool __fastcall IsAlly_Wrapper(HouseClass* pThis, void* _, HouseClass* pOther)
{
	return pThis->IsObserver() || pOther->IsAlliedWith(pOther) || (RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool __fastcall IsControlledByCurrentPlayer_Wrapper(HouseClass* pThis)
{
	HouseClass* pCurrent = HouseClass::CurrentPlayer;
	AffectedHouse visibilityFlags = RulesExt::Global()->DisguiseBlinkingVisibility;

	if (SessionClass::IsCampaign() && (pThis->IsHumanPlayer || pThis->IsInPlayerControl))
	{
		if ((visibilityFlags & AffectedHouse::Allies) != AffectedHouse::None && pCurrent->IsAlliedWith(pThis))
			return true;

		return (visibilityFlags & AffectedHouse::Owner) != AffectedHouse::None;
	}

	return pCurrent->IsObserver() || EnumFunctions::CanTargetHouse(visibilityFlags, pCurrent, pThis);
}

DEFINE_JUMP(CALL, 0x4DEDD2, GET_OFFSET(IsAlly_Wrapper));                      // FootClass_GetImage
DEFINE_JUMP(CALL, 0x70EE5D, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x70EE70, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x7062FB, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_DrawObject

DEFINE_HOOK(0x7060A9, TechnoClas_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	LightConvertClass* convert = nullptr;

	auto const pType = pThis->IsDisguised() ? TechnoTypeExt::GetTechnoType(pThis->Disguise) : nullptr;
	int colorIndex = pThis->GetDisguiseHouse(true)->ColorSchemeIndex;

	if (pType && pType->Palette && pType->Palette->Count > 0)
		convert = pType->Palette->GetItem(colorIndex)->LightConvert;
	else
		convert = ColorScheme::Array->GetItem(colorIndex)->LightConvert;

	R->EBX(convert);

	return SkipGameCode;
}

#pragma endregion

DEFINE_HOOK(0x702E4E, TechnoClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (pTechnoExt)
	{
		for (auto& trail : pTechnoExt->LaserTrails)
		{
			trail.Visible = false;
			trail.LastLocation = { };
		}
	}

	return 0;
}

// I don't think buildings should have laser-trails
DEFINE_HOOK(0x4D7221, FootClass_Unlimbo_LaserTrails, 0x6)
{
	GET(FootClass*, pTechno, ESI);

	if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
	{
		for (auto& trail : pTechnoExt->LaserTrails)
		{
			trail.LastLocation = { };
			trail.Visible = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain_Organics, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));
	GET_STACK(bool, isForceShield, STACK_OFFSET(0x10, 0xC));

	enum { MakeInvulnerable = 0x4DEB38, SkipGameCode = 0x4DEBA2 };

	if (!pType->Organic && pThis->WhatAmI() != AbstractType::Infantry)
		return MakeInvulnerable;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainEffect icEffect = !isForceShield ? pTypeExt->IronCurtain_Effect.Get(RulesExt::Global()->IronCurtain_EffectOnOrganics) :
		pTypeExt->ForceShield_Effect.Get(RulesExt::Global()->ForceShield_EffectOnOrganics);

	switch (icEffect)
	{
	case IronCurtainEffect::Ignore:
		R->EAX(DamageState::Unaffected);
		break;
	case IronCurtainEffect::Invulnerable:
		return MakeInvulnerable;
		break;
	default:
		auto pWH = RulesClass::Instance->C4Warhead;

		if (!isForceShield)
			pWH = pTypeExt->IronCurtain_KillWarhead.Get(RulesExt::Global()->IronCurtain_KillOrganicsWarhead.Get(pWH));
		else
			pWH = pTypeExt->ForceShield_KillWarhead.Get(RulesExt::Global()->ForceShield_KillOrganicsWarhead.Get(pWH));

		R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, pWH, nullptr, true, false, pSource));
		break;
	}

	return SkipGameCode;
}

DEFINE_JUMP(VTABLE, 0x7EB1AC, 0x4DEAE0); // Redirect InfantryClass::IronCurtain to FootClass::IronCurtain

DEFINE_HOOK(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		return pExt->NoManualMove ? 0x700C62 : 0;

	return 0;
}

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x6)
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (pThis)
	{
		maxSpeed = pThis->GetTechnoType()->Speed;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->UseDisguiseMovementSpeed && pThis->IsDisguised())
		{
			if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
				maxSpeed = pType->Speed;
		}
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}


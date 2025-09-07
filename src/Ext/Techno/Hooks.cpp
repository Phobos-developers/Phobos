#include "Body.h"

#include <AircraftClass.h>
#include <EventClass.h>
#include <ScenarioClass.h>
#include <TunnelLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/AresHelper.h>
#include <Utilities/AresFunctions.h>

#pragma region GetTechnoType

// Avoid secondary jump
DEFINE_JUMP(VTABLE, 0x7E2328, 0x41C200) // AircraftClass_GetTechnoType -> AircraftClass_GetType
DEFINE_JUMP(VTABLE, 0x7E3F40, 0x459EE0) // BuildingClass_GetTechnoType -> BuildingClass_GetType
DEFINE_JUMP(VTABLE, 0x7EB0DC, 0x51FAF0) // InfantryClass_GetTechnoType -> InfantryClass_GetType
DEFINE_JUMP(VTABLE, 0x7F5CF4, 0x741490) // UnitClass_GetTechnoType -> UnitClass_GetType

#pragma endregion

#pragma region Update

// Early, before ObjectClass_AI
DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::ExtMap.Find(pThis)->OnEarlyUpdate();

	return 0;
}

// After TechnoClass_AI
DEFINE_HOOK(0x4DA54E, FootClass_AI, 0x6)
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->PreviousType)
		pExt->UpdateTypeData_Foot();

	pExt->UpdateWarpInDelay();
	pExt->UpdateTiberiumEater();

	return 0;
}

// After FootClass_AI
DEFINE_HOOK(0x736480, UnitClass_AI, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateKeepTargetOnMove();
	pExt->DepletedAmmoActions();

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

DEFINE_HOOK_AGAIN(0x51B389, FootClass_TunnelAI_Enter, 0x6) // InfantryClass_TunnelAI
DEFINE_HOOK(0x735A26, FootClass_TunnelAI_Enter, 0x6)       // UnitClass_TunnelAI
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	const auto pType = pThis->GetTechnoType();
	const auto pImage = pType->AlphaImage;

	if (pImage && AresHelper::CanUseAres)
	{
		auto& alphaExt = *AresFunctions::AlphaExtMap;

		if (const auto pAlpha = alphaExt.get_or_default(pThis))
		{
			GameDelete(pAlpha);

			const auto tacticalPos = TacticalClass::Instance->TacticalPos;
			const Point2D off = { tacticalPos.X - (pImage->Width / 2), tacticalPos.Y - (pImage->Height / 2) };
			const auto point = TacticalClass::Instance->CoordsToClient(pThis->GetCoords()).first + off;
			const RectangleStruct dirty = { point.X - tacticalPos.X, point.Y - tacticalPos.Y, pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(dirty, true);
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x51BA94, FootClass_TunnelAI_Exit, 0x7) // InfantryClass_TunnelAI
DEFINE_HOOK(0x736005, FootClass_TunnelAI_Exit, 0x6)       // UnitClass_TunnelAI
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelExit();

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

	auto const pType = pThis->GetTechnoType();

	auto aresProcess = [pType]() { return (pType->Turret) ? 0x6F9FB7 : 0x6FA054; };

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const pVeteranAnim = !pTypeExt->Promote_VeteranAnimation.empty() ? pTypeExt->Promote_VeteranAnimation : RulesExt::Global()->Promote_VeteranAnimation;
	auto const pEliteAnim = !pTypeExt->Promote_EliteAnimation.empty() ? pTypeExt->Promote_EliteAnimation : RulesExt::Global()->Promote_EliteAnimation;

	if (pVeteranAnim.empty() && pEliteAnim.empty())
		return aresProcess();

	if (pThis->CurrentRanking != pThis->Veterancy.GetRemainingLevel() && pThis->CurrentRanking != Rank::Invalid && (pThis->Veterancy.GetRemainingLevel() != Rank::Rookie))
	{
		if (pThis->Veterancy.GetRemainingLevel() == Rank::Veteran && !pVeteranAnim.empty())
			AnimExt::CreateRandomAnim(pVeteranAnim, pThis->GetCenterCoords(), pThis, pThis->Owner, true, true);
		else if (!pEliteAnim.empty())
			AnimExt::CreateRandomAnim(pEliteAnim, pThis->GetCenterCoords(), pThis, pThis->Owner, true, true);
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

	const int turretCount = pType->TurretCount;
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

	if (!pType) // Critical sanity check in s/l
		return 0;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	pExt->TypeExtData = pTypeExt;

	pExt->CurrentShieldType = pTypeExt->ShieldType;
	pExt->InitializeAttachEffects();
	pExt->InitializeDisplayInfo();
	pExt->InitializeLaserTrails();

	if (!pExt->AE.HasTint && !pExt->CurrentShieldType)
		pExt->UpdateTintValues();

	if (pTypeExt->Harvester_Counted)
		HouseExt::ExtMap.Find(pThis->Owner)->OwnedCountedHarvesters.push_back(pThis);

	if ((pThis->Owner->IsControlledByHuman() || !RulesExt::Global()->DistributeTargetingFrame_AIOnly)
		&& pTypeExt->DistributeTargetingFrame.Get(RulesExt::Global()->DistributeTargetingFrame))
	{
		pThis->TargetingTimer.Start(ScenarioClass::Instance->Random.RandomRanged(0, 15));
	}

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
		pThis->DisguisedAsHouse = pThis->Owner;
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

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (R->Origin() != 0x517D69)
	{
		if (R->Origin() != 0x442C7B)
			R->EAX(pTypeExt->InitialStrength.Get(R->EAX<int>()));
		else
			R->ECX(pTypeExt->InitialStrength.Get(R->ECX<int>()));
	}
	else
	{
		auto const strength = pTypeExt->InitialStrength.Get(R->EDX<int>());
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

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->Shield)
		pExt->Shield->KillAnim();

	return 0;
}

bool __fastcall TechnoClass_Limbo_Wrapper(TechnoClass* pThis)
{
	// Do not remove attached effects from undeploying buildings.
	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		if (pBuilding->Type->UndeploysInto && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus == 2)
			return pThis->TechnoClass::Limbo();
	}

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	bool markForRedraw = false;
	bool altered = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;

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

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4A34, TechnoClass_Limbo_Wrapper); // TechnoClass
DEFINE_FUNCTION_JUMP(CALL, 0x4DB3B1, TechnoClass_Limbo_Wrapper);   // FootClass
DEFINE_FUNCTION_JUMP(CALL, 0x445DDA, TechnoClass_Limbo_Wrapper)    // BuildingClass

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

	if (const auto pExt = TechnoExt::ExtMap.TryFind(abstract_cast<TechnoClass*>(pTarget)))
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
	GET(const int, damage, EAX);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x18, 0x8));

	R->EAX(CalculateArmorMultipliers(pTarget, damage, pWeapon->Warhead));

	return 0;
}

DEFINE_HOOK(0x701966, TechnoClass_ReceiveDamage_ArmorMultiplier, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const int, damage, EAX);
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

bool __fastcall IsAlly_Wrapper(HouseClass* pTechnoOwner, void* _, HouseClass* pCurrentPlayer)
{
	return pCurrentPlayer->IsObserver() || pTechnoOwner->IsAlliedWith(pCurrentPlayer) || (RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool __fastcall IsControlledByCurrentPlayer_Wrapper(HouseClass* pThis)
{
	HouseClass* pCurrent = HouseClass::CurrentPlayer;
	const AffectedHouse visibilityFlags = RulesExt::Global()->DisguiseBlinkingVisibility;

	if (SessionClass::IsCampaign() && (pThis->IsHumanPlayer || pThis->IsInPlayerControl))
	{
		if ((visibilityFlags & AffectedHouse::Allies) != AffectedHouse::None && pCurrent->IsAlliedWith(pThis))
			return true;

		return (visibilityFlags & AffectedHouse::Owner) != AffectedHouse::None;
	}

	return pCurrent->IsObserver() || EnumFunctions::CanTargetHouse(visibilityFlags, pCurrent, pThis);
}

DEFINE_FUNCTION_JUMP(CALL, 0x4DEDD2, IsAlly_Wrapper);                      // FootClass_GetImage
DEFINE_FUNCTION_JUMP(CALL, 0x70EE5D, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_ClearlyVisibleTo
DEFINE_FUNCTION_JUMP(CALL, 0x70EE70, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_ClearlyVisibleTo
DEFINE_FUNCTION_JUMP(CALL, 0x7062FB, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_DrawObject

DEFINE_HOOK(0x7060A9, TechnoClas_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	LightConvertClass* convert = nullptr;

	auto const pType = pThis->IsDisguised() ? TechnoTypeExt::GetTechnoType(pThis->Disguise) : nullptr;
	const int colorIndex = pThis->GetDisguiseHouse(true)->ColorSchemeIndex;

	if (pType && pType->Palette && pType->Palette->Count > 0)
		convert = pType->Palette->GetItem(colorIndex)->LightConvert;
	else
		convert = ColorScheme::Array.GetItem(colorIndex)->LightConvert;

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

	auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	for (const auto& pTrail : pTechnoExt->LaserTrails)
	{
		pTrail->Visible = false;
		pTrail->LastLocation = { };
	}

	return 0;
}

// I don't think buildings should have laser-trails
DEFINE_HOOK(0x4D7221, FootClass_Unlimbo_LaserTrails, 0x6)
{
	GET(FootClass*, pTechno, ESI);

	auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	for (const auto& pTrail : pTechnoExt->LaserTrails)
	{
		pTrail->LastLocation = { };
		pTrail->Visible = true;
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain_Organics, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));
	GET_STACK(const bool, isForceShield, STACK_OFFSET(0x10, 0xC));

	enum { MakeInvulnerable = 0x4DEB38, SkipGameCode = 0x4DEBA2 };

	if (!pType->Organic && pThis->WhatAmI() != AbstractType::Infantry)
		return MakeInvulnerable;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const IronCurtainEffect icEffect = !isForceShield
		? pTypeExt->IronCurtain_Effect.Get(RulesExt::Global()->IronCurtain_EffectOnOrganics)
		: pTypeExt->ForceShield_Effect.Get(RulesExt::Global()->ForceShield_EffectOnOrganics);

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

#pragma region NoManualMove

DEFINE_HOOK(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	return TechnoExt::ExtMap.Find(pThis)->TypeExtData->NoManualMove ? 0x700C62 : 0;
}

DEFINE_HOOK(0x4437B3, BuildingClass_CellClickedAction_NoManualMove, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);

	return TechnoTypeExt::ExtMap.Find(pType)->NoManualMove ? 0x44384E : 0;
}

DEFINE_HOOK(0x44F62B, BuildingClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);

	R->ECX(TechnoTypeExt::ExtMap.Find(pType)->NoManualMove ? 0 : pType->UndeploysInto);

	return 0x44F631;
}

#pragma endregion

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x6)
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	auto const pThisType = pThis->GetTechnoType();
	int maxSpeed = pThisType->Speed;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);

	if (pTypeExt->UseDisguiseMovementSpeed && pThis->IsDisguised())
	{
		if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
			maxSpeed = pType->Speed;
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

DEFINE_HOOK(0x73B4DA, UnitClass_DrawVXL_WaterType_Extra, 0x6)
{
	enum { Continue = 0x73B4E0 };

	GET(UnitClass*, pThis, EBP);

	if (pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer) && !pThis->Deployed)
	{
		if (UnitTypeClass* pCustomType = TechnoExt::GetUnitTypeExtra(pThis))
			R->EBX<ObjectTypeClass*>(pCustomType);
	}

	return 0;
}

DEFINE_HOOK(0x73C602, UnitClass_DrawSHP_WaterType_Extra, 0x6)
{
	enum { Continue = 0x73C608 };

	GET(UnitClass*, pThis, EBP);

	if (pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer) && !pThis->Deployed)
	{
		if (const UnitTypeClass* pCustomType = TechnoExt::GetUnitTypeExtra(pThis))
		{
			if (SHPStruct* Image = pCustomType->GetImage())
				R->EAX<SHPStruct*>(Image);
		}
	}

	R->ECX(pThis->Type);
	return Continue;
}

DEFINE_HOOK(0x414987, AircraftClass_Draw_Extra, 0x6)
{
	enum { Continue = 0x41498D };

	GET(AircraftClass*, pThis, EBP);

	R->ESI<AircraftTypeClass*>(TechnoExt::GetAircraftTypeExtra(pThis));

	return Continue;
}

DEFINE_HOOK(0x414665, AircraftClass_Draw_ExtraSHP, 0x6)
{
	enum { Continue = 0x41466B };

	GET(AircraftClass*, pThis, EBP);

	R->EAX<AircraftTypeClass*>(TechnoExt::GetAircraftTypeExtra(pThis));

	return Continue;
}

// Do not explicitly reset target for KeepTargetOnMove vehicles when issued move command.
DEFINE_HOOK(0x4C7462, EventClass_Execute_KeepTargetOnMove, 0x5)
{
	enum { SkipGameCode = 0x4C74C0 };

	GET(TechnoClass*, pTechno, EDI);

	if (pTechno->WhatAmI() != AbstractType::Unit)
		return 0;

	GET(EventClass*, pThis, ESI);
	auto const mission = static_cast<Mission>(pThis->MegaMission.Mission);
	auto const pExt = TechnoExt::ExtMap.Find(pTechno);

	if (mission == Mission::Move && pExt->TypeExtData->KeepTargetOnMove && pTechno->Target)
	{
		GET(AbstractClass*, pTarget, EBX);

		if (!pTarget && pTechno->IsCloseEnoughToAttack(pTechno->Target))
		{
			auto const pDestination = pThis->MegaMission.Destination.As_Abstract();
			pTechno->SetDestination(pDestination, true);
			pExt->KeepTargetOnMove = true;

			return SkipGameCode;
		}
	}

	pExt->KeepTargetOnMove = false;

	return 0;
}

#pragma region BuildingTypeSelectable

namespace BuildingTypeSelectable
{
	bool ProcessingIDMatches = false;
}

DEFINE_HOOK_AGAIN(0x732B28, TypeSelectExecute_SetContext, 0x6)
DEFINE_HOOK(0x732A85, TypeSelectExecute_SetContext, 0x7)
{
	BuildingTypeSelectable::ProcessingIDMatches = true;
	return 0;
}

// This func has two retn, but one of them is affected by Ares' hook. Thus we only hook the other one.
// If you have any problem, check Ares in IDA before making any changes.
DEFINE_HOOK(0x732C97, TechnoClass_IDMatches_ResetContext, 0x5)
{
	BuildingTypeSelectable::ProcessingIDMatches = false;
	return 0;
}

// If the context is set as well as the flags is enabled, this will make the vfunc CanBeSelectedNow return true to enable the type selection.
DEFINE_HOOK(0x465D40, BuildingClass_Is1x1AndUndeployable_BuildingMassSelectable, 0x6)
{
	enum { SkipGameCode = 0x465D6A };

	// Since Ares hooks around, we have difficulty juggling Ares and no Ares.
	// So we simply disable this feature if no Ares.
	if (!AresHelper::CanUseAres)
		return 0;

	if (!BuildingTypeSelectable::ProcessingIDMatches || !RulesExt::Global()->BuildingTypeSelectable)
		return 0;

	R->EAX(true);
	return SkipGameCode;
}

#pragma endregion

DEFINE_HOOK(0x521D94, InfantryClass_CurrentSpeed_ProneSpeed, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(int, currentSpeed, ECX);

	const auto pType = pThis->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const double multiplier = pTypeExt->ProneSpeed.Get(pType->Crawls ? RulesExt::Global()->ProneSpeed_Crawls : RulesExt::Global()->ProneSpeed_NoCrawls);
	currentSpeed = static_cast<int>(static_cast<double>(currentSpeed) * multiplier);
	R->ECX(currentSpeed);
	return 0x521DC5;
}

DEFINE_HOOK_AGAIN(0x6A343F, LocomotionClass_Process_DamagedSpeedMultiplier, 0x6)// Ship
DEFINE_HOOK(0x4B3DF0, LocomotionClass_Process_DamagedSpeedMultiplier, 0x6)// Drive
{
	GET(FootClass*, pLinkedTo, ECX);
	const auto pTypeExt = TechnoExt::ExtMap.Find(pLinkedTo)->TypeExtData;

	const double multiplier = pTypeExt->DamagedSpeed.Get(RulesExt::Global()->DamagedSpeed);
	__asm fmul multiplier;

	return R->Origin() + 0x6;
}

DEFINE_HOOK(0x62A0AA, ParasiteClass_AI_CullingTarget, 0x5)
{
	enum { ExecuteCulling = 0x62A0B7, CannotCulling = 0x62A0D3 };

	GET(ParasiteClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWarhead, EBP);
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	return EnumFunctions::IsTechnoEligible(pThis->Victim, pWHExt->Parasite_CullingTarget) ? ExecuteCulling : CannotCulling;
}

DEFINE_HOOK(0x6298CC, ParasiteClass_AI_GrippleAnim, 0x5)
{
	enum { SkipGameCode = 0x6298D6 };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x68, -0x4C));
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	R->EAX(pWHExt->Parasite_GrappleAnim.Get(RulesExt::Global()->Parasite_GrappleAnim.Get(AnimTypeClass::FindIndex("SQDG"))));
	return SkipGameCode;
}

#pragma region RadarDrawing

DEFINE_HOOK(0x655DDD, RadarClass_ProcessPoint_RadarInvisible, 0x6)
{
	enum { Invisible = 0x655E66, GoOtherChecks = 0x655E19 };

	GET_STACK(const bool, isInShrouded, STACK_OFFSET(0x40, 0x4));
	GET(TechnoClass*, pTechno, EBP);

	if (isInShrouded && !pTechno->Owner->IsControlledByCurrentPlayer())
		return Invisible;

	auto const pType = pTechno->GetTechnoType();

	if (pType->RadarInvisible)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (EnumFunctions::CanTargetHouse(pTypeExt->RadarInvisibleToHouse.Get(AffectedHouse::Enemies), pTechno->Owner, HouseClass::CurrentPlayer))
			return Invisible;
	}

	return GoOtherChecks;
}

#pragma endregion

#pragma region Customized FallingDown Damage

DEFINE_HOOK(0x5F416A, ObjectClass_DropAsBomb_ResetFallRateRate, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	// Reset value, otherwise it'll keep accelerating.
	pThis->FallRate = 0;
	return 0;
}

DEFINE_HOOK(0x5F4032, ObjectClass_FallingDown_ToDead, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	pThis->FallRate = 0;

	if (const auto pTechno = abstract_cast<TechnoClass*, true>(pThis))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pCell = pTechno->GetCell();

		if (!pCell->IsClearToMove(pType->SpeedType, true, true, -1, pType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
			return 0;

		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		double ratio = 0.0;

		if (pCell->LandType == LandType::Water && !pTechno->OnBridge)
			ratio = pTypeExt->FallingDownDamage_Water.Get(pTypeExt->FallingDownDamage.Get());
		else
			ratio = pTypeExt->FallingDownDamage.Get();

		int damage = 0;

		if (ratio < 0.0)
			damage = static_cast<int>(pThis->Health * std::abs(ratio));
		else if (ratio >= 0.0 && ratio <= 1.0)
			damage = static_cast<int>(pType->Strength * ratio);
		else
			damage = static_cast<int>(ratio);

		pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

		if (pThis->Health > 0 && pThis->IsAlive)
		{
			pThis->IsABomb = false;
			const auto abs = pThis->WhatAmI();

			if (abs == AbstractType::Infantry)
			{
				const auto pInf = static_cast<InfantryClass*>(pTechno);
				const auto sequenceAnim = pInf->SequenceAnim;
				pInf->ShouldDeploy = false;

				if (pCell->LandType == LandType::Water && !pInf->OnBridge)
				{
					if (sequenceAnim != Sequence::Swim)
						pInf->PlayAnim(Sequence::Swim, true, false);
				}
				else if (sequenceAnim != Sequence::Guard)
				{
					pInf->PlayAnim(Sequence::Ready, true, false);
				}

				pInf->Scatter(pInf->GetCoords(), true, false);
			}
			else if (abs == AbstractType::Unit)
			{
				static_cast<UnitClass*>(pTechno)->UpdatePosition(PCPType::During);
			}
		}

		return 0x5F405B;
	}

	return 0;
}

#pragma endregion

#pragma region SetTarget

DEFINE_HOOK_AGAIN(0x4DF3D3, FootClass_UpdateAttackMove_SetTarget, 0xA)
DEFINE_HOOK_AGAIN(0x4DF46A, FootClass_UpdateAttackMove_SetTarget, 0xA)
DEFINE_HOOK(0x4DF4A5, FootClass_UpdateAttackMove_SetTarget, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (R->Origin() != 0x4DF4A5)
	{
		pThis->Target = nullptr;
		pExt->UpdateGattlingRateDownReset();

		return R->Origin() + 0xA;
	}
	else
	{
		GET(AbstractClass*, pTarget, EAX);

		pThis->Target = pTarget;
		pExt->UpdateGattlingRateDownReset();

		return 0x4DF4AB;
	}
}

DEFINE_HOOK(0x6FCF3E, TechnoClass_SetTarget_After, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	if (pThis->LocomotorTarget != pTarget)
		pThis->ReleaseLocomotor(true);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (const auto pUnit = abstract_cast<UnitClass*, true>(pThis))
	{
		const auto pUnitType = pUnit->Type;

		if (!pUnitType->Turret && !pUnitType->Voxel)
		{
			const auto pTypeExt = pExt->TypeExtData;

			if (!pTarget || pTypeExt->FireUp < 0 || pTypeExt->FireUp_ResetInRetarget
				|| !pThis->IsCloseEnough(pTarget, pThis->SelectWeapon(pTarget)))
			{
				pUnit->CurrentFiringFrame = -1;
				pExt->FiringAnimationTimer.Stop();
			}
		}
	}

	pThis->Target = pTarget;
	pExt->UpdateGattlingRateDownReset();

	if (!pThis->Target)
		pExt->ResetDelayedFireTimer();

	return 0x6FCF44;
}

#pragma endregion

DEFINE_JUMP(LJMP, 0x7389B1, 0x7389C4) // Skip ReleaseLocomotor in UnitClass::EnterIdleMode()

DEFINE_HOOK(0x6FABC4, TechnoClass_AI_AnimationPaused, 0x6)
{
	enum { SkipGameCode = 0x6FAC31 };

	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->DelayedFireSequencePaused)
		return SkipGameCode;

	return 0;
}

DEFINE_HOOK(0x519FEC, InfantryClass_UpdatePosition_EngineerRepair, 0xA)
{
	enum { SkipGameCode = 0x51A010 };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pTarget, EDI);
	const bool wasDamaged = pTarget->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	pTarget->Mark(MarkType::Change);

	const auto pTargetType = pTarget->Type;
	const int repairBuilding = TechnoTypeExt::ExtMap.Find(pTargetType)->EngineerRepairAmount;
	const int repairEngineer = TechnoTypeExt::ExtMap.Find(pThis->Type)->EngineerRepairAmount;
	const int strength = pTargetType->Strength;

	auto repair = [strength, pTarget](int repair)
		{
			int repairAmount = strength;

			if (repair > 0)
			{
				repairAmount = std::clamp(pTarget->Health + repair, 0, strength);
			}
			else if (repair < 0)
			{
				const double percentage = std::clamp(pTarget->GetHealthPercentage() - (static_cast<double>(repair) / 100), 0.0, 1.0);
				repairAmount = static_cast<int>(std::round(strength * percentage));
			}

			return repairAmount;
		};

	pTarget->Health = Math::min(repair(repairBuilding), repair(repairEngineer));
	pTarget->EstimatedHealth = pTarget->Health;
	pTarget->SetRepairState(0);

	if ((pTarget->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow) != wasDamaged)
	{
		pTarget->ToggleDamagedAnims(!wasDamaged);

		if (wasDamaged && pTarget->DamageParticleSystem)
			pTarget->DamageParticleSystem->UnInit();
	}

	VocClass::PlayAt(BuildingTypeExt::ExtMap.Find(pTargetType)->BuildingRepairedSound.Get(RulesClass::Instance->BuildingRepairedSound), pTarget->GetCoords());
	return SkipGameCode;
}

#pragma region AttackMove

DEFINE_HOOK(0x4DF410, FootClass_UpdateAttackMove_TargetAcquired, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pThis->IsCloseEnoughToAttack(pThis->Target)
		&& pTypeExt->AttackMove_StopWhenTargetAcquired.Get(RulesExt::Global()->AttackMove_StopWhenTargetAcquired.Get(!pType->OpportunityFire)))
	{
		if (auto const pJumpjetLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
		{
			auto const crd = pThis->GetCoords();
			pJumpjetLoco->DestinationCoords.X = crd.X;
			pJumpjetLoco->DestinationCoords.Y = crd.Y;
			pJumpjetLoco->CurrentSpeed = 0;
			pJumpjetLoco->MaxSpeed = 0;
			pJumpjetLoco->State = JumpjetLocomotionClass::State::Hovering;
			pThis->AbortMotion();
		}
		else
		{
			pThis->StopMoving();
			pThis->AbortMotion();
		}
	}

	if (pTypeExt->AttackMove_PursuitTarget)
		pThis->SetDestination(pThis->Target, true);

	return 0;
}

DEFINE_HOOK(0x4DF4DB, TechnoClass_RefreshMegaMission_CheckMissionFix, 0xA)
{
	enum { ClearMegaMission = 0x4DF4F9, ContinueMegaMission = 0x4DF4CF };

	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const mission = pThis->GetCurrentMission();
	bool stopWhenTargetAcquired = pTypeExt->AttackMove_StopWhenTargetAcquired.Get(RulesExt::Global()->AttackMove_StopWhenTargetAcquired.Get(!pType->OpportunityFire));
	bool clearMegaMission = mission != Mission::Guard;

	if (stopWhenTargetAcquired && clearMegaMission)
		clearMegaMission = !(mission == Mission::Move && pThis->MegaDestination && pThis->DistanceFrom(pThis->MegaDestination) > 256);

	return clearMegaMission ? ClearMegaMission : ContinueMegaMission;
}

DEFINE_HOOK(0x711E90, TechnoTypeClass_CanAttackMove_IgnoreWeapon, 0x6)
{
	enum { SkipGameCode = 0x711E9A };

	return RulesExt::Global()->AttackMove_IgnoreWeaponCheck ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4DF3A6, FootClass_UpdateAttackMove_Follow, 0x6)
{
	enum { FuncRet = 0x4DF425 };

	GET(FootClass* const, pThis, ESI);

	auto const mission = pThis->GetCurrentMission();

	// Refresh mega mission if mission is somehow changed to incorrect missions.
	if (mission != Mission::Attack && mission != Mission::Move)
	{
		bool continueMission = true;

		// Aug 30, 2025 - Starkku: SimpleDeployer needs special handling here.
		// Without this if you interrupt waypoint mode path with deploy command
		// it will not execute properly as it interrupts it with movement.
		if (mission == Mission::Unload)
		{
			if (auto const pUnit = abstract_cast<UnitClass*>(pThis))
			{
				if (pUnit->Type->IsSimpleDeployer)
					continueMission = false;
			}
		}

		if (continueMission)
			pThis->ContinueMegaMission();
	}

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeExt->AttackMove_Follow || pTypeExt->AttackMove_Follow_IfMindControlIsFull && pThis->CaptureManager && pThis->CaptureManager->CannotControlAnyMore())
	{
		auto const& pTechnoVectors = Helpers::Alex::getCellSpreadItems(pThis->GetCoords(),
			pThis->GetGuardRange(2) / (double)Unsorted::LeptonsPerCell, pTypeExt->AttackMove_Follow_IncludeAir);

		TechnoClass* pClosestTarget = nullptr;
		int closestRange = 65536;
		const auto pMegaMissionTarget = pThis->MegaDestination ? pThis->MegaDestination : (pThis->MegaTarget ? pThis->MegaTarget : pThis);
		const auto pOwner = pThis->Owner;

		for (auto const pTechno : pTechnoVectors)
		{
			if ((pTechno->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None
				&& pTechno != pThis && pTechno->Owner == pOwner
				&& pTechno->MegaMissionIsAttackMove())
			{
				auto const pTargetExt = TechnoExt::ExtMap.Find(pTechno);

				// Check this to prevent the followed techno from being surrounded
				if (pTargetExt->AttackMoveFollowerTempCount >= 6)
					continue;

				auto const pTargetTypeExt = pTargetExt->TypeExtData;

				if (!pTargetTypeExt->AttackMove_Follow)
				{
					auto const dist = pTechno->DistanceFrom(pMegaMissionTarget);

					if (dist < closestRange)
					{
						pClosestTarget = pTechno;
						closestRange = dist;
					}
				}
			}
		}

		if (pClosestTarget)
		{
			auto const pTargetExt = TechnoExt::ExtMap.Find(pClosestTarget);
			pTargetExt->AttackMoveFollowerTempCount += pThis->WhatAmI() == AbstractType::Infantry ? 1 : 3;
			pThis->SetDestination(pClosestTarget, false);
			pThis->SetArchiveTarget(pClosestTarget);
			pThis->QueueMission(Mission::Area_Guard, true);
		}
		else
		{
			if (pThis->MegaTarget)
				pThis->SetDestination(pThis->MegaTarget, false);
			else // MegaDestination can be nullptr
				pThis->SetDestination(pThis->MegaDestination, false);
		}

		pThis->ClearMegaMissionData();

		R->EAX(pClosestTarget);
		return FuncRet;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x708FC0, TechnoClass_ResponseMove_Pickup, 0x5)
{
	enum { SkipResponse = 0x709015 };

	GET(TechnoClass*, pThis, ECX);

	const AbstractType rtti = pThis->WhatAmI();

	if (rtti == AbstractType::Aircraft)
	{
		auto const pAircraft = static_cast<AircraftClass*>(pThis);
		auto const pType = pAircraft->Type;

		if (pType->Carryall && pAircraft->HasAnyLink()
			&& generic_cast<FootClass*>(pAircraft->Destination))
		{
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

			if (pTypeExt->VoicePickup.isset())
			{
				pThis->QueueVoice(pTypeExt->VoicePickup.Get());

				R->EAX(1);
				return SkipResponse;
			}
		}
	}
	else if (rtti == AbstractType::Unit)
	{
		auto const pUnit = static_cast<UnitClass*>(pThis);

		if (TechnoExt::CannotMove(pUnit))
			return SkipResponse;
	}

	return 0;
}

// Handle disabling deploy action & cursor for vehicles and aircraft.
// Possible hook locations for other types in same function: Building: 0x700E3F, Infantry: 0x700E2C
DEFINE_HOOK(0x7010C1, TechnoClass_CanShowDeployCursor_UnitsAndAircraft, 0x5)
{
	enum { DoNotAllowDeploy = 0x700DCE };

	GET(FootClass*, pThis, ESI);

	if (auto const pUnit = abstract_cast<UnitClass*>(pThis))
	{
		// If in tank bunker skip rest of the checks.
		if (pThis->BunkerLinkedItem)
			return 0;

		// Ammo-based deploy blocking.
		if (!TechnoExt::HasAmmoToDeploy(pUnit))
			return DoNotAllowDeploy;

		// IsSimpleDeployer + type conversion
		if (pUnit->Type->IsSimpleDeployer && AresFunctions::ConvertTypeTo)
		{
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->Type);

			if (auto const pTypeConvert = pTypeExt->Convert_Deploy)
			{
				auto const pCell = pUnit->GetCell();

				if (!pCell->IsClearToMove(pTypeConvert->SpeedType, true, true, -1, pTypeConvert->MovementZone, -1, pCell->ContainsBridge()))
					return DoNotAllowDeploy;
			}
		}
	}

	return 0;
}

// Handle customized WarpAway
DEFINE_HOOK(0x71A8BD, TemporalClass_Update_WarpAwayAnim, 0x5)
{
	GET(TemporalClass*, pThis, ESI);

	// Target must exist here
	auto const pTarget = pThis->Target;
	auto const pExt = TechnoExt::ExtMap.Find(pTarget)->TypeExtData;

	if (pExt->WarpAway.size() > 0)
	{
		AnimExt::CreateRandomAnim(pExt->WarpAway, pTarget->Location, nullptr, pTarget->Owner);
	}
	else if (auto const pWarpAway = RulesClass::Instance->WarpAway)
	{
		auto const pAnim = GameCreate<AnimClass>(pWarpAway, pTarget->Location);
		AnimExt::SetAnimOwnerHouseKind(pAnim, pTarget->Owner, nullptr, false, true);
	}

	return 0x71A90E;
}

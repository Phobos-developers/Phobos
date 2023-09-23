#include <ScenarioClass.h>
#include "Body.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pType = pThis->GetTechnoType();

	// Set only if unset or type is changed
	// Notice that Ares may handle type conversion in the same hook here, which is executed right before this one thankfully
	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pType)
		pExt->UpdateTypeData(pType);

	pExt->IsInTunnel = false; // TechnoClass::AI is only called when not in tunnel.

	if (pExt->CheckDeathConditions())
		return 0;

	pExt->ApplyInterceptor();
	pExt->EatPassengers();
	pExt->UpdateShield();
	pExt->ApplySpawnLimitRange();
	pExt->UpdateLaserTrails();
	pExt->DepletedAmmoActions();

	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::UpdateUniversalDeploy(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x51BAC7, FootClass_AI_Tunnel, 0x6)//InfantryClass_AI_Tunnel
DEFINE_HOOK(0x7363B5, FootClass_AI_Tunnel, 0x6)//UnitClass_AI_Tunnel
{
	GET(FootClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	// Autodeploy logic for Universal Deploy
	if (pThis->Target || pThis->HasBeenAttacked)
	{
		bool enableAutodeploy = false;

		if (pExt->TypeExtData->Convert_Autodeploy
			&& !pExt->Convert_UniversalDeploy_InProgress
			&& pExt->TypeExtData->Convert_UniversalDeploy.size() > 0)
		{
			if (pThis->Owner->IsControlledByHuman())
			{
				enableAutodeploy = true;
			}
			else
			{
				// AI doesn't have it's own tag enabled: AI will use this tag value instead
				if (!pExt->TypeExtData->Convert_AI_Autodeploy.isset())
					enableAutodeploy = true;
			}
		}

		if (!enableAutodeploy
			&& pExt->TypeExtData->Convert_AI_Autodeploy.isset()
			&& !pThis->Owner->IsControlledByHuman())
		{
			enableAutodeploy = pExt->TypeExtData->Convert_AI_Autodeploy.Get();
		}

		if (enableAutodeploy)
			TechnoExt::AutoDeploy(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipGameSelfHeal;
}

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

	return 0;
}

DEFINE_HOOK(0x4DBF13, FootClass_SetOwningHouse, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto& trail : pExt->LaserTrails)
	{
		if (trail.Type->IsHouseColor)
			trail.CurrentColor = pThis->Owner->LaserColor;
	}

	if (pThis->Owner->IsHumanPlayer)
		TechnoExt::ChangeOwnerMissionFix(pThis);

	return 0;
}

DEFINE_HOOK(0x4483C0, BuildingClass_SetOwningHouse_MuteSound, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	REF_STACK(bool, announce, STACK_OFFSET(0x60, 0x8));

	announce = announce && !pThis->Type->IsVehicle();

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

DEFINE_HOOK(0x702E4E, TechnoClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	if (auto const pInf = abstract_cast<InfantryClass*>(R->EDI<FootClass*>()))
	{
		if (pBuilding && pBuilding->Type->Cloning)
		{
			if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
			{
				double percentage = GeneralUtils::GetRangedRandomOrSingleValue(pTypeExt->InitialStrength_Cloning);
				int strength = Math::clamp(static_cast<int>(pInf->Type->Strength * percentage), 1, pInf->Type->Strength);

				pInf->Health = strength;
				pInf->EstimatedHealth = strength;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets)
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (Phobos::DisplayDamageNumbers && *pDamage)
		TechnoExt::DisplayDamageNumberString(pThis, *pDamage, false);

	return 0;
}

DEFINE_HOOK(0x70265F, TechnoClass_ReceiveDamage_Explodes, 0x6)
{
	enum { SkipKillingPassengers = 0x702669 };

	GET(TechnoClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt->Explodes_KillPassengers)
		return SkipKillingPassengers;

	return 0;
}

// Issue #237 NotHuman additional animations support
// Author: Otamaa
DEFINE_HOOK(0x518505, InfantryClass_ReceiveDamage_NotHuman, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0xD0, 0x4));

	// Die1-Die5 sequences are offset by 10
	constexpr auto Die = [](int x) { return x + 10; };

	int resultSequence = Die(1);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->NotHuman_RandomDeathSequence.Get())
		resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));

	if (receiveDamageArgs.WH)
	{
		if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
		{
			int whSequence = pWarheadExt->NotHuman_DeathSequence.Get();
			if (whSequence > 0)
				resultSequence = Math::min(Die(whSequence), Die(5));
		}
	}

	R->ECX(pThis);
	pThis->PlayAnim(static_cast<Sequence>(resultSequence), true);

	return 0x518515;
}

DEFINE_HOOK(0x7098B9, TechnoClass_TargetSomethingNearby_AutoFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->AutoFire)
		{
			if (pExt->AutoFire_TargetSelf)
				pThis->SetTarget(pThis);
			else
				pThis->SetTarget(pThis->GetCell());

			return 0x7099B8;
		}
	}

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

DEFINE_HOOK(0x6FD446, TechnoClass_LaserZap_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (!pLaser->IsHouseColor && pWeaponExt->Laser_IsSingleColor)
			pLaser->IsHouseColor = true;
	}

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	pLaser->IsSupported = pLaser->Thickness > 3;

	return 0;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
		pExt->UpdateMindControlAnim();

	return 0;
}

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_CloakVisibility, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	// Allow observers to always see cloaked objects.
	// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x45455B, BuildingClass_VisualCharacter_CloakVisibility, 0x5)
{
	enum { UseShadowyVisual = 0x45452D, CheckMutualAlliance = 0x454564 };

	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain_Organics, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));

	enum { MakeInvunlnerable = 0x4DEB38, SkipGameCode = 0x4DEBA2 };

	if (!pType->Organic && pThis->WhatAmI() != AbstractType::Infantry)
		return MakeInvunlnerable;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainEffect icEffect = pTypeExt->IronCurtain_Effect.Get(RulesExt::Global()->IronCurtain_EffectOnOrganics);

	switch (icEffect)
	{
	case IronCurtainEffect::Ignore:
	{
		R->EAX(DamageState::Unaffected);
	}break;
	case IronCurtainEffect::Invulnerable:
	{
		return MakeInvunlnerable;
	}break;
	default:
	{
		R->EAX
		(
			pThis->ReceiveDamage
			(
				&pThis->Health,
				0,
				pTypeExt->IronCurtain_KillWarhead.Get(RulesExt::Global()->IronCurtain_KillOrganicsWarhead.Get(RulesClass::Instance->C4Warhead)),
				nullptr,
				true,
				false,
				pSource
			)
		);
	}break;
	}

	return SkipGameCode;
}

DEFINE_JUMP(VTABLE, 0x7EB1AC, 0x4DEAE0); // Redirect InfantryClass::IronCurtain to FootClass::IronCurtain

namespace MapZoneTemp
{
	TargetZoneScanType zoneScanType;
}

DEFINE_HOOK(0x6F9C67, TechnoClass_GreatestThreat_MapZoneSetContext, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	MapZoneTemp::zoneScanType = pTypeExt->TargetZoneScanType;

	return 0;
}

DEFINE_HOOK(0x6F7E47, TechnoClass_EvaluateObject_MapZone, 0x7)
{
	enum { AllowedObject = 0x6F7EA2, DisallowedObject = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	GET(int, zone, EBP);

	if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
	{
		if (!TechnoExt::AllowedTargetByZone(pThis, pTechno, MapZoneTemp::zoneScanType, nullptr, true, zone))
			return DisallowedObject;
	}

	return AllowedObject;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x98, 0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExt::DrawInsignia(pThis, pLocation, pBounds);

	return SkipGameCode;
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

DEFINE_HOOK(0x54B188, JumpjetLocomotionClass_Process_LayerUpdate, 0x6)
{
	GET(TechnoClass*, pLinkedTo, EAX);

	TechnoExt::UpdateAttachedAnimLayers(pLinkedTo);

	return 0;
}

DEFINE_HOOK(0x4CD4E1, FlyLocomotionClass_Update_LayerUpdate, 0x6)
{
	GET(TechnoClass*, pLinkedTo, ECX);

	if (pLinkedTo->LastLayer != pLinkedTo->InWhichLayer())
		TechnoExt::UpdateAttachedAnimLayers(pLinkedTo);

	return 0;
}

// Move to UnitClass hooks file if it is ever created.
DEFINE_HOOK(0x736234, UnitClass_ChronoSparkleDelay, 0x5)
{
	R->ECX(RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x736239;
}

// Move to InfantryClass hooks file if it is ever created.
DEFINE_HOOK(0x51BAFB, InfantryClass_ChronoSparkleDelay, 0x5)
{
	R->ECX(RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x51BB00;
}

DEFINE_HOOK(0x730B8F, DeployCommand_UniversalDeploy, 0x6)
{
	GET(int, index, EDI);

	TechnoClass* pThis = static_cast<TechnoClass*>(ObjectClass::CurrentObjects->GetItem(index));

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	if (pExt->Convert_UniversalDeploy_InProgress)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (!pTypeExt)
		return 0;

	if (pTypeExt->Convert_UniversalDeploy.size() == 0)
		return 0;
		
	if (pThis->WhatAmI() == AbstractType::Building)
	{
		pExt->Convert_Autodeploy_RememberTarget = pThis->Target;
		pThis->MissionStatus = 0;
		pThis->CurrentMission = Mission::Selling;
		pExt->Convert_UniversalDeploy_InProgress = true;

		return 0x730C10;
	}
	else if (pThis->WhatAmI() == AbstractType::Unit)
	{
		// Abort if the cell is occupied
		int nObjects = 0;

		for (auto pObject = pThis->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
		{
			auto const pItem = static_cast<TechnoClass*>(pObject);

			if (pItem && pItem != pThis)
				nObjects++;
		}

		if (nObjects > 0)
		{
			CoordStruct loc = CoordStruct::Empty;
			pThis->Scatter(loc, true, false);

			return 0;
		}

		// Stop and rotate if needed
		auto const pFoot = static_cast<FootClass*>(pThis);

		if (!pThis->IsFallingDown && pThis->CurrentMission != Mission::Guard)
		{
			pExt->Convert_Autodeploy_RememberTarget = pThis->Target;
			pFoot->SetDestination(pThis, false);
			pFoot->Locomotor->Stop_Moving();
		}

		// Initialize the conversion
		if (pTypeExt->Convert_DeployToLand)
		{
			auto newCell = MapClass::Instance->GetCellAt(pThis->Location);

			// If the cell is occupied abort operation
			if (pThis->GetHeight() > 0 &&
				pThis->IsCellOccupied(newCell, FacingType::None, -1, nullptr, false) != Move::OK)
			{
				pExt->Convert_Autodeploy_RememberTarget = nullptr;
				return 0;
			}

			pThis->IsFallingDown = true;
		}

		pExt->Convert_UniversalDeploy_InProgress = true;

		return 0x730C10;
	}

	return 0;
}

DEFINE_HOOK(0x522510, InfantryClass_UniversalDeploy_DoingDeploy, 0x6)
{
	GET(InfantryClass*, pThis, ECX);

	if (!pThis)
		return 0;

	auto pOldTechno = static_cast<TechnoClass*>(pThis);
	if (!pOldTechno)
		return 0;

	auto const pOldTechnoExt = TechnoExt::ExtMap.Find(pOldTechno);
	if (!pOldTechnoExt)
		return 0;

	if (pOldTechnoExt->Convert_UniversalDeploy_InProgress)
		return 0;

	auto const pOldTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pOldTechno->GetTechnoType());
	if (!pOldTechnoTypeExt)
		return 0;

	// Preparing UniversalDeploy logic
	int nObjects = 0;

	for (auto pObject = pOldTechno->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
	{
		auto const pItem = static_cast<TechnoClass*>(pObject);

		if (pItem && pItem != pOldTechno)
			nObjects++;
	}

	if (nObjects > 0)
	{
		CoordStruct loc = CoordStruct::Empty;
		pOldTechno->Scatter(loc, true, false);

		return 0;
	}

	if (pOldTechnoTypeExt->Convert_DeployToLand)
	{
		auto newCell = MapClass::Instance->GetCellAt(pThis->Location);

		// If the cell is occupied abort operation
		if (pThis->GetHeight() > 0 &&
			pThis->IsCellOccupied(newCell, FacingType::None, -1, nullptr, false) != Move::OK)
		{
			pOldTechnoExt->Convert_Autodeploy_RememberTarget = nullptr;
			return 0;
		}

		pThis->IsFallingDown = true;
	}

	pOldTechnoExt->Convert_Autodeploy_RememberTarget = pOldTechno->Target;
	pOldTechnoExt->Convert_UniversalDeploy_InProgress = true;

	return 0;
}

DEFINE_HOOK(0x449E6B, BuildingClass_MissionDeconstruction_UniversalDeploy, 0x5)
{
	GET(UnitClass*, pUnit, EBX);
	GET(BuildingClass*, pBuilding, ECX);

	if (!pUnit || !pBuilding)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());
	if (!pTypeExt)
		return 0;

	if (pTypeExt && pTypeExt->Convert_UniversalDeploy.size() == 0)
		return 0;

	pUnit->Limbo();

	auto pOldTechno = static_cast<TechnoClass*>(pBuilding);
	if (!pOldTechno)
		return 0;

	CoordStruct deployLocation = pOldTechno->GetCoords();
	auto deployed = TechnoExt::UniversalConvert(pOldTechno, nullptr);

	if (deployed)
	{
		if (pTypeExt->Convert_AnimFX.isset())
		{
			const auto pAnimType = pTypeExt->Convert_AnimFX.Get();
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, deployLocation))
			{
				if (pTypeExt->Convert_AnimFX_FollowDeployer)
					pAnim->SetOwnerObject(deployed);

				pAnim->Owner = deployed->Owner;
			}
		}

		pUnit->UnInit();
		return 0x44A1D8;
	}

	// Restoring unit after a failed deploy attempt. I don't know if this can happen.
	if (pTypeExt->Convert_DeployDir >= 0)
	{
		DirStruct desiredFacing;
		desiredFacing.SetDir(static_cast<DirType>(pTypeExt->Convert_DeployDir * 32));
		pUnit->PrimaryFacing.SetCurrent(desiredFacing);
	}
	else
	{
		pUnit->Unlimbo(pUnit->Location, DirType::North);
	}

	return 0;
}

DEFINE_HOOK(0x44725F, BuildingClass_WhatAction_UniversalDeploy_EnableDeployIcon, 0x5)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET_STACK(FootClass*, pFoot, 0x1C);
	
	if (!pBuilding)
		return 0;

	if (!pFoot)
		return 0;

	auto pFootTechno = static_cast<TechnoClass*>(pFoot);
	if (!pFootTechno)
		return 0;

	if (pFoot->Location == pBuilding->Location)
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());
		if (!pTypeExt)
			return 0;

		if (pTypeExt->Convert_UniversalDeploy.size() > 0)
		{
			R->EAX(Action::Self_Deploy);
			return 0x447273;
		}
	}

	return 0;
}

DEFINE_HOOK(0x457DE9, BuildingClass_EvictOccupiers_UniversalDeploy_DontEjectOccupiers, 0xC)
{
	GET(BuildingClass*, pBuilding, ECX);

	if (!pBuilding)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);
	if (!pTypeExt)
		return 0;

	// Don't eject the infantry if the UniversalDeploy is being used. UniversalDeploy Manages that operation
	if (pTypeExt->Convert_UniversalDeploy.size() > 0)
		return 0x4581DB;

	return 0;
}

DEFINE_HOOK(0x4ABEE9, BuildingClass_MouseLeftRelease_UniversalDeploy_ExecuteDeploy, 0x7)
{
	GET(TechnoClass* const, pTechno, ESI);
	GET(Action const, actionType, EBX);

	if (!pTechno)
		return 0;

	if (!pTechno->IsSelected)
		return 0;

	if (actionType != Action::Self_Deploy)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt)
		return 0;

	// Don't allow start again the process if it is still in process
	if (pExt->Convert_UniversalDeploy_InProgress)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
	if (!pTypeExt)
		return 0;

	if (pTypeExt->Convert_UniversalDeploy.size() == 0)
		return 0;

	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		pExt->Convert_Autodeploy_RememberTarget = pTechno->Target;
		R->EBX(Action::None);
		pTechno->MissionStatus = 0;
		pTechno->CurrentMission = Mission::Selling;
		pExt->Convert_UniversalDeploy_InProgress = true;
	}
	else if (pTechno->WhatAmI() == AbstractType::Unit)
	{
		int nObjects = 0;

		for (auto pObject = pTechno->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
		{
			auto const pItem = static_cast<TechnoClass*>(pObject);

			if (pItem && pItem != pTechno)
				nObjects++;
		}

		// If the cell is occupied abort operation
		if (nObjects > 0)
		{
			CoordStruct loc = CoordStruct::Empty;
			pTechno->Scatter(loc, true, false);

			return 0;
		}

		auto pFoot = static_cast<FootClass*>(pTechno);

		if (!pTechno->IsFallingDown && pTechno->CurrentMission != Mission::Guard)
		{
			pExt->Convert_Autodeploy_RememberTarget = pTechno->Target;
			// Can not be converted if the object is moving
			pFoot->SetDestination(pTechno, false);
			pFoot->Locomotor->Stop_Moving();
		}

		// Start the conversion
		auto newCell = MapClass::Instance->GetCellAt(pTechno->Location);

		if (pTypeExt->Convert_DeployToLand)
		{
			// If the cell is occupied abort operation
			if (pTechno->GetHeight() > 0 &&
				pTechno->IsCellOccupied(newCell, FacingType::None, -1, nullptr, false) != Move::OK)
			{
				pExt->Convert_Autodeploy_RememberTarget = nullptr;
				return 0;
			}

			pTechno->IsFallingDown = true;
			pFoot->ParalysisTimer.Start(15);
		}

		pExt->Convert_UniversalDeploy_InProgress = true;
	}

	return 0;
}

DEFINE_HOOK(0x73B4DA, UnitClass_DrawVoxel_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x73C5DC };

	GET(UnitClass*, pThis, EBP);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt)
		return 0;

	// VXL units won't draw graphics when deploy
	if (pExt->Convert_UniversalDeploy_MakeInvisible)
		return Skip;

	return 0;
}

// Probably obsolete since I hooked DrawVoxel
/*DEFINE_HOOK(0x5F4CF1, ObjectClass_Render_UniversalDeploy_DontRenderObject, 0x9)
{
	enum { Skip = 0x5F4D09 };

	GET(ObjectClass*, pThis, ECX);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt)
		return 0;

	// Some objects won't draw graphics when deploy
	if (pExt->Convert_UniversalDeploy_MakeInvisible)
		return Skip;

	return 0;
}*/

DEFINE_HOOK(0x73C602, TechnoClass_DrawObject_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x73CE00 };

	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	// SHP units won't draw graphics when deploy
	if (pExt->Convert_UniversalDeploy_MakeInvisible)
		return Skip;

	return 0;
}

DEFINE_HOOK(0x518FBC, InfantryClass_DrawIt_UniversalDeploy_DontRenderObject, 0x6)
{
	enum { Skip = 0x5192B5 };

	GET(InfantryClass*, pThis, EBP);

	if (!pThis)
		return 0;

	auto pTechno = static_cast<TechnoClass*>(pThis);
	if (!pTechno)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pExt)
		return 0;

	// Here enters SHP units when deploy
	if (pExt->Convert_UniversalDeploy_MakeInvisible)
		return Skip;

	return 0;
}

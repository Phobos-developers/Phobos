#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::ApplyInterceptor(pThis);
	TechnoExt::ApplyPowered_KillSpawns(pThis);
	TechnoExt::ApplySpawn_LimitRange(pThis);
	TechnoExt::CheckDeathConditions(pThis);
	TechnoExt::EatPassengers(pThis);
	TechnoExt::UpdateMindControlAnim(pThis);
	TechnoExt::UpdateUniversalDeploy(pThis);

	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail : pExt->LaserTrails)
		trail->Update(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));

	return 0;
}


DEFINE_HOOK(0x6F42F7, TechnoClass_Init_NewEntities, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::InitializeShield(pThis);
	TechnoExt::InitializeLaserTrails(pThis);

	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_Save_Killer_Techno, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK_AGAIN(0x7355C0, TechnoClass_Init_InitialStrength, 0x6) // UnitClass_Init
DEFINE_HOOK_AGAIN(0x517D69, TechnoClass_Init_InitialStrength, 0x6) // InfantryClass_Init
DEFINE_HOOK_AGAIN(0x442C7B, TechnoClass_Init_InitialStrength, 0x6) // BuildingClass_Init
DEFINE_HOOK(0x414057, TechnoClass_Init_InitialStrength, 0x6)       // AircraftClass_Init
{
	GET(TechnoClass*, pThis, ESI);

	if (R->Origin() != 0x517D69)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (R->Origin() != 0x442C7B)
				R->EAX(pTypeExt->InitialStrength.Get(R->EAX<int>()));
			else
				R->ECX(pTypeExt->InitialStrength.Get(R->ECX<int>()));
		}
	}
	else if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		auto strength = pTypeExt->InitialStrength.Get(R->EDX<int>());
		pThis->Health = strength;
		pThis->EstimatedHealth = strength;
	}

	return 0;
}

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(FootClass*, pFoot, EDI);

	bool isCloner = false;

	if (pBuilding && pBuilding->Type->Cloning)
		isCloner = true;

	if (isCloner && pFoot)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType()))
		{
			if (auto pTypeUnit = pFoot->GetTechnoType())
			{
				Vector2D<double> range = pTypeExt->InitialStrength_Cloning.Get();
				int min = static_cast<int>(range.X * 100);
				int max = static_cast<int>(range.Y * 100);
				double percentage = range.X >= range.Y ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
				int strength = static_cast<int>(pTypeUnit->Strength * percentage);

				if (strength <= 0)
					strength = 1;

				pFoot->Health = strength;
				pFoot->EstimatedHealth = strength;
			}
		}
	}

	return 0;
}

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_Rearm_Delay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int burstDelay = -1;

	if (pWeaponExt->Burst_Delays.size() > (unsigned)pThis->CurrentBurstIndex)
		burstDelay = pWeaponExt->Burst_Delays[pThis->CurrentBurstIndex - 1];
	else if (pWeaponExt->Burst_Delays.size() > 0)
		burstDelay = pWeaponExt->Burst_Delays[pWeaponExt->Burst_Delays.size() - 1];

	if (burstDelay >= 0)
	{
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	// Restore overridden instructions
	GET(int, idxCurrentBurst, ECX);
	return idxCurrentBurst <= 0 || idxCurrentBurst > 4 ? 0x6FD084 : 0x6FD067;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	FLH = TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (!FLHFound)
	{
		if (auto pInf = abstract_cast<InfantryClass*>(pThis))
			FLH = TechnoExt::GetSimpleFLH(pInf, weaponIndex, FLHFound);
	}

	if (FLHFound)
	{
		R->ECX(FLH.X);
		R->EBP(FLH.Y);
		R->EAX(FLH.Z);
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));
	bool FLHFound = false;

	TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (FLHFound)
		R->EAX(0);

	return 0;
}

// Issue #237 NotHuman additional animations support
// Author: Otamaa
DEFINE_HOOK(0x518505, InfantryClass_TakeDamage_NotHuman, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0xD0, -0x4));

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

// Author: Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1  ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

// Customizable OpenTopped Properties
// Author: Otamaa

DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0xC)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pTransport = pThis->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EAX(pExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
			return 0x6F72DE;
		}
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0xC)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto pTransport = pThis->Owner->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EDX(pExt->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
			return 0x71A838;
		}
	}

	return 0;
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

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets)
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x73DE90, UnitClass_SimpleDeployer_TransferLaserTrails, 0x6)
{
	GET(UnitClass*, pUnit, ESI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pUnit);
	if (!pTechnoExt)
		return 0;

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());
	if (!pTechnoTypeExt)
		return 0;
	
	// LaserTrails transfer
	if (pTechnoExt && pTechnoTypeExt)
	{
		if (pTechnoExt->LaserTrails.size())
			pTechnoExt->LaserTrails.clear();

		for (auto const& entry : pTechnoTypeExt->LaserTrailData)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[entry.idxType].get())
			{
				pTechnoExt->LaserTrails.push_back(std::make_unique<LaserTrailClass>(
					pLaserType, pUnit->Owner, entry.FLH, entry.IsOnTurret));
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

	if (pTechnoExt && pTechnoTypeExt)
	{
		for (auto& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation = { };
		}
	}

	return 0;
}

DEFINE_HOOK(0x5F4F4E, ObjectClass_Unlimbo_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, ECX);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	if (pTechnoExt)
	{
		for (auto& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->LastLocation = { };
			pLaserTrail->Visible = true;
		}
	}

	return 0;
}

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

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

	return 0;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (Phobos::Debug_DisplayDamageNumbers && *pDamage)
		TechnoExt::DisplayDamageNumberString(pThis, *pDamage, false);

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipGameSelfHeal;
}


DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C2, TechnoClass_WeaponRange, 0x8)
{
	enum { ReturnResult = 0x70138F };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0x8, -0x4));

	int result = 0;
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pWeapon)
	{
		result = pWeapon->Range;
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pThis->GetTechnoType()->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding)
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else if (pPassenger->GetTechnoType()->TurretCount > 0)
					tWeaponIndex = pPassenger->CurrentWeaponNumber;

				WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType;

				if (pTWeapon)
				{
					if (pTWeapon->Range < smallestRange)
						smallestRange = pTWeapon->Range;
				}

				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	R->EBX(result);
	return ReturnResult;
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

	if (pTypeExt->Convert_UniversalDeploy.size() > 0)
	{
		if (pThis->WhatAmI() == AbstractType::Building)
		{

			pThis->MissionStatus = 0;
			pThis->CurrentMission = Mission::Selling;
			pExt->Convert_UniversalDeploy_InProgress = true;

			return 0x730C10;
		}
		else if (pThis->WhatAmI() == AbstractType::Unit)
		{
			if (pTypeExt->Convert_DeployToLand)
			{
				auto newCell = MapClass::Instance->GetCellAt(pThis->Location);

				// If the cell is occupied abort operation
				if (pThis->GetHeight() > 0 &&
					pThis->IsCellOccupied(newCell, -1, -1, nullptr, false) != Move::OK)
					return 0;

				pThis->IsFallingDown = true;
			}

			pExt->Convert_UniversalDeploy_InProgress = true;

			return 0x730C10;
		}
	}

	return 0;
}

DEFINE_HOOK(0x522510, InfantryClass_DoingDeploy, 0x6)
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

	// Preparing UniversalDeploy logic
	auto const pOldTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pOldTechno->GetTechnoType());
	if (!pOldTechnoTypeExt)
		return 0;

	if (pOldTechnoTypeExt->Convert_DeployToLand)
	{
		auto newCell = MapClass::Instance->GetCellAt(pThis->Location);

		// If the cell is occupied abort operation
		if (pThis->GetHeight() > 0 &&
			pThis->IsCellOccupied(newCell, -1, -1, nullptr, false) != Move::OK)
			return 0;

		pThis->IsFallingDown = true;
	}

	pOldTechnoExt->Convert_UniversalDeploy_InProgress = true;

	return 0;
}

DEFINE_HOOK(0x449C38, BuildingClass_MissionDeconstruction_UniversalDeploy_1, 0x6)
{
	GET(BuildingClass*, pBuilding, ECX);

	if (!pBuilding)
		return 0;

	if (!pBuilding->Type->UndeploysInto)
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());
		if (!pTypeExt)
			return 0;

		if (pTypeExt && pTypeExt->Convert_UniversalDeploy.size() > 0)
		{
			for (auto techno : *TechnoTypeClass::Array)
			{
				if (techno->WhatAmI() == AbstractType::UnitType)
				{
					// Note: This hack is for deleting the need of a UndeploysInto tag in the "Building into Object" case.
					// In every mod exist vehicles so we use the first vehicle in [VehicleTypes] for a dummy hack. This unit won't appear because will be replaced by the designated object.
					pBuilding->Type->UndeploysInto = static_cast<UnitTypeClass*>(techno);
					break;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x449E6B, BuildingClass_MissionDeconstruction_UniversalDeploy_2, 0x5)
{
	GET(UnitClass*, pUnit, EBX);
	GET(BuildingClass*, pBuilding, ECX);

	if (!pUnit || !pBuilding)
		return 0;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());
	if (!pTypeExt)
		return 0;

	if (pTypeExt && pTypeExt->Convert_UniversalDeploy.size() > 0)
	{
		pUnit->Limbo();

		if (auto pOldTechno = static_cast<TechnoClass*>(pBuilding))
		{
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
		}
	}

	// Restoring unit after a failed deploy attempt
	pUnit->Unlimbo(pUnit->Location, Direction::North);

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

	if (auto pFootTechno = static_cast<TechnoClass*>(pFoot))
	{
		if (pFoot->Location == pBuilding->Location)
		{
			if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType()))
			{
				if (pTypeExt->Convert_UniversalDeploy.size() > 0)
				{
					R->EAX(Action::Self_Deploy);
					return 0x447273;
				}
			}
		}
	}

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

	if (pExt->Convert_UniversalDeploy_InProgress)
		return 0;

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()))
	{
		if (pTypeExt->Convert_UniversalDeploy.size() > 0)
		{
			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				R->EBX(Action::None);
				pTechno->MissionStatus = 0;
				pTechno->CurrentMission = Mission::Selling;
				pExt->Convert_UniversalDeploy_InProgress = true;
				
				return 0;
			}
			else if (pTechno->WhatAmI() == AbstractType::Unit)
			{
				auto newCell = MapClass::Instance->GetCellAt(pTechno->Location);

				if (pTypeExt->Convert_DeployToLand)
				{
					// If the cell is occupied abort operation
					if (pTechno->GetHeight() > 0 &&
						pTechno->IsCellOccupied(newCell, -1, -1, nullptr, false) != Move::OK)
						return 0;

					pTechno->IsFallingDown = true;
				}

				pExt->Convert_UniversalDeploy_InProgress = true;

				return 0;
			}
		}
	}

	return 0;
}

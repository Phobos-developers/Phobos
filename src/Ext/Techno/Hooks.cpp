#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <GameStrings.h>
#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Building/Body.h>
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

	if (pExt->CheckDeathConditions())
		return 0;

	pExt->ApplyInterceptor();
	pExt->EatPassengers();
	pExt->UpdateShield();
	pExt->ApplySpawnLimitRange();

	pExt->IsInTunnel = false; // TechnoClass::AI is only called when not in tunnel.

	TechnoExt::ApplyMindControlRangeLimit(pThis);

	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail : pExt->LaserTrails)
	{
		if (pThis->CloakState == CloakState::Cloaked && !trail->Type->CloakVisible)
			continue;

		if (!pExt->IsInTunnel)
			trail->Visible = true;

		CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret);
		if (pThis->CloakState == CloakState::Uncloaking && !trail->Type->CloakVisible)
			trail->LastLocation = trailLoc;
		else
			trail->Update(trailLoc);
	}

	return 0;
}

DEFINE_HOOK(0x51BAC7, InfantryClass_AI_Tunnel, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	return 0;
}

DEFINE_HOOK(0x7363B5, UnitClass_AI_Tunnel, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	return 0;
}

DEFINE_HOOK(0x6F42F7, TechnoClass_Init, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->TypeExtData)
		pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt->TypeExtData)
		pExt->CurrentShieldType = pExt->TypeExtData->ShieldType;

	pExt->InitializeLaserTrails();

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

	if (auto const pInf = abstract_cast<InfantryClass*>(R->EDI<FootClass*>()))
	{
		if (pBuilding && pBuilding->Type->Cloning)
		{
			if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
			{
				Vector2D<double> range = pTypeExt->InitialStrength_Cloning.Get();
				int min = static_cast<int>(range.X * 100);
				int max = static_cast<int>(range.Y * 100);
				double percentage = range.X >= range.Y ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
				int strength = Math::clamp(static_cast<int>(pInf->Type->Strength * percentage), 1, pInf->Type->Strength);

				pInf->Health = strength;
				pInf->EstimatedHealth = strength;
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
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));
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
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));
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

// Author: Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
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
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets)
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (pTechnoExt)
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

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	pLaser->IsSupported = pLaser->Thickness > 3;

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

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6)
{
	enum { KillTheSlave = 0x6B0BDF, ChangeSlaveOwner = 0x6B0BB4 };

	GET(InfantryClass*, pSlave, ESI);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->Type))
	{
		switch (pTypeExt->Slaved_OwnerWhenMasterKilled.Get())
		{
		case SlaveChangeOwnerType::Suicide:
			return KillTheSlave;

		case SlaveChangeOwnerType::Master:
			R->EAX(pSlave->Owner);
			return ChangeSlaveOwner;

		case SlaveChangeOwnerType::Neutral:
			if (auto pNeutral = HouseClass::FindNeutral())
			{
				R->EAX(pNeutral);
				return ChangeSlaveOwner;
			}

		default: // SlaveChangeOwnerType::Killer
			return 0x0;
		}
	}

	return 0x0;
}

// Fix slaves cannot always suicide due to armor multiplier or something
DEFINE_PATCH(0x6B0BF7,
	0x6A, 0x01  // push 1       // ignoreDefense=false->true
);

DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x74, 0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x74, 0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C2, TechnoClass_WeaponRange, 0x8)
{
	enum { ReturnResult = 0x70138F };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, STACK_OFFSET(0x8, 0x4));

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
				else
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType;

				if (pTWeapon && pTWeapon->FireInTransport)
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

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A0, TechnoClass_GetTurretWeapon_LaserWeapon, 0x5)
{
	enum { ReturnResult = 0x70E1C8 };

	GET(TechnoClass* const, pThis, ECX);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		if (auto const pExt = BuildingExt::ExtMap.Find(pBuilding))
		{
			if (!pExt->CurrentLaserWeaponIndex.empty())
			{
				auto weaponStruct = pThis->GetWeapon(pExt->CurrentLaserWeaponIndex.get());
				R->EAX(weaponStruct);
				return ReturnResult;
			}
		}
	}

	return 0;
}

// SellSound and EVA dehardcode
DEFINE_HOOK(0x449CC1, BuildingClass_Mission_Deconstruction_EVA_Sold_1, 0x6)
{
	enum { SkipVoxPlay = 0x449CEA };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pTypeExt->EVA_Sold.isset())
	{
		if (pThis->IsOwnedByCurrentPlayer && !pThis->Type->UndeploysInto)
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get());

		return SkipVoxPlay;
	}

	return 0x0;
}

DEFINE_HOOK(0x44AB22, BuildingClass_Mission_Deconstruction_EVA_Sold_2, 0x6)
{
	enum { SkipVoxPlay = 0x44AB3B };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pTypeExt->EVA_Sold.isset())
	{
		if (pThis->IsOwnedByCurrentPlayer)
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get());

		return SkipVoxPlay;
	}

	return 0x0;
}

DEFINE_HOOK(0x44A850, BuildingClass_Mission_Deconstruction_Sellsound, 0x6)
{
	enum { PlayVocLocally = 0x44A856 };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pTypeExt->SellSound.isset())
	{
		R->ECX(pTypeExt->SellSound.Get());
		return PlayVocLocally;
	}

	return 0x0;
}

DEFINE_HOOK(0x4D9F8A, FootClass_Sell_Sellsound, 0x5)
{
	enum { SkipVoxVocPlay = 0x4D9FB5 };
	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_UnitSold)));
	//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
	VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);

	return SkipVoxVocPlay;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::UpdateMindControlAnim(pThis);

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

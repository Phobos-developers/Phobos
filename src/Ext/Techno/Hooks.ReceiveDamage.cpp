#include "Body.h"

#include <TacticalClass.h>
#include <RadarEventClass.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/House/Body.h>

namespace ReceiveDamageTemp
{
	bool SkipLowDamageCheck = false;
}

// #issue 88 : shield logic
DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, 0x4);

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	int& damage = *args->Damage;

	// AffectsAbove/BelowPercent & AffectsNeutral can ignore IgnoreDefenses like AffectsAllies/Enmies/Owner
	// They should be checked here to cover all cases that directly use ReceiveDamage to deal damage
	if (!pWHExt->IsHealthInThreshold(pThis) || (!pWHExt->AffectsNeutral && pThis->Owner->IsNeutral()))
	{
		damage = 0;
		return 0;
	}

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pSourceHouse = args->SourceHouse;
	const auto pTargetHouse = pThis->Owner;

	// Calculate Damage Multiplier
	if (!args->IgnoreDefenses && damage)
	{
		double multiplier = 1.0;

		if (!pSourceHouse || !pTargetHouse || !pSourceHouse->IsAlliedWith(pTargetHouse))
			multiplier = pWHExt->DamageEnemiesMultiplier.Get(RulesExt::Global()->DamageEnemiesMultiplier);
		else if (pSourceHouse != pTargetHouse)
			multiplier = pWHExt->DamageAlliesMultiplier.Get(!pWHExt->AffectsEnemies ? RulesExt::Global()->DamageAlliesMultiplier_NotAffectsEnemies.Get(RulesExt::Global()->DamageAlliesMultiplier) : RulesExt::Global()->DamageAlliesMultiplier);
		else
			multiplier = pWHExt->DamageOwnerMultiplier.Get(!pWHExt->AffectsEnemies ? RulesExt::Global()->DamageOwnerMultiplier_NotAffectsEnemies.Get(RulesExt::Global()->DamageOwnerMultiplier) : RulesExt::Global()->DamageOwnerMultiplier);

		if (pWHExt->DamageSourceHealthMultiplier && args->Attacker)
			multiplier += pWHExt->DamageSourceHealthMultiplier * args->Attacker->GetHealthPercentage();

		if (pWHExt->DamageTargetHealthMultiplier)
			multiplier += pWHExt->DamageTargetHealthMultiplier * pThis->GetHealthPercentage();

		if (multiplier != 1.0)
		{
			const auto sgnDamage = damage > 0 ? 1 : -1;
			const auto calculateDamage = static_cast<int>(damage * multiplier);
			damage = calculateDamage ? calculateDamage : sgnDamage;
		}
	}

	// Raise Combat Alert
	if (RulesExt::Global()->CombatAlert && damage > 1)
	{
		auto raiseCombatAlert = [&]()
		{
			if (!pTargetHouse->IsControlledByCurrentPlayer() || (RulesExt::Global()->CombatAlert_SuppressIfAllyDamage && pTargetHouse->IsAlliedWith(pSourceHouse)))
				return;

			const auto pHouseExt = HouseExt::ExtMap.Find(pTargetHouse);

			if (pHouseExt->CombatAlertTimer.HasTimeLeft() || pWHExt->CombatAlert_Suppress.Get(!pWHExt->Malicious || pWHExt->Nonprovocative))
				return;

			const auto pTypeExt = pExt->TypeExtData;
			const auto pType = pTypeExt->OwnerObject();

			if (!pTypeExt->CombatAlert.Get(RulesExt::Global()->CombatAlert_Default.Get(!pType->Insignificant && !pType->Spawned)) || !pThis->IsInPlayfield)
				return;

			const auto pBuilding = abstract_cast<BuildingClass*>(pThis);

			if (RulesExt::Global()->CombatAlert_IgnoreBuilding && pBuilding && !pTypeExt->CombatAlert_NotBuilding.Get(pBuilding->Type->IsVehicle()))
				return;

			const auto coordInMap = pThis->GetCoords();

			if (RulesExt::Global()->CombatAlert_SuppressIfInScreen)
			{
				const auto pTactical = TacticalClass::Instance;
				const auto coordInScreen = pTactical->CoordsToScreen(coordInMap) - pTactical->TacticalPos;
				const auto screenArea = DSurface::Composite->GetRect();

				if (screenArea.Width >= coordInScreen.X && screenArea.Height >= coordInScreen.Y && coordInScreen.X >= 0 && coordInScreen.Y >= 0) // check if the unit is in screen
					return;
			}

			pHouseExt->CombatAlertTimer.Start(RulesExt::Global()->CombatAlert_Interval);
			RadarEventClass::Create(RadarEventType::Combat, CellClass::Coord2Cell(coordInMap));
			int index = -1;

			if (!RulesExt::Global()->CombatAlert_MakeAVoice) // No one want to play two sound at a time, I guess?
				return;
			else if (pTypeExt->CombatAlert_UseFeedbackVoice.Get(RulesExt::Global()->CombatAlert_UseFeedbackVoice) && pType->VoiceFeedback.Count > 0) // Use VoiceFeedback first
				VocClass::PlayGlobal(pType->VoiceFeedback.GetItem(0), 0x2000, 1.0);
			else if (pTypeExt->CombatAlert_UseAttackVoice.Get(RulesExt::Global()->CombatAlert_UseAttackVoice) && pType->VoiceAttack.Count > 0) // Use VoiceAttack then
				VocClass::PlayGlobal(pType->VoiceAttack.GetItem(0), 0x2000, 1.0);
			else if (pTypeExt->CombatAlert_UseEVA.Get(RulesExt::Global()->CombatAlert_UseEVA)) // Use Eva finally
				index = pTypeExt->CombatAlert_EVA.Get(VoxClass::FindIndex((const char*)"EVA_UnitsInCombat"));

			if (index != -1)
				VoxClass::PlayIndex(index);
		};
		raiseCombatAlert();
	}

	// Shield Receive Damage
	if (!args->IgnoreDefenses)
	{
		int nDamageLeft = damage;

		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsActive())
			{
				nDamageLeft = pShieldData->ReceiveDamage(args);

				if (nDamageLeft >= 0)
				{
					damage = nDamageLeft;

					if (const auto pTag = pThis->AttachedTag)
						pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::ShieldBroken, pThis, CellStruct::Empty);
				}

				if (nDamageLeft == 0)
					ReceiveDamageTemp::SkipLowDamageCheck = true;
			}
			else if (!pShieldData->IsAvailable() || pShieldData->GetHP() <= 0)
			{
				pShieldData->SetRespawnRestartInCombat();
			}
		}

		if ((!pWHExt->CanKill || pExt->AE.Unkillable)
			&& pThis->Health > 0 && nDamageLeft != 0
			&& pWHExt->CanTargetHouse(pSourceHouse, pThis)
			&& MapClass::GetTotalDamage(nDamageLeft, args->WH, pThis->GetTechnoType()->Armor, args->DistanceToEpicenter) >= pThis->Health)
		{
			// Update remaining damage and check if the target will die and should be avoided
			damage = 0;
			pThis->Health = 1;
			pThis->EstimatedHealth = 1;
			ReceiveDamageTemp::SkipLowDamageCheck = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 0x5)
{
	if (ReceiveDamageTemp::SkipLowDamageCheck)
	{
		ReceiveDamageTemp::SkipLowDamageCheck = false;
	}
	else
	{
		// Restore overridden instructions
		GET(int*, nDamage, EBX);
		if (*nDamage < 1)
			*nDamage = 1;
	}

	return 0x7019E3;
}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (auto const pExt = WarheadTypeExt::ExtMap.TryFind(pWarhead))
	{
		if (pExt->DecloakDamagedTargets)
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	if (!Phobos::DisplayDamageNumbers)
		return 0;

	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (*pDamage)
		GeneralUtils::DisplayDamageNumberString(*pDamage, DamageDisplayType::Regular, pThis->GetRenderCoords(), TechnoExt::ExtMap.Find(pThis)->DamageNumberOffset);

	return 0;
}

DEFINE_HOOK(0x702603, TechnoClass_ReceiveDamage_Explodes, 0x6)
{
	enum { SkipExploding = 0x702672, SkipKillingPassengers = 0x702669 };

	GET(TechnoClass*, pThis, ESI);

	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (!pTypeExt->Explodes_DuringBuildup && (pThis->CurrentMission == Mission::Construction || pThis->CurrentMission == Mission::Selling))
			return SkipExploding;
	}

	if (!pTypeExt->Explodes_KillPassengers)
		return SkipKillingPassengers;

	return 0;
}

DEFINE_HOOK(0x702672, TechnoClass_ReceiveDamage_RevengeWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	TechnoExt::ApplyKillWeapon(pThis, pSource, pWarhead);

	if (pSource)
		TechnoExt::ApplyRevengeWeapon(pThis, pSource, pWarhead);

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
	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pTypeExt->NotHuman_RandomDeathSequence.Get())
		resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));

	if (receiveDamageArgs.WH)
	{
		auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH);
		const int whSequence = pWarheadExt->NotHuman_DeathSequence.Get();

		if (whSequence > 0)
			resultSequence = Math::min(Die(whSequence), Die(5));
	}

	R->ECX(pThis);
	pThis->PlayAnim(static_cast<Sequence>(resultSequence), true);

	return 0x518515;
}

DEFINE_HOOK(0x702050, TechnoClass_ReceiveDamage_AttachEffectExpireWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	std::set<AttachEffectTypeClass*> cumulativeTypes;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;

	for (auto const& attachEffect : pExt->AttachedEffects)
	{
		auto const pType = attachEffect->GetType();

		if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Death) != ExpireWeaponCondition::None)
		{
			if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || !cumulativeTypes.contains(pType))
			{
				if (pType->Cumulative && pType->ExpireWeapon_CumulativeOnlyOnce)
					cumulativeTypes.insert(pType);

				if (pType->ExpireWeapon_UseInvokerAsOwner)
				{
					if (auto const pInvoker = attachEffect->GetInvoker())
						expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pInvoker));
				}
				else
				{
					expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pThis));
				}
			}
		}
	}

	auto const coords = pThis->GetCoords();

	for (auto const& pair : expireWeapons)
	{
		auto const pInvoker = pair.second;
		WeaponTypeExt::DetonateAt(pair.first, coords, pInvoker, pInvoker->Owner, pThis);
	}

	return 0;
}

DEFINE_HOOK(0x701E18, TechnoClass_ReceiveDamage_ReflectDamage, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const int*, pDamage, EBX);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));
	GET_STACK(HouseClass*, pSourceHouse, STACK_OFFSET(0xC4, 0x1C));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (*pDamage <= 0)
		return 0;

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (pWHExt->Reflected)
		return 0;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto& random = ScenarioClass::Instance->Random;
	const auto& suppressType = pWHExt->SuppressReflectDamage_Types;
	const auto& suppressGroup = pWHExt->SuppressReflectDamage_Groups;
	const bool suppress = pWHExt->SuppressReflectDamage;
	const bool suppressByType = suppressType.size() > 0;
	const bool suppressByGroup = suppressGroup.size() > 0;

	if (pExt->AE.ReflectDamage && *pDamage > 0 && (!suppress || suppressByType || suppressByGroup))
	{
		for (auto const& attachEffect : pExt->AttachedEffects)
		{
			if (!attachEffect->IsActive())
				continue;

			auto const pType = attachEffect->GetType();

			if (!pType->ReflectDamage)
				continue;

			if (pType->ReflectDamage_Chance < random.RandomDouble())
				continue;

			if (suppress)
			{
				if (suppressByType && suppressType.Contains(pType))
					continue;

				if (suppressByGroup && pType->HasGroups(suppressGroup, false))
					continue;
			}

			auto const pWH = pType->ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
			int damage = pType->ReflectDamage_Override.Get(static_cast<int>(*pDamage * pType->ReflectDamage_Multiplier));

			if (pType->ReflectDamage_UseInvokerAsOwner)
			{
				auto const pInvoker = attachEffect->GetInvoker();

				if (pInvoker && EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pInvoker->Owner, pSourceHouse))
				{
					auto const pWHExtRef = WarheadTypeExt::ExtMap.Find(pWH);
					pWHExtRef->Reflected = true;

					if (pType->ReflectDamage_Warhead_Detonate)
						WarheadTypeExt::DetonateAt(pWH, pSource, pInvoker, damage, pInvoker->Owner);
					else
						pSource->ReceiveDamage(&damage, 0, pWH, pInvoker, false, false, pInvoker->Owner);

					pWHExtRef->Reflected = false;
				}
			}
			else if (EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pThis->Owner, pSourceHouse))
			{
				auto const pWHExtRef = WarheadTypeExt::ExtMap.Find(pWH);
				pWHExtRef->Reflected = true;

				if (pType->ReflectDamage_Warhead_Detonate)
					WarheadTypeExt::DetonateAt(pWH, pSource, pThis, damage, pThis->Owner);
				else
					pSource->ReceiveDamage(&damage, 0, pWH, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}
		}
	}

	return 0;
}

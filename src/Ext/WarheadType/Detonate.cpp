#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <BitFont.h>
#include <SuperClass.h>
#include <AircraftClass.h>

#include <Utilities/Helpers.Alex.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/EnumFunctions.h>
#include <Misc/AresData.h>

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletExt::ExtData* pBulletExt, CoordStruct coords)
{
	auto const pBullet = pBulletExt ? pBulletExt->OwnerObject() : nullptr;

	if (pOwner && pBulletExt)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		if (pTypeExt->Interceptor && pBulletExt->IsInterceptor)
			this->InterceptBullets(pOwner, pBullet->WeaponType, coords);
	}

	if (pHouse)
	{
		if (this->BigGap)
		{
			for (auto pOtherHouse : *HouseClass::Array)
			{
				if (pOtherHouse->IsControlledByHuman() &&   // Not AI
					!pOtherHouse->IsObserver() &&         // Not Observer
					!pOtherHouse->Defeated &&             // Not Defeated
					pOtherHouse != pHouse &&              // Not pThisHouse
					!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
				{
					pOtherHouse->ReshroudMap();
				}
			}
		}

		if (this->SpySat)
			MapClass::Instance->Reveal(pHouse);

		if (this->TransactMoney)
		{
			pHouse->TransactMoney(this->TransactMoney);

			if (this->TransactMoney_Display)
			{
				auto displayCoords = this->TransactMoney_Display_AtFirer ? (pOwner ? pOwner->Location : coords) : coords;
				FlyingStrings::AddMoneyString(this->TransactMoney, pHouse, this->TransactMoney_Display_Houses, displayCoords, this->TransactMoney_Display_Offset);
			}
		}

		for (const int swIdx : this->LaunchSW)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(swIdx))
			{
				const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
				const auto cell = CellClass::Coord2Cell(coords);
				if (!this->LaunchSW_RealLaunch || (pSuper->Granted && pSuper->IsCharged && !pSuper->IsOnHold && pHouse->CanTransactMoney(pSWExt->Money_Amount)))
				{
					if (this->LaunchSW_IgnoreInhibitors || !pSWExt->HasInhibitor(pHouse, cell)
					&& (this->LaunchSW_IgnoreDesignators || pSWExt->HasDesignator(pHouse, cell)))
					{
						int oldstart = pSuper->RechargeTimer.StartTime;
						int oldleft = pSuper->RechargeTimer.TimeLeft;
						// If you don't set it ready, NewSWType::Active will give false in Ares if RealLaunch=false
						// and therefore it will reuse the vanilla routine, which will crash inside of it
						pSuper->SetReadiness(true);
						// TODO: Can we use ClickFire instead of Launch?
						pSuper->Launch(cell, true);
						pSuper->Reset();
						if (!this->LaunchSW_RealLaunch)
						{
							pSuper->RechargeTimer.StartTime = oldstart;
							pSuper->RechargeTimer.TimeLeft = oldleft;
						}
					}
				}
			}
		}
	}

	this->HasCrit = false;
	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		this->Crit_Chance ||
		this->Shield_Break ||
		this->Shield_Respawn_Duration > 0 ||
		this->Shield_SelfHealing_Duration > 0 ||
		this->Shield_AttachTypes.size() > 0 ||
		this->Shield_RemoveTypes.size() > 0 ||
		this->Convert_To.size() > 0;

	bool bulletWasIntercepted = pBulletExt && pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted;

	const float cellSpread = this->OwnerObject()->CellSpread;
	if (cellSpread && isCellSpreadWarhead)
	{
		for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
			this->DetonateOnOneUnit(pHouse, pTarget, pOwner, bulletWasIntercepted);
	}
	else if (pBullet && isCellSpreadWarhead)
	{
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
			this->DetonateOnOneUnit(pHouse, pTarget, pOwner, bulletWasIntercepted);
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, bool bulletWasIntercepted)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health || pTarget->IsSinking)
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	this->ApplyShieldModifiers(pTarget);

	if (this->RemoveDisguise)
		this->ApplyRemoveDisguiseToInf(pHouse, pTarget);

	if (this->RemoveMindControl)
		this->ApplyRemoveMindControl(pHouse, pTarget);

	if (this->Crit_Chance && (!this->Crit_SuppressWhenIntercepted || !bulletWasIntercepted))
		this->ApplyCrit(pHouse, pTarget, pOwner);

	if (this->Convert_To.size() > 0)
		this->ApplyConvert(pHouse, pTarget);
}

void WarheadTypeExt::ExtData::ApplyShieldModifiers(TechnoClass* pTarget)
{
	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

		int shieldIndex = -1;
		double ratio = 1.0;

		// Remove shield.
		if (pExt->Shield && canAffectTarget)
		{
			const auto shieldType = pExt->Shield->GetType();
			shieldIndex = this->Shield_RemoveTypes.IndexOf(shieldType);

			if (shieldIndex >= 0)
			{
				ratio = pExt->Shield->GetHealthRatio();
				pExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
				pExt->Shield->KillAnim();
				pExt->Shield = nullptr;
			}
		}

		// Attach shield.
		if (canAffectTarget && Shield_AttachTypes.size() > 0)
		{
			ShieldTypeClass* shieldType = nullptr;

			if (this->Shield_ReplaceOnly)
			{
				if (shieldIndex >= 0)
					shieldType = Shield_AttachTypes[Math::min(shieldIndex, (signed)Shield_AttachTypes.size() - 1)];
			}
			else
			{
				shieldType = Shield_AttachTypes.size() > 0 ? Shield_AttachTypes[0] : nullptr;
			}

			if (shieldType)
			{
				if (shieldType->Strength && (!pExt->Shield || (this->Shield_ReplaceNonRespawning && pExt->Shield->IsBrokenAndNonRespawning() &&
					pExt->Shield->GetFramesSinceLastBroken() >= this->Shield_MinimumReplaceDelay)))
				{
					pExt->CurrentShieldType = shieldType;
					pExt->Shield = std::make_unique<ShieldClass>(pTarget, true);

					if (this->Shield_ReplaceOnly && this->Shield_InheritStateOnReplace)
					{
						pExt->Shield->SetHP((int)(shieldType->Strength * ratio));

						if (pExt->Shield->GetHP() == 0)
							pExt->Shield->SetRespawn(shieldType->Respawn_Rate, shieldType->Respawn, shieldType->Respawn_Rate, true);
					}
				}
			}
		}

		// Apply other modifiers.
		if (pExt->Shield)
		{
			if (this->Shield_AffectTypes.size() > 0 && !this->Shield_AffectTypes.Contains(pExt->Shield->GetType()))
				return;

			if (this->Shield_Break && pExt->Shield->IsActive())
				pExt->Shield->BreakShield(this->Shield_BreakAnim.Get(nullptr), this->Shield_BreakWeapon.Get(nullptr));

			if (this->Shield_Respawn_Duration > 0)
				pExt->Shield->SetRespawn(this->Shield_Respawn_Duration, this->Shield_Respawn_Amount, this->Shield_Respawn_Rate, this->Shield_Respawn_ResetTimer);

			if (this->Shield_SelfHealing_Duration > 0)
			{
				double amount = this->Shield_SelfHealing_Amount.Get(pExt->Shield->GetType()->SelfHealing);
				pExt->Shield->SetSelfHealing(this->Shield_SelfHealing_Duration, amount, this->Shield_SelfHealing_Rate, this->Shield_SelfHealing_ResetTimer);
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy)
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pInf = abstract_cast<InfantryClass*>(pTarget))
	{
		if (pInf->IsDisguised())
			pInf->Disguised = false;
	}
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	double dice;

	if (this->Crit_ApplyChancePerTarget)
		dice = ScenarioClass::Instance->Random.RandomDouble();
	else
		dice = this->RandomBuffer;

	if (this->Crit_Chance < dice)
		return;

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (pExt->TypeExtData->ImmuneToCrit)
			return;

		auto pSld = pExt->Shield.get();
		if (pSld && pSld->IsActive() && pSld->GetType()->ImmuneToCrit)
			return;

		if (pTarget->GetHealthPercentage() > this->Crit_AffectBelowPercent)
			return;
	}

	if (pHouse && !EnumFunctions::CanTargetHouse(this->Crit_AffectsHouses, pHouse, pTarget->Owner))
		return;

	if (!EnumFunctions::IsCellEligible(pTarget->GetCell(), this->Crit_Affects))
		return;

	if (!EnumFunctions::IsTechnoEligible(pTarget, this->Crit_Affects))
		return;

	this->HasCrit = true;

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		int idx = this->OwnerObject()->EMEffect || this->Crit_AnimList_PickRandom.Get(this->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_AnimList.size() - 1) : 0;

		GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location);
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (this->Crit_Warhead.isset())
		WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTarget, pOwner, damage);
	else
		pTarget->ReceiveDamage(&damage, 0, this->OwnerObject(), pOwner, false, false, pHouse);
}

void WarheadTypeExt::ExtData::InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords)
{
	if (!pOwner || !pWeapon)
		return;

	float cellSpread = this->OwnerObject()->CellSpread;

	if (cellSpread == 0.0)
	{
		if (auto const pBullet = specific_cast<BulletClass*>(pOwner->Target))
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			auto const pTypeExt = pExt->TypeExtData;

			// 1/8th of a cell as a margin of error.
			if (pTypeExt && pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0)
				pExt->InterceptBullet(pOwner, pWeapon);
		}
	}
	else
	{
		for (auto const& pBullet : *BulletClass::Array)
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			auto const pTypeExt = pExt->TypeExtData;

			// Cells don't know about bullets that may or may not be located on them so it has to be this way.
			if (pTypeExt && pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= cellSpread * Unsorted::LeptonsPerCell)
				pExt->InterceptBullet(pOwner, pWeapon);
		}
	}
}


// TODO: Unfinished placeholder, in case Ares 3.0+ isn't used -- Trsdy
// So who said it was merely a Type pointer replacement and he could make a better one than Ares?
bool DummyConvertToType(FootClass* pThis, TechnoTypeClass* pToType)
{
	AbstractType rtti;
	TechnoTypeClass** nowTypePtr;

	// Different types prohibited
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<InfantryClass*>(pThis)->Type));
		rtti = AbstractType::InfantryType;
		break;
	case AbstractType::Unit:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<UnitClass*>(pThis)->Type));
		rtti = AbstractType::UnitType;
		break;
	case AbstractType::Aircraft:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<AircraftClass*>(pThis)->Type));
		rtti = AbstractType::AircraftType;
		break;
	default:
		Debug::Log("%s is not a moveable type, conversion not allowed\n", pToType->get_ID());
		return false;
	}
	if (pToType->WhatAmI() != rtti)
	{
		Debug::Log("Incompatible types between %s and %s\n", pThis->get_ID(), pToType->get_ID());
		return false;
	}

	// Detach CLEG targeting
	auto tempUsing = pThis->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->Detach();

	HouseClass* const pOwner = pThis->Owner;

	// Remove tracking of old techno
	if (!pThis->InLimbo)
		pOwner->RegisterLoss(pThis, false);
	pOwner->RemoveTracking(pThis);

	int oldHealth = pThis->Health;

	// Generic type-conversion
	TechnoTypeClass* prevType = *nowTypePtr;
	*nowTypePtr = pToType;

	// Readjust health according to percentage
	pThis->SetHealthPercentage((double)(oldHealth) / (double)prevType->Strength);
	pThis->EstimatedHealth = pThis->Health;

	// Add tracking of new techno
	pOwner->AddTracking(pThis);
	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, false);
	pOwner->RecheckTechTree = true;

	// Update AttachEffects -- skipped
	// RecalculateStats -- skipped

	// Adjust ammo
	pThis->Ammo = Math::min(pThis->Ammo, pToType->Ammo);
	// ResetSpotlights -- skipped

	// Adjust ROT
	pThis->PrimaryFacing.SetROT(pToType->ROT);
	// Adjust TurretROT -- skipped


	// Locomotor change, referenced from Otamaa's comment, not sure if correct, untested
	CLSID nowLocoID;
	ILocomotion* iloco = pThis->Locomotor.get();
	const auto& toLoco = pToType->Locomotor;
	if ((SUCCEEDED(static_cast<LocomotionClass*>(iloco)->GetClassID(&nowLocoID)) && nowLocoID != toLoco))
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(pThis->Locomotor));
		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto NewLoco = LocomotionClass::CreateInstance(toLoco))
		{
			pThis->Locomotor.reset(NewLoco.release());
			pThis->Locomotor->Link_To_Object(pThis);
		}
	}

	// TODO : Jumpjet locomotor special treatement
	return true;
}

void WarheadTypeExt::ExtData::ApplyConvert(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pTargetFoot = abstract_cast<FootClass*>(pTarget))
	{
		auto Conversion = [this, pTargetFoot](TechnoTypeClass* pResultType)
		{
			if (AresData::CanUseAres)
				return AresData::ConvertTypeTo(pTargetFoot, pResultType);
			else
				return DummyConvertToType(pTargetFoot, pResultType);
		};

		if (this->Convert_To.size())
		{
			if (this->Convert_From.size())
			{
				// explicitly unsigned because the compiler wants it
				for (size_t i = 0; i < this->Convert_From.size(); i++)
				{
					// Check if the target matches upgrade-from TechnoType and it has something to upgrade-to
					if (this->Convert_To.size() >= i && this->Convert_From[i] == pTarget->GetTechnoType())
					{
						Conversion(this->Convert_To[i]);
						break;
					}
				}
			}
			else
			{
				Conversion(this->Convert_To[0]);
			}
		}
	}
}

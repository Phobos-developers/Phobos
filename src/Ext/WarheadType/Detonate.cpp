#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <BitFont.h>
#include <SuperClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/EnumFunctions.h>

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletExt::ExtData* pBulletExt, CoordStruct coords)
{
	auto const pBullet = pBulletExt ? pBulletExt->OwnerObject() : nullptr;

	if (pOwner && pBulletExt)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		if (pTypeExt->InterceptorType && pBulletExt->IsInterceptor)
			this->InterceptBullets(pOwner, pBullet->WeaponType, coords);
	}

	if (pHouse)
	{
		if (this->BigGap)
		{
			for (auto pOtherHouse : *HouseClass::Array)
			{
				if (pOtherHouse->IsControlledByHuman() && // Not AI
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

		if (this->SpawnsCrate_Types.size() > 0)
		{
			int index = GeneralUtils::ChooseOneWeighted(ScenarioClass::Instance->Random.RandomDouble(), &this->SpawnsCrate_Weights);

			if (index < static_cast<int>(this->SpawnsCrate_Types.size()))
				MapClass::Instance->PlacePowerupCrate(CellClass::Coord2Cell(coords), this->SpawnsCrate_Types.at(index));
		}

		for (const int swIdx : this->LaunchSW)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(swIdx))
			{
				const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
				const auto cell = CellClass::Coord2Cell(coords);

				if (pHouse->CanTransactMoney(pSWExt->Money_Amount) && (!this->LaunchSW_RealLaunch || (pSuper->IsPresent && pSuper->IsReady && !pSuper->IsSuspended)))
				{
					if ((this->LaunchSW_IgnoreInhibitors || !pSWExt->HasInhibitor(pHouse, cell))
					&& (this->LaunchSW_IgnoreDesignators || pSWExt->HasDesignator(pHouse, cell)))
					{
						if (this->LaunchSW_DisplayMoney && pSWExt->Money_Amount != 0)
							FlyingStrings::AddMoneyString(pSWExt->Money_Amount, pHouse, this->LaunchSW_DisplayMoney_Houses, coords, this->LaunchSW_DisplayMoney_Offset);

						int oldstart = pSuper->RechargeTimer.StartTime;
						int oldleft = pSuper->RechargeTimer.TimeLeft;
						// If you don't set it ready, NewSWType::Active will give false in Ares if RealLaunch=false
						// and therefore it will reuse the vanilla routine, which will crash inside of it
						pSuper->SetReadiness(true);
						// TODO: Can we use ClickFire instead of Launch?
						pSuper->Launch(cell, pHouse->IsCurrentPlayer());
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

	this->Crit_Active = false;
	this->Crit_CurrentChance = this->GetCritChance(pOwner);

	if (this->PossibleCellSpreadDetonate || this->Crit_CurrentChance > 0.0)
	{
		if (!this->Crit_ApplyChancePerTarget)
			this->Crit_RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

		if (this->Crit_ActiveChanceAnims.size() > 0 && this->Crit_CurrentChance > 0.0)
		{
			int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_ActiveChanceAnims.size() - 1);
			GameCreate<AnimClass>(this->Crit_ActiveChanceAnims[idx], coords);
		}

		bool bulletWasIntercepted = pBulletExt && pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted;
		const float cellSpread = this->OwnerObject()->CellSpread;

		if (cellSpread)
		{
			for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
				this->DetonateOnOneUnit(pHouse, pTarget, pOwner, bulletWasIntercepted);
		}
		else if (pBullet)
		{
			if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
			{
				// Starkku: We should only detonate on the target if the bullet, at the moment of detonation is within acceptable distance of the target.
				// Ares uses 64 leptons / quarter of a cell as a tolerance, so for sake of consistency we're gonna do the same here.
				if (pBullet->DistanceFrom(pTarget) < Unsorted::LeptonsPerCell / 4)
					this->DetonateOnOneUnit(pHouse, pTarget, pOwner, bulletWasIntercepted);
			}
		}
		else if (this->DamageAreaTarget)
		{
			if (coords.DistanceFrom(this->DamageAreaTarget->GetCoords()) < Unsorted::LeptonsPerCell / 4)
				this->DetonateOnOneUnit(pHouse, this->DamageAreaTarget, pOwner, bulletWasIntercepted);
		}
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, bool bulletWasIntercepted)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health || pTarget->IsSinking || pTarget->BeingWarpedOut)
		return;

	TechnoExt::ExtData* pTargetExt = nullptr;

	if (!this->CanTargetHouse(pHouse, pTarget) || !this->CanAffectTarget(pTarget, pTargetExt))
		return;

	this->ApplyShieldModifiers(pTarget, pTargetExt);

	if (this->RemoveDisguise)
		this->ApplyRemoveDisguise(pHouse, pTarget);

	if (this->RemoveMindControl)
		this->ApplyRemoveMindControl(pTarget);

	if (this->Crit_CurrentChance > 0.0 && (!this->Crit_SuppressWhenIntercepted || !bulletWasIntercepted))
		this->ApplyCrit(pHouse, pTarget, pOwner, pTargetExt);

	if (this->Convert_Pairs.size() > 0)
		this->ApplyConvert(pHouse, pTarget);

	if (this->AttachEffects.AttachTypes.size() > 0 || this->AttachEffects.RemoveTypes.size() > 0 || this->AttachEffects.RemoveGroups.size() > 0)
		this->ApplyAttachEffects(pTarget, pHouse, pOwner);

#ifdef LOCO_TEST_WARHEADS
	if (this->InflictLocomotor)
		this->ApplyLocomotorInfliction(pTarget);

	if (this->RemoveInflictedLocomotor)
		this->ApplyLocomotorInflictionReset(pTarget);
#endif

}

void WarheadTypeExt::ExtData::ApplyShieldModifiers(TechnoClass* pTarget, TechnoExt::ExtData* pTargetExt = nullptr)
{
	if (!pTargetExt)
		pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (pTargetExt)
	{
		int shieldIndex = -1;
		double ratio = 1.0;

		// Remove shield.
		if (pTargetExt->Shield)
		{
			const auto shieldType = pTargetExt->Shield->GetType();
			shieldIndex = this->Shield_RemoveTypes.IndexOf(shieldType);

			if (shieldIndex >= 0 || this->Shield_RemoveAll)
			{
				ratio = pTargetExt->Shield->GetHealthRatio();
				pTargetExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
				pTargetExt->Shield->KillAnim();
				pTargetExt->Shield = nullptr;
			}
		}

		// Attach shield.
		if (Shield_AttachTypes.size() > 0)
		{
			ShieldTypeClass* shieldType = nullptr;

			if (this->Shield_ReplaceOnly)
			{
				if (shieldIndex >= 0)
					shieldType = Shield_AttachTypes[Math::min(shieldIndex, (signed)Shield_AttachTypes.size() - 1)];
				else if (this->Shield_RemoveAll)
					shieldType = Shield_AttachTypes[0];
			}
			else
			{
				shieldType = Shield_AttachTypes[0];
			}

			if (shieldType)
			{
				if (shieldType->Strength && (!pTargetExt->Shield || (this->Shield_ReplaceNonRespawning && pTargetExt->Shield->IsBrokenAndNonRespawning() &&
					pTargetExt->Shield->GetFramesSinceLastBroken() >= this->Shield_MinimumReplaceDelay)))
				{
					pTargetExt->CurrentShieldType = shieldType;
					pTargetExt->Shield = std::make_unique<ShieldClass>(pTarget, true);

					if (this->Shield_ReplaceOnly && this->Shield_InheritStateOnReplace)
					{
						pTargetExt->Shield->SetHP((int)(shieldType->Strength * ratio));

						if (pTargetExt->Shield->GetHP() == 0)
							pTargetExt->Shield->SetRespawn(shieldType->Respawn_Rate, shieldType->Respawn, shieldType->Respawn_Rate, true);
					}
				}
			}
		}

		// Apply other modifiers.
		if (pTargetExt->Shield)
		{
			auto isShieldTypeEligible = [pTargetExt](Iterator<ShieldTypeClass*> pShieldTypeList) -> bool
				{
					return !(pShieldTypeList.size() > 0 && !pShieldTypeList.contains(pTargetExt->Shield->GetType()));
				};

			if (this->Shield_Break && pTargetExt->Shield->IsActive() && isShieldTypeEligible(this->Shield_Break_Types.GetElements(this->Shield_AffectTypes)))
				pTargetExt->Shield->BreakShield(this->Shield_BreakAnim, this->Shield_BreakWeapon);

			if (this->Shield_Respawn_Duration > 0 && isShieldTypeEligible(this->Shield_Respawn_Types.GetElements(this->Shield_AffectTypes)))
			{
				double amount = this->Shield_Respawn_Amount.Get(pTargetExt->Shield->GetType()->Respawn);
				pTargetExt->Shield->SetRespawn(this->Shield_Respawn_Duration, amount, this->Shield_Respawn_Rate, this->Shield_Respawn_RestartTimer);
			}

			if (this->Shield_SelfHealing_Duration > 0 && isShieldTypeEligible(this->Shield_SelfHealing_Types.GetElements(this->Shield_AffectTypes)))
			{
				double amount = this->Shield_SelfHealing_Amount.Get(pTargetExt->Shield->GetType()->SelfHealing);

				pTargetExt->Shield->SetSelfHealing(this->Shield_SelfHealing_Duration, amount, this->Shield_SelfHealing_Rate,
					this->Shield_SelfHealing_RestartInCombat.Get(pTargetExt->Shield->GetType()->SelfHealing_RestartInCombat),
					this->Shield_SelfHealing_RestartInCombatDelay, this->Shield_SelfHealing_RestartTimer);
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pTarget->IsDisguised())
	{
		if (auto pSpy = specific_cast<InfantryClass*>(pTarget))
			pSpy->Disguised = false;
		else if (auto pMirage = specific_cast<UnitClass*>(pTarget))
			pMirage->ClearDisguise();
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy)
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, TechnoExt::ExtData* pTargetExt = nullptr)
{
	double dice;

	if (this->Crit_ApplyChancePerTarget)
		dice = ScenarioClass::Instance->Random.RandomDouble();
	else
		dice = this->Crit_RandomBuffer;

	if (this->Crit_CurrentChance < dice)
		return;

	if (!pTargetExt)
		pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (pTargetExt)
	{
		auto const pTypeExt = pTargetExt->TypeExtData;

		if (pTypeExt->ImmuneToCrit)
			return;

		auto pSld = pTargetExt->Shield.get();
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

	this->Crit_Active = true;

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		if (!this->Crit_AnimList_CreateAll.Get(false))
		{
			int idx = this->OwnerObject()->EMEffect || this->Crit_AnimList_PickRandom.Get(false) ?
				ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_AnimList.size() - 1) : 0;

			auto const pAnim = GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location);
			pAnim->Owner = pHouse;
			AnimExt::ExtMap.Find(pAnim)->SetInvoker(pOwner, pHouse);
		}
		else
		{
			for (auto const& pType : this->Crit_AnimList)
			{
				auto const pAnim = GameCreate<AnimClass>(pType, pTarget->Location);
				pAnim->Owner = pHouse;
				AnimExt::ExtMap.Find(pAnim)->SetInvoker(pOwner, pHouse);
			}
		}
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (this->Crit_Warhead)
	{
		if (this->Crit_Warhead_FullDetonation)
			WarheadTypeExt::DetonateAt(this->Crit_Warhead, pTarget, pOwner, damage, pHouse);
		else
			this->DamageAreaWithTarget(pTarget->GetCoords(), damage, pOwner, this->Crit_Warhead, true, pHouse, pTarget);
	}
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

			// 1/8th of a cell as a margin of error if not Inviso interceptor.
			bool distanceCheck = pWeapon->Projectile->Inviso || pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0;

			if (pTypeExt && pTypeExt->Interceptable && distanceCheck)
				pExt->InterceptBullet(pOwner, pWeapon);
		}
	}
	else
	{
		for (auto const pBullet : *BulletClass::Array)
		{
			if (pBullet->Location.DistanceFrom(coords) > cellSpread * Unsorted::LeptonsPerCell)
				continue;

			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			auto const pBulletTypeExt = pBulletExt->TypeExtData;

			// Cells don't know about bullets that may or may not be located on them so it has to be this way.
			if (pBulletTypeExt && pBulletTypeExt->Interceptable)
				pBulletExt->InterceptBullet(pOwner, pWeapon);
		}
	}
}

void WarheadTypeExt::ExtData::ApplyConvert(HouseClass* pHouse, TechnoClass* pTarget)
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);

	if (!pTargetFoot || this->Convert_Pairs.size() == 0)
		return;

	TypeConvertGroup::Convert(pTargetFoot, this->Convert_Pairs, pHouse);
}

void WarheadTypeExt::ExtData::ApplyLocomotorInfliction(TechnoClass* pTarget)
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);
	if (!pTargetFoot)
		return;

	// same locomotor? no point to change
	CLSID targetCLSID { };
	CLSID inflictCLSID = this->OwnerObject()->Locomotor;
	IPersistPtr pLocoPersist = pTargetFoot->Locomotor;
	if (SUCCEEDED(pLocoPersist->GetClassID(&targetCLSID)) && targetCLSID == inflictCLSID)
		return;

	// prevent endless piggyback
	IPiggybackPtr pTargetPiggy = pTargetFoot->Locomotor;
	if (pTargetPiggy != nullptr && pTargetPiggy->Is_Piggybacking())
		return;

	LocomotionClass::ChangeLocomotorTo(pTargetFoot, inflictCLSID);
}

void WarheadTypeExt::ExtData::ApplyLocomotorInflictionReset(TechnoClass* pTarget)
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);

	if (!pTargetFoot)
		return;

	// remove only specific inflicted locomotor if specified
	CLSID removeCLSID = this->OwnerObject()->Locomotor;
	if (removeCLSID != CLSID())
	{
		CLSID targetCLSID { };
		IPersistPtr pLocoPersist = pTargetFoot->Locomotor;
		if (SUCCEEDED(pLocoPersist->GetClassID(&targetCLSID)) && targetCLSID != removeCLSID)
			return;
	}

	// // we don't want to remove non-ok-to-end locos
	// IPiggybackPtr pTargetPiggy = pTargetFoot->Locomotor;
	// if (pTargetPiggy != nullptr && (!pTargetPiggy->Is_Ok_To_End()))
	// 	return;

	LocomotionClass::End_Piggyback(pTargetFoot->Locomotor);
}

void WarheadTypeExt::ExtData::ApplyAttachEffects(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker)
{
	if (!pTarget)
		return;

	std::vector<int> dummy = std::vector<int>();
	auto const& info = this->AttachEffects;
	AttachEffectClass::Attach(pTarget, pInvokerHouse, pInvoker, this->OwnerObject(), info);
	AttachEffectClass::Detach(pTarget, info);
	AttachEffectClass::DetachByGroups(pTarget, info);
}

double WarheadTypeExt::ExtData::GetCritChance(TechnoClass* pFirer) const
{
	double critChance = this->Crit_Chance;

	if (!pFirer)
		return critChance;

	auto const pExt = TechnoExt::ExtMap.Find(pFirer);
	double extraChance = 0.0;

	for (auto& attachEffect : pExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (pType->Crit_Multiplier == 1.0 && pType->Crit_ExtraChance == 0.0)
			continue;

		if (pType->Crit_AllowWarheads.size() > 0 && !pType->Crit_AllowWarheads.Contains(this->OwnerObject()))
			continue;

		if (pType->Crit_DisallowWarheads.size() > 0 && pType->Crit_DisallowWarheads.Contains(this->OwnerObject()))
			continue;

		critChance = critChance * Math::max(pType->Crit_Multiplier, 0);
		extraChance += pType->Crit_ExtraChance;
	}

	return critChance + extraChance;
}

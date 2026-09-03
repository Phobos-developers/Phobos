#include "ShieldClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

std::vector<ShieldClass*> ShieldClass::Array;

ShieldClass::ShieldClass()
	: Techno { nullptr }
	, HP { 0 }
	, Timers { }
	, AreAnimsHidden { false }
{
	ShieldClass::Array.emplace_back(this);
}

ShieldClass::ShieldClass(TechnoClass* pTechno, bool isAttached)
	: Techno { pTechno }
	, IdleAnim { nullptr }
	, Timers { }
	, Cloak { false }
	, Online { true }
	, Temporal { false }
	, AreAnimsHidden { false }
	, Attached { isAttached }
	, SelfHealing_Rate_Warhead { -1 }
	, Respawn_Rate_Warhead { -1 }
	, IsSelfHealingEnabled { true }
{
	auto const pType = TechnoExt::Fetch(pTechno)->CurrentShieldType;
	this->Type = pType;
	this->SetHP(pType->InitialStrength.Get(pType->Strength));
	this->TechnoID = pTechno->GetTechnoType();

	if (pTechno->AbstractFlags & AbstractFlags::Foot)
		this->BracketDelta = this->TechnoID->PixelSelectionBracketDelta + pType->BracketDelta - 3;

	ShieldClass::Array.emplace_back(this);
}

ShieldClass::~ShieldClass()
{
	auto it = std::find(ShieldClass::Array.begin(), ShieldClass::Array.end(), this);

	if (it != ShieldClass::Array.end())
		ShieldClass::Array.erase(it);
}

void ShieldClass::PointerGotInvalid(void* ptr, bool removed)
{
	if (!removed) // TODO: might be risky, needs further investigation
		return;

	auto const abs = static_cast<AbstractClass*>(ptr);

	if (auto const pAnim = abstract_cast<AnimClass*, true>(abs))
	{
		auto const pAnimExt = AnimExt::TryFetch(pAnim);

		// the flag is only a fast-path gate: during scenario teardown the anim's
		// extension is already gone, and the references must still be dropped
		if (!pAnimExt || pAnimExt->IsShieldIdleAnim)
		{
			for (auto const pShield : ShieldClass::Array)
			{
				if (pAnim == pShield->IdleAnim)
				{
					pShield->IdleAnim = nullptr;
					break; // one anim must be used by less than one shield
				}
			}
		}
	}
}

// =============================
// load / save

template <typename T>
bool ShieldClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Techno)
		.Process(this->TechnoID)
		.Process(this->IdleAnim)
		.Process(this->Timers.SelfHealing_CombatRestart)
		.Process(this->Timers.SelfHealing)
		.Process(this->Timers.SelfHealing_WHModifier)
		.Process(this->Timers.Respawn_CombatRestart)
		.Process(this->Timers.Respawn)
		.Process(this->Timers.Respawn_WHModifier)
		.Process(this->HP)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Attached)
		.Process(this->AreAnimsHidden)
		.Process(this->Type)
		.Process(this->SelfHealing_Warhead)
		.Process(this->SelfHealing_Rate_Warhead)
		.Process(this->SelfHealing_RestartInCombat_Warhead)
		.Process(this->SelfHealing_RestartInCombatDelay_Warhead)
		.Process(this->Respawn_Warhead)
		.Process(this->Respawn_Rate_Warhead)
		.Process(this->Respawn_RestartInCombat_Warhead)
		.Process(this->Respawn_RestartInCombatDelay_Warhead)
		.Process(this->Respawn_Anim_Warhead)
		.Process(this->Respawn_Weapon_Warhead)
		.Process(this->LastBreakFrame)
		.Process(this->LastTechnoHealthRatio)
		.Process(this->IsSelfHealingEnabled)
		.Success();
}

bool ShieldClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool ShieldClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<ShieldClass*>(this)->Serialize(Stm);
}

// =============================
//
// Is used for DeploysInto/UndeploysInto
void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::Fetch(pFrom);
	const auto pToExt = TechnoExt::Fetch(pTo);

	if (pFromExt->Shield)
	{
		pToExt->CurrentShieldType = pFromExt->CurrentShieldType;
		pToExt->Shield = std::make_unique<ShieldClass>(pTo);
		pToExt->Shield->HP = pFromExt->Shield->HP;

		// handle shield conversion and tint
		pToExt->Shield->ConvertCheck(pToExt->TypeExtData->OwnerObject());

		if (pToExt->Shield)
			pToExt->Shield->UpdateTint();

		if (pFrom->WhatAmI() == AbstractType::Building)
			pFromExt->Shield = nullptr;
	}
}

bool ShieldClass::ShieldIsBrokenTEvent(ObjectClass* pAttached)
{
	if (auto const pTechno = abstract_cast<TechnoClass*>(pAttached))
	{
		auto const pShield = TechnoExt::Fetch(pTechno)->Shield.get();
		return !pShield || pShield->HP <= 0;
	}

	return false;
}

int ShieldClass::ReceiveDamage(args_ReceiveDamage* args)
{
	int& damage = *args->Damage;
	int& health = this->HP;

	if (!health || this->Temporal || damage == 0)
		return damage;

	auto const pTechno = this->Techno;

	// Handle a special case where parasite damages shield but not the unit and unit itself cannot be targeted by repair weapons.
	if (damage < 0)
	{
		if (auto const pFoot = abstract_cast<FootClass*, true>(pTechno))
		{
			if (auto const pParasite = pFoot->ParasiteEatingMe)
			{
				// Remove parasite.
				pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
				pParasite->ParasiteImUsing->ExitUnit();
			}
		}
	}

	auto const pWH = args->WH;
	auto const pWHExt = WarheadTypeExt::Fetch(pWH);
	const bool IC = pWHExt->CanAffectInvulnerable(pTechno);

	if (!IC || this->CanBePenetrated(pWH))
		return damage;

	auto const pAttacker = args->Attacker;
	auto const pTechnoType = this->TechnoID;

	if (pTechnoType->Immune || TechnoExt::IsTypeImmune(pTechno, pTechnoType, pAttacker))
		return damage;

	int nDamage = 0;
	int shieldDamage = 0;
	int healthDamage = 0;
	double armorMultiplier = 1.0;
	auto const pType = this->Type;
	auto const pSourceHouse = pAttacker ? pAttacker->Owner : args->SourceHouse;

	if (pWHExt->CanTargetHouse(pSourceHouse, pTechno) && !pWH->Temporal)
	{
		if (damage >= 0)
		{
			nDamage = damage;

			if (pType->ApplyArmorMult.Get(RulesExt::Global()->ShieldApplyArmorMult))
			{
				armorMultiplier = TechnoExt::GetCurrentArmorMultiplier(pTechno, pTechnoType, pSourceHouse, pWH);
				nDamage = Math::max(static_cast<int>(nDamage / armorMultiplier), 0);
			}

			nDamage = MapClass::GetTotalDamage(nDamage, pWH, this->GetArmorType(pTechnoType), args->DistanceToEpicenter);
		}
		else
		{
			nDamage = -MapClass::GetTotalDamage(-damage, pWH, this->GetArmorType(pTechnoType), args->DistanceToEpicenter);
		}

		const bool affectsShield = pWHExt->Shield_AffectTypes.size() <= 0 || pWHExt->Shield_AffectTypes.Contains(pType);
		const double absorbPercent = affectsShield ? pWHExt->Shield_AbsorbPercent.Get(pType->AbsorbPercent) : pType->AbsorbPercent;
		const double passPercent = affectsShield ? pWHExt->Shield_PassPercent.Get(pType->PassPercent) : pType->PassPercent;

		shieldDamage = (int)((double)nDamage * absorbPercent);
		// passthrough damage shouldn't be affected by shield armor
		healthDamage = (int)((double)damage * passPercent);
	}

	const int originalShieldDamage = shieldDamage;
	const int min = pWHExt->Shield_ReceivedDamage_Minimum.Get(pType->ReceivedDamage_Minimum);
	const int max = pWHExt->Shield_ReceivedDamage_Maximum.Get(pType->ReceivedDamage_Maximum);
	const int minDmg = GeneralUtils::SafeMultiply(min, pWHExt->Shield_ReceivedDamage_MinMultiplier);
	const int maxDmg = GeneralUtils::SafeMultiply(max, pWHExt->Shield_ReceivedDamage_MaxMultiplier);
	shieldDamage = Math::clamp(shieldDamage, minDmg, maxDmg);

	if (Phobos::DisplayDamageNumbers && shieldDamage != 0)
		GeneralUtils::DisplayDamageNumberString(shieldDamage, DamageDisplayType::Shield, pTechno->GetRenderCoords(), TechnoExt::Fetch(pTechno)->DamageNumberOffset);

	if (shieldDamage > 0)
	{
		const bool whModifiersApplied = this->Timers.SelfHealing_WHModifier.InProgress();
		const bool restart = whModifiersApplied ? this->SelfHealing_RestartInCombat_Warhead : pType->SelfHealing_RestartInCombat;

		if (restart)
		{
			const int delay = whModifiersApplied ? this->SelfHealing_RestartInCombatDelay_Warhead : pType->SelfHealing_RestartInCombatDelay;

			if (delay > 0)
			{
				this->Timers.SelfHealing_CombatRestart.Start(delay);
				this->Timers.SelfHealing.Stop();
			}
			else
			{
				const int rate = whModifiersApplied ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
				this->Timers.SelfHealing.Start(rate); // when attacked, restart the timer
			}
		}

		if (!pWHExt->Nonprovocative)
			this->ResponseAttack();

		if (pWHExt->DecloakDamagedTargets.Get(RulesExt::Global()->DecloakDamagedTargets))
			pTechno->Uncloak(false);

		const int residueDamage = shieldDamage - health;

		if (residueDamage >= 0)
		{
			if (pType->AbsorbOverDamage)
			{
				this->BreakShield(pWHExt->Shield_BreakAnim, pWHExt->Shield_BreakWeapon.Get(nullptr));
				return healthDamage;
			}

			const int actualResidueDamage = Math::max(0, int((double)(originalShieldDamage - health) * armorMultiplier /
				GeneralUtils::GetWarheadVersusArmor(pWH, this->GetArmorType(pTechnoType)))); //only absord percentage damage

			this->BreakShield(pWHExt->Shield_BreakAnim, pWHExt->Shield_BreakWeapon.Get(nullptr));
			return actualResidueDamage + healthDamage;
		}
		else
		{
			if (pType->HitFlash && pWHExt->Shield_HitFlash)
			{
				const int size = pType->HitFlash_FixedSize.Get((shieldDamage * 2));
				SpotlightFlags flags = SpotlightFlags::NoColor;

				if (pType->HitFlash_Black)
				{
					flags = SpotlightFlags::NoColor;
				}
				else
				{
					if (!pType->HitFlash_Red)
						flags = SpotlightFlags::NoRed;
					if (!pType->HitFlash_Green)
						flags |= SpotlightFlags::NoGreen;
					if (!pType->HitFlash_Blue)
						flags |= SpotlightFlags::NoBlue;
				}

				MapClass::FlashbangWarheadAt(size, pWH, pTechno->Location, true, flags);
			}

			if (!pWHExt->Shield_SkipHitAnim)
				this->WeaponNullifyAnim(pWHExt->Shield_HitAnim);

			health = -residueDamage;

			this->UpdateIdleAnim(pType);

			return healthDamage;
		}
	}
	else if (shieldDamage < 0)
	{
		const int nLostHP = pType->Strength - health;

		if (!nLostHP)
		{
			int result = damage;

			if (result * GeneralUtils::GetWarheadVersusArmor(pWH, pTechnoType->Armor) > 0)
				result = 0;

			return result;
		}

		const int nRemainLostHP = nLostHP + shieldDamage;

		if (nRemainLostHP < 0)
			health = pType->Strength;
		else
			health -= shieldDamage;

		this->UpdateIdleAnim(pType);

		return 0;
	}

	// else if (nDamage == 0)
	return healthDamage;
}

void ShieldClass::ResponseAttack()
{
	const auto pTechno = this->Techno;

	if (pTechno->Owner != HouseClass::CurrentPlayer)
		return;

	if (const auto pBld = abstract_cast<BuildingClass*, true>(pTechno))
	{
		pTechno->Owner->BuildingUnderAttack(pBld);
	}
	else if (const auto pUnit = abstract_cast<UnitClass*, true>(pTechno))
	{
		if (pUnit->Type->Harvester)
		{
			const auto pos = pUnit->GetDestination(pUnit);
			if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(pos)))
				VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack);
		}
	}
}

void ShieldClass::WeaponNullifyAnim(const std::vector<AnimTypeClass*>& pHitAnim)
{
	if (this->AreAnimsHidden)
		return;

	const auto pTechno = this->Techno;
	AnimExt::CreateRandomAnim((pHitAnim.empty() ? this->Type->HitAnim : pHitAnim), pTechno->GetCoords(), pTechno, nullptr, true, true);
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon) const
{
	if (!pWeapon)
		return false;

	if (this->CanBePenetrated(pWeapon->Warhead) || !this->HP)
		return true;

	return GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, this->GetArmorType()) != 0.0;
}

bool ShieldClass::CanBePenetrated(WarheadTypeClass* pWarhead) const
{
	if (!pWarhead)
		return false;

	const auto pWHExt = WarheadTypeExt::Fetch(pWarhead);

	const auto affectedTypes = pWHExt->Shield_Penetrate_Types.GetElements(pWHExt->Shield_AffectTypes);

	if (affectedTypes.size() > 0 && !affectedTypes.contains(this->Type))
		return false;

	if (pWarhead->Psychedelic)
		return !this->Type->ImmuneToBerserk;

	return pWHExt->Shield_Penetrate;
}

void ShieldClass::AI_Temporal()
{
	if (!this->Temporal)
	{
		this->Temporal = true;

		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		timer->Pause();

		this->CloakCheck();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
}

void ShieldClass::AI()
{
	auto const pTechno = this->Techno;

	if (pTechno->InLimbo || pTechno->IsImmobilized)
		return;

	auto const pTechnoExt = TechnoExt::Fetch(pTechno);

	if (pTechno->Health <= 0 || !pTechno->IsAlive || pTechno->IsSinking)
	{
		pTechnoExt->Shield = nullptr;
		return;
	}

	this->CloakCheck();
	this->TemporalCheck();
	this->OnlineCheck();
	this->EnabledByCheck();

	if (this->IsSelfHealingEnabled)
	{
		this->RespawnShield();
		this->SelfHealing();
	}

	auto const pType = this->Type;

	// there're 2 cases to trigger the idle anim update: the techno's own/shield hp change
	// the former one will only take effect when IdleAnimDamaged is set
	// the latter one has already been covered separatedly, so no need to do it here
	if (pType->IdleAnimDamaged.isset())
	{
		const double ratio = pTechno->GetHealthPercentage();

		if (GeneralUtils::HasHealthRatioThresholdChanged(this->LastTechnoHealthRatio, ratio))
			this->UpdateIdleAnim(pType, ratio);

		this->LastTechnoHealthRatio = ratio;
	}

	if (!this->IdleAnim && this->Online && this->HP > 0 && !this->AreAnimsHidden)
		this->CreateAnim(pType);

	if (this->Timers.Respawn_WHModifier.Completed())
		this->Timers.Respawn_WHModifier.Stop();

	if (this->Timers.SelfHealing_WHModifier.Completed())
		this->Timers.SelfHealing_WHModifier.Stop();
}

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->Cloak = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->Cloak && this->IdleAnim && AnimTypeExt::Fetch(this->IdleAnim->Type)->DetachOnCloak)
		this->KillAnim();
}

void ShieldClass::EnabledByCheck()
{
	auto const& enabledBy = this->Type->SelfHealing_EnabledBy;

	if (enabledBy.empty())
		return;

	this->IsSelfHealingEnabled = false;

	for (auto const pBuilding : this->Techno->Owner->Buildings)
	{
		const bool isActive = !(pBuilding->Deactivated || pBuilding->IsUnderEMP()) && pBuilding->IsPowerOnline();

		if (enabledBy.Contains(pBuilding->Type) && isActive)
		{
			this->IsSelfHealingEnabled = true;
			break;
		}
	}

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	if (!this->IsSelfHealingEnabled)
		timer->Pause();
	else
		timer->Resume();
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	const auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && pTechno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(pTechno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		if (this->Online)
		{
			this->Online = false;
			this->UpdateTint();
		}

		timer->Pause();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_OfflineAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
	else
	{
		if (!this->Online)
		{
			this->Online = true;
			this->UpdateTint();
		}

		timer->Resume();

		if (this->IdleAnim)
		{
			this->IdleAnim->UnderTemporal = false;
			this->IdleAnim->Unpause();
		}
	}
}

void ShieldClass::TemporalCheck()
{
	if (!this->Temporal)
		return;

	this->Temporal = false;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
	timer->Resume();

	if (this->IdleAnim)
	{
		this->IdleAnim->UnderTemporal = false;
		this->IdleAnim->Unpause();
	}
}

// Is used for DeploysInto/UndeploysInto and Type conversion
void ShieldClass::ConvertCheck(TechnoTypeClass* pTechnoType)
{
	const auto pTechnoExt = TechnoExt::Fetch(this->Techno);
	const auto pOldType = this->Type;

	if (!pOldType->AllowTransfer.Get(Attached))
	{
		const auto pTechnoTypeExt = TechnoTypeExt::Fetch(pTechnoType);
		pTechnoExt->CurrentShieldType = pTechnoTypeExt->ShieldType && pTechnoTypeExt->ShieldType->Strength > 0 ? pTechnoTypeExt->ShieldType : nullptr;

		if (!pTechnoExt->CurrentShieldType)
		{
			// Case 1: Old shield is not allowed to transfer or there's no eligible new shield type -> delete shield.
			this->KillAnim();
			pTechnoExt->Shield = nullptr;
			return;
		}
		else
		{
			// Case 2: Old shield is not allowed to transfer and the new type is eligible for activation -> use the new shield type.
			this->Type = pTechnoTypeExt->ShieldType;
		}
	}

	// Our new type is either the old shield or the changed type from the above two scenarios.
	const auto pNewType = pTechnoExt->CurrentShieldType;
	
	// Calculation that's irrelevant to old shield type
	this->TechnoID = pTechnoType;
	this->BracketDelta = pTechnoType->PixelSelectionBracketDelta + pNewType->BracketDelta - 3;

	if (pNewType == pOldType)
		return;

	// Calculation that's related to old shield type
	const bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
	const double healthRatio = this->GetHealthRatio();

	if (pOldType->GetIdleAnimType(isDamaged, healthRatio) != pNewType->GetIdleAnimType(isDamaged, healthRatio))
		this->KillAnim();

	const bool respawn = this->HP <= 0;

	if (!respawn)
	{
		this->HP = (int)round(
			(double)this->HP /
			pOldType->Strength *
			pNewType->Strength
		);
	}

	// Update respawn and self heal
	const auto timerWHModifier = respawn ? &this->Timers.Respawn_WHModifier : &this->Timers.SelfHealing_WHModifier;

	// Reuse warhead modifier if active
	if (!timerWHModifier->InProgress())
	{
		const auto timer = respawn ? &this->Timers.Respawn : &this->Timers.SelfHealing;

		// Bail out if can't respawn or self heal
		if (respawn ? pNewType->Respawn == 0.0 : pNewType->SelfHealing == 0.0)
		{
			timer->Stop();
			return;
		}

		const int newRate = respawn ? pNewType->Respawn_Rate : pNewType->SelfHealing_Rate;

		if (respawn ? pOldType->Respawn : pOldType->SelfHealing)
		{
			const int oldRate = respawn ? pOldType->Respawn_Rate : pOldType->SelfHealing_Rate;

			// Recalculate timer based on both old and new shield types
			if (oldRate > 0)
				timer->TimeLeft = static_cast<int>((double)timer->GetTimeLeft() * newRate / oldRate);
		}
		// Handle the case where old shield type doesn't have respawn or self heal
		else
		{
			// Restart the timer if it's not in combat status
			const auto timerCombatRestart = respawn ? &this->Timers.Respawn_CombatRestart : &this->Timers.SelfHealing_CombatRestart;

			if (!timerCombatRestart->InProgress())
				timer->Start(newRate);
		}
	}
}

void ShieldClass::SelfHealing()
{
	const auto timerCombatRestart = &this->Timers.SelfHealing_CombatRestart;

	if (timerCombatRestart->InProgress())
		return;

	const auto pType = this->Type;
	const auto timerWHModifier = &this->Timers.SelfHealing_WHModifier;
	const bool hasModifier = timerWHModifier->InProgress();
	const auto timer = &this->Timers.SelfHealing;

	if (timerCombatRestart->Completed())
	{
		const int rate = hasModifier ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
		timer->Start(rate);
		timerCombatRestart->Stop();
	}

	if (timerWHModifier->Completed() && timer->InProgress())
	{
		const double mult = this->SelfHealing_Rate_Warhead > 0 ? (double)pType->SelfHealing_Rate / this->SelfHealing_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}

	const double amount = hasModifier ? this->SelfHealing_Warhead : pType->SelfHealing;
	const int percentageAmount = this->GetPercentageAmount(amount);

	if (percentageAmount != 0)
	{
		const int rate = hasModifier ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
		auto& health = this->HP;

		if ((health < pType->Strength || percentageAmount < 0) && timer->StartTime == -1)
			timer->Start(rate);

		if (health > 0 && timer->Completed())
		{
			timer->Start(rate);
			health += percentageAmount;

			this->UpdateIdleAnim(pType);

			if (health > pType->Strength)
			{
				health = pType->Strength;
				timer->Stop();
			}
			else if (health <= 0)
			{
				std::vector<AnimTypeClass*> nothing;
				this->BreakShield(nothing);
			}
		}
	}
}

void ShieldClass::BreakShield(const std::vector<AnimTypeClass*>& pBreakAnim, WeaponTypeClass* pBreakWeapon)
{
	this->HP = 0;
	auto const pType = this->Type;
	auto const pTechno = this->Techno;
	const bool hasModifier = this->Timers.Respawn_WHModifier.InProgress();

	if (hasModifier ? this->Respawn_Warhead : pType->Respawn)
		this->Timers.Respawn.Start(hasModifier ? this->Respawn_Rate_Warhead : pType->Respawn_Rate);

	this->Timers.SelfHealing.Stop();
	this->KillAnim();

	if (!this->AreAnimsHidden)
		AnimExt::CreateRandomAnim(pBreakAnim.empty() ? pType->BreakAnim : pBreakAnim, pTechno->Location, pTechno, nullptr, true, true);

	const auto pWeaponType = pBreakWeapon ? pBreakWeapon : pType->BreakWeapon;
	this->LastBreakFrame = Unsorted::CurrentFrame;
	this->UpdateTint();

	if (pWeaponType)
		TechnoExt::FireWeaponAtSelf(pTechno, pWeaponType);
}

void ShieldClass::RespawnShield()
{
	const auto timerCombatRestart = &this->Timers.Respawn_CombatRestart;

	if (timerCombatRestart->InProgress())
		return;

	const auto pType = this->Type;
	const auto timerWHModifier = &this->Timers.Respawn_WHModifier;
	const bool hasModifier = timerWHModifier->InProgress();
	const auto timer = &this->Timers.Respawn;

	if (timerCombatRestart->Completed())
	{
		const int rate = hasModifier ? this->Respawn_Rate_Warhead : pType->Respawn_Rate;
		timer->Start(rate);
		timerCombatRestart->Stop();
	}

	if (this->HP <= 0 && timer->Completed())
	{
		timer->Stop();
		const double amount = hasModifier ? this->Respawn_Warhead : this->Type->Respawn;
		this->HP = this->GetPercentageAmount(amount);
		this->UpdateTint();
		const auto pAnimList = hasModifier ? this->Respawn_Anim_Warhead : pType->Respawn_Anim;
		const auto pWeapon = hasModifier ? this->Respawn_Weapon_Warhead : pType->Respawn_Weapon;
		const auto pTechno = this->Techno;

		AnimExt::CreateRandomAnim(pAnimList, pTechno->Location, pTechno, pTechno->Owner, true, true);

		if (pWeapon)
			TechnoExt::FireWeaponAtSelf(pTechno, pWeapon);
	}
	else if (timerWHModifier->Completed() && timer->InProgress())
	{
		const double mult = this->Respawn_Rate_Warhead > 0 ? (double)pType->Respawn_Rate / this->Respawn_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetRespawn(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer, std::vector<AnimTypeClass*> anim, WeaponTypeClass* weapon)
{
	const auto timerWHModifier = &this->Timers.Respawn_WHModifier;
	const auto timerCombatRestart = &this->Timers.Respawn_CombatRestart;
	const auto pType = this->Type;
	const int oldRate = this->Respawn_Rate_Warhead;

	if (duration > 0)
	{
		this->Respawn_Warhead = amount;
		this->Respawn_Rate_Warhead = rate >= 0 ? rate : pType->Respawn_Rate;
		this->Respawn_RestartInCombat_Warhead = restartInCombat;
		this->Respawn_RestartInCombatDelay_Warhead = restartInCombatDelay >= 0 ? restartInCombatDelay : pType->Respawn_RestartInCombatDelay;
		this->Respawn_Anim_Warhead = anim.size() > 0 ? anim : pType->Respawn_Anim;
		this->Respawn_Weapon_Warhead = weapon ? weapon : pType->Respawn_Weapon;
		timerWHModifier->Start(duration);
	}

	if (this->HP > 0)
		return;

	const auto timer = &this->Timers.Respawn;

	if (resetTimer)
	{
		if (!timerCombatRestart->InProgress())
			timer->Start(this->Respawn_Rate_Warhead >= 0 ? this->Respawn_Rate_Warhead : pType->Respawn_Rate);
	}
	else if (duration > 0 && this->Respawn_Rate_Warhead != oldRate && timer->HasTimeLeft())
	{
		const double mult = oldRate > 0 ? (double)this->Respawn_Rate_Warhead / oldRate : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetRespawnRestartInCombat()
{
	if (this->Timers.Respawn.HasStarted())
	{
		const auto pType = this->Type;
		const bool whModifiersApplied = this->Timers.Respawn_WHModifier.InProgress();
		const bool restart = whModifiersApplied ? this->Respawn_RestartInCombat_Warhead : pType->Respawn_RestartInCombat;

		if (restart)
		{
			const int delay = whModifiersApplied ? this->Respawn_RestartInCombatDelay_Warhead : pType->Respawn_RestartInCombatDelay;

			if (delay > 0)
			{
				this->Timers.Respawn_CombatRestart.Start(delay);
				this->Timers.Respawn.Stop();
			}
			else
			{
				const int rate = whModifiersApplied ? this->Respawn_Rate_Warhead : pType->Respawn_Rate;
				this->Timers.Respawn.Start(rate); // when attacked, restart the timer
			}
		}
	}
}

void ShieldClass::SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer)
{
	const auto pType = this->Type;
	const auto timer = &this->Timers.SelfHealing;
	const auto timerWHModifier = &this->Timers.SelfHealing_WHModifier;
	const auto timerCombatRestart = &this->Timers.SelfHealing_CombatRestart;
	const int oldRate = this->SelfHealing_Rate_Warhead;

	if (duration > 0)
	{
		this->SelfHealing_Warhead = amount;
		this->SelfHealing_Rate_Warhead = rate >= 0 ? rate : pType->SelfHealing_Rate;
		this->SelfHealing_RestartInCombat_Warhead = restartInCombat;
		this->SelfHealing_RestartInCombatDelay_Warhead = restartInCombatDelay >= 0 ? restartInCombatDelay : pType->SelfHealing_RestartInCombatDelay;
		timerWHModifier->Start(duration);
	}

	if (resetTimer)
	{
		if (!timerCombatRestart->InProgress())
			timer->Start(this->SelfHealing_Rate_Warhead >= 0 ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate);
	}
	else if (duration > 0 && this->SelfHealing_Rate_Warhead != oldRate && timer->HasStarted())
	{
		const double mult = oldRate > 0 ? (double)this->SelfHealing_Rate_Warhead / oldRate : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::CreateAnim(ShieldTypeClass* pType, AnimTypeClass* idleAnimType)
{
	if (!idleAnimType)
	{
		const bool idleAnimSet = pType->IdleAnim.isDamagedValueSet();
		const bool idleAnimDamagedSet = pType->IdleAnimDamaged.isset();

		if (idleAnimSet || idleAnimDamagedSet)
			idleAnimType = this->GetIdleAnimType(pType, idleAnimSet, idleAnimDamagedSet);
		else if (pType->IdleAnim.isset())
			idleAnimType = pType->IdleAnim.BaseValue;
		else
			return;
	}

	if (idleAnimType)
	{
		if (this->Cloak && AnimTypeExt::Fetch(idleAnimType)->DetachOnCloak)
			return;

		auto const pTechno = this->Techno;
		auto const pAnim = GameCreate<AnimClass>(idleAnimType, pTechno->Location);

		pAnim->SetOwnerObject(pTechno);
		pAnim->Owner = pTechno->Owner;

		auto const pAnimExt = AnimExt::Fetch(pAnim);
		pAnimExt->SetInvoker(pTechno);
		pAnimExt->IsShieldIdleAnim = true;

		pAnim->RemainingIterations = 0xFFu;
		this->IdleAnim = pAnim;
	}
}

void ShieldClass::UpdateIdleAnim(ShieldTypeClass* pType, double ratio)
{
	if (this->AreAnimsHidden)
		return;

	const bool idleAnimSet = pType->IdleAnim.isDamagedValueSet();
	const bool idleAnimDamagedSet = pType->IdleAnimDamaged.isset();

	if (!idleAnimSet && !idleAnimDamagedSet)
		return;

	if (auto const pAnim = this->IdleAnim)
	{
		auto const pAnimType = this->GetIdleAnimType(pType, idleAnimSet, idleAnimDamagedSet, ratio);

		if (pAnim->Type != pAnimType)
		{
			this->KillAnim();
			this->CreateAnim(pType, pAnimType);
		}
	}
}

void ShieldClass::UpdateTint()
{
	if (this->Type->HasTint())
	{
		auto const pTechno = this->Techno;
		TechnoExt::Fetch(pTechno)->UpdateTintValues();
		pTechno->MarkForRedraw();
	}
}

AnimTypeClass* ShieldClass::GetIdleAnimType(ShieldTypeClass* pType, bool idleAnimSet, bool idleAnimDamagedSet, double ratio)
{
	bool isDamaged = false;

	if (idleAnimDamagedSet)
	{
		if (ratio == 0.0)
			ratio = this->Techno->GetHealthPercentage();

		isDamaged = ratio <= RulesClass::Instance->ConditionYellow;
	}

	if (!idleAnimSet && !pType->IdleAnimDamaged.isDamagedValueSet())
		return isDamaged ? pType->IdleAnimDamaged.BaseValue : pType->IdleAnim.BaseValue;

	return pType->GetIdleAnimType(isDamaged, this->GetHealthRatio());
}

void ShieldClass::DrawShieldBar_Building(const int length, RectangleStruct* pBound)
{
	if (this->HP <= 0 && this->Type->Pips_HideIfNoStrength)
		return;

	Point2D selectBracketPosition = TechnoExt::GetBuildingSelectBracketPosition(this->Techno, this->TechnoID, BuildingSelectBracketPosition::Top);
	selectBracketPosition.X -= 6;
	selectBracketPosition.Y -= 3;
	const int totalLength = DrawShieldBar_PipAmount(length);
	const int frame = this->DrawShieldBar_Pip(true);

	if (totalLength > 0)
	{
		for (int frameIdx = totalLength, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			Point2D position = selectBracketPosition;
			position.X -= deltaX;
			position.Y -= deltaY;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				frame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (totalLength < length)
	{
		const int emptyFrame = this->Type->Pips_Building_Empty.Get(RulesExt::Global()->Pips_Shield_Building_Empty.Get(0));

		for (int frameIdx = length - totalLength, deltaX = 4 * totalLength, deltaY = -2 * totalLength;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			Point2D position = selectBracketPosition;
			position.X -= deltaX;
			position.Y -= deltaY;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				emptyFrame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(const int length, RectangleStruct* pBound, bool isInfantry)
{
	const auto pType = this->Type;

	if (this->HP <= 0 && pType->Pips_HideIfNoStrength)
		return;

	auto position = TechnoExt::GetFootSelectBracketPosition(this->Techno, Anchor(HorizontalPosition::Left, VerticalPosition::Top), isInfantry);
	const auto pipBoard = pType->Pips_Background.Get(RulesExt::Global()->Pips_Shield_Background.Get(FileSystem::PIPBRD_SHP));
	int frame = pipBoard->Frames > 2 ? 2 : 0;

	position.X -= 1;
	position.Y += this->BracketDelta;

	if (this->Techno->IsSelected)
	{
		int offset = length + 1;

		if (length == 8)
		{
			frame += 1;
			offset += length + 1;
		}

		position.X += offset;
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pipBoard,
			frame, &position, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		position.X -= offset;
	}

	frame = this->DrawShieldBar_Pip(false);
	position.Y += 1;

	const int totalLength = DrawShieldBar_PipAmount(length);
	for (int i = 0; i < totalLength; ++i)
	{
		position.X += 2;
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			frame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

int ShieldClass::DrawShieldBar_Pip(const bool isBuilding) const
{
	const auto pType = this->Type;
	const int strength = pType->Strength.Get();
	const auto pipsShield = isBuilding ? pType->Pips_Building.Get() : pType->Pips.Get();

	const auto shieldPip = pipsShield.X != -1
		? pipsShield
		: (isBuilding
			? RulesExt::Global()->Pips_Shield_Building.Get()
			: RulesExt::Global()->Pips_Shield.Get());

	if (this->HP > pType->GetConditionYellow() * strength && shieldPip.X != -1)
		return shieldPip.X;
	else if (this->HP > pType->GetConditionRed() * strength && (shieldPip.Y != -1 || shieldPip.X != -1))
		return shieldPip.Y == -1 ? shieldPip.X : shieldPip.Y;
	else if (shieldPip.Z != -1 || shieldPip.X != -1)
		return shieldPip.Z == -1 ? shieldPip.X : shieldPip.Z;

	return isBuilding ? 5 : 16;
}

int ShieldClass::DrawShieldBar_PipAmount(int length) const
{
	return this->IsActive()
		? Math::clamp((int)round(this->GetHealthRatio() * length), 1, length)
		: 0;
}

ArmorType ShieldClass::GetArmorType(TechnoTypeClass* pTechnoType) const
{
	const auto pShieldType = this->Type;
	const auto pTechno = this->Techno;

	if (pTechno && pShieldType->InheritArmorFromTechno)
	{
		if (!pTechnoType)
			pTechnoType = pTechno->GetTechnoType();

		if (pShieldType->InheritArmor_Allowed.empty() || pShieldType->InheritArmor_Allowed.Contains(pTechnoType)
			&& !pShieldType->InheritArmor_Disallowed.Contains(pTechnoType))
		{
			return pTechnoType->Armor;
		}
	}

	return pShieldType->Armor.Get();
}

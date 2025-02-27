#include "ShieldClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <TacticalClass.h>

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
	, Available { true }
	, AreAnimsHidden { false }
	, Attached { isAttached }
	, SelfHealing_Rate_Warhead { -1 }
	, Respawn_Rate_Warhead { -1 }
{
	this->UpdateType();
	this->SetHP(this->Type->InitialStrength.Get(this->Type->Strength));
	this->TechnoID = this->Techno->GetTechnoType();
	ShieldClass::Array.emplace_back(this);
}

ShieldClass::~ShieldClass()
{
	auto it = std::find(ShieldClass::Array.begin(), ShieldClass::Array.end(), this);

	if (it != ShieldClass::Array.end())
		ShieldClass::Array.erase(it);
}

void ShieldClass::UpdateType()
{
	this->Type = TechnoExt::ExtMap.Find(this->Techno)->CurrentShieldType;
}

void ShieldClass::PointerGotInvalid(void* ptr, bool removed)
{
	if (auto const pAnim = abstract_cast<AnimClass*>(static_cast<AbstractClass*>(ptr)))
	{
		for (auto pShield : ShieldClass::Array)
		{
			if (pAnim == pShield->IdleAnim)
				pShield->IdleAnim = nullptr;
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
		.Process(this->Timers.SelfHealing)
		.Process(this->Timers.SelfHealing_WHModifier)
		.Process(this->Timers.Respawn)
		.Process(this->Timers.Respawn_WHModifier)
		.Process(this->HP)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Available)
		.Process(this->Attached)
		.Process(this->AreAnimsHidden)
		.Process(this->Type)
		.Process(this->SelfHealing_Warhead)
		.Process(this->SelfHealing_Rate_Warhead)
		.Process(this->SelfHealing_RestartInCombat_Warhead)
		.Process(this->SelfHealing_RestartInCombatDelay_Warhead)
		.Process(this->Respawn_Warhead)
		.Process(this->Respawn_Rate_Warhead)
		.Process(this->LastBreakFrame)
		.Process(this->LastTechnoHealthRatio)
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
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->Shield)
	{
		pToExt->CurrentShieldType = pFromExt->CurrentShieldType;
		pToExt->Shield = std::make_unique<ShieldClass>(pTo);
		pToExt->Shield->TechnoID = pFromExt->Shield->TechnoID;
		pToExt->Shield->Available = pFromExt->Shield->Available;
		pToExt->Shield->HP = pFromExt->Shield->HP;
	}

	if (pFrom->WhatAmI() == AbstractType::Building && pFromExt->Shield)
		pFromExt->Shield = nullptr;
}

bool ShieldClass::ShieldIsBrokenTEvent(ObjectClass* pAttached)
{
	if (auto pTechno = abstract_cast<TechnoClass*>(pAttached))
	{
		if (auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			ShieldClass* pShield = pExt->Shield.get();
			return !pShield || pShield->HP <= 0;
		}
	}

	return false;
}

int ShieldClass::ReceiveDamage(args_ReceiveDamage* args)
{
	if (!this->HP || this->Temporal || *args->Damage == 0)
		return *args->Damage;

	// Handle a special case where parasite damages shield but not the unit and unit itself cannot be targeted by repair weapons.
	if (*args->Damage < 0)
	{
		if (auto const pFoot = abstract_cast<FootClass*>(this->Techno))
		{
			if (auto const pParasite = pFoot->ParasiteEatingMe)
			{
				// Remove parasite.
				pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
				pParasite->ParasiteImUsing->ExitUnit();
			}
		}
	}

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	bool IC = pWHExt->CanAffectInvulnerable(this->Techno);

	if (!IC || CanBePenetrated(args->WH) || this->Techno->GetTechnoType()->Immune || TechnoExt::IsTypeImmune(this->Techno, args->Attacker))
		return *args->Damage;

	int nDamage = 0;
	int shieldDamage = 0;
	int healthDamage = 0;

	if (pWHExt->CanTargetHouse(args->SourceHouse, this->Techno) && !args->WH->Temporal)
	{
		if (*args->Damage > 0)
			nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->GetArmorType(), args->DistanceToEpicenter);
		else
			nDamage = -MapClass::GetTotalDamage(-*args->Damage, args->WH, this->GetArmorType(), args->DistanceToEpicenter);

		bool affectsShield = pWHExt->Shield_AffectTypes.size() <= 0 || pWHExt->Shield_AffectTypes.Contains(this->Type);
		double absorbPercent = affectsShield ? pWHExt->Shield_AbsorbPercent.Get(this->Type->AbsorbPercent) : this->Type->AbsorbPercent;
		double passPercent = affectsShield ? pWHExt->Shield_PassPercent.Get(this->Type->PassPercent) : this->Type->PassPercent;

		shieldDamage = (int)((double)nDamage * absorbPercent);
		// passthrough damage shouldn't be affected by shield armor
		healthDamage = (int)((double)*args->Damage * passPercent);
	}

	int originalShieldDamage = shieldDamage;
	int min = pWHExt->Shield_ReceivedDamage_Minimum.Get(this->Type->ReceivedDamage_Minimum);
	int max = pWHExt->Shield_ReceivedDamage_Maximum.Get(this->Type->ReceivedDamage_Maximum);
	int minDmg = static_cast<int>(min * pWHExt->Shield_ReceivedDamage_MinMultiplier);
	int maxDmg = static_cast<int>(max * pWHExt->Shield_ReceivedDamage_MaxMultiplier);
	shieldDamage = Math::clamp(shieldDamage, minDmg, maxDmg);

	if (Phobos::DisplayDamageNumbers && shieldDamage != 0)
		GeneralUtils::DisplayDamageNumberString(shieldDamage, DamageDisplayType::Shield, this->Techno->GetRenderCoords(), TechnoExt::ExtMap.Find(this->Techno)->DamageNumberOffset);

	if (shieldDamage > 0)
	{
		bool whModifiersApplied = this->Timers.SelfHealing_WHModifier.InProgress();
		bool restart = whModifiersApplied ? this->SelfHealing_RestartInCombat_Warhead : this->Type->SelfHealing_RestartInCombat;

		if (restart)
		{
			int delay = whModifiersApplied ? this->SelfHealing_RestartInCombatDelay_Warhead : this->Type->SelfHealing_RestartInCombatDelay;

			if (delay > 0)
			{
				this->Timers.SelfHealing_CombatRestart.Start(this->Type->SelfHealing_RestartInCombatDelay);
				this->Timers.SelfHealing.Stop();
			}
			else
			{
				const int rate = whModifiersApplied ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;
				this->Timers.SelfHealing.Start(rate); // when attacked, restart the timer
			}
		}

		if (!pWHExt->Nonprovocative)
			this->ResponseAttack();

		if (pWHExt->DecloakDamagedTargets)
			this->Techno->Uncloak(false);

		int residueDamage = shieldDamage - this->HP;

		if (residueDamage >= 0)
		{
			int actualResidueDamage = Math::max(0, int((double)(originalShieldDamage - this->HP) /
				GeneralUtils::GetWarheadVersusArmor(args->WH, this->GetArmorType()))); //only absord percentage damage

			this->BreakShield(pWHExt->Shield_BreakAnim, pWHExt->Shield_BreakWeapon.Get(nullptr));

			return this->Type->AbsorbOverDamage ? healthDamage : actualResidueDamage + healthDamage;
		}
		else
		{
			if (this->Type->HitFlash && pWHExt->Shield_HitFlash)
			{
				int size = this->Type->HitFlash_FixedSize.Get((shieldDamage * 2));
				SpotlightFlags flags = SpotlightFlags::NoColor;

				if (this->Type->HitFlash_Black)
				{
					flags = SpotlightFlags::NoColor;
				}
				else
				{
					if (!this->Type->HitFlash_Red)
						flags = SpotlightFlags::NoRed;
					if (!this->Type->HitFlash_Green)
						flags |= SpotlightFlags::NoGreen;
					if (!this->Type->HitFlash_Blue)
						flags |= SpotlightFlags::NoBlue;
				}

				MapClass::FlashbangWarheadAt(size, args->WH, this->Techno->Location, true, flags);
			}

			if (!pWHExt->Shield_SkipHitAnim)
				this->WeaponNullifyAnim(pWHExt->Shield_HitAnim);

			this->HP = -residueDamage;

			this->UpdateIdleAnim();

			return healthDamage;
		}
	}
	else if (shieldDamage < 0)
	{
		const int nLostHP = this->Type->Strength - this->HP;

		if (!nLostHP)
		{
			int result = *args->Damage;

			if (result * GeneralUtils::GetWarheadVersusArmor(args->WH, this->Techno->GetTechnoType()->Armor) > 0)
				result = 0;

			return result;
		}

		const int nRemainLostHP = nLostHP + shieldDamage;

		if (nRemainLostHP < 0)
			this->HP = this->Type->Strength;
		else
			this->HP -= shieldDamage;

		this->UpdateIdleAnim();

		return 0;
	}

	// else if (nDamage == 0)
	return healthDamage;
}

void ShieldClass::ResponseAttack()
{
	if (this->Techno->Owner != HouseClass::CurrentPlayer)
		return;

	if (const auto pBld = abstract_cast<BuildingClass*>(this->Techno))
	{
		this->Techno->Owner->BuildingUnderAttack(pBld);
	}
	else if (const auto pUnit = abstract_cast<UnitClass*>(this->Techno))
	{
		if (pUnit->Type->Harvester)
		{
			const auto pos = pUnit->GetDestination(pUnit);
			if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(pos)))
				VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack);
		}
	}
}

void ShieldClass::WeaponNullifyAnim(AnimTypeClass* pHitAnim)
{
	if (this->AreAnimsHidden)
		return;

	const auto pAnimType = pHitAnim ? pHitAnim : this->Type->HitAnim;

	if (pAnimType)
		GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
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

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

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
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	if (this->Techno->Health <= 0 || !this->Techno->IsAlive || this->Techno->IsSinking)
	{
		if (auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno))
		{
			pTechnoExt->Shield = nullptr;
			return;
		}
	}

	if (this->ConvertCheck())
		return;

	this->UpdateType();
	this->CloakCheck();

	if (!this->Available)
		return;

	this->TemporalCheck();

	if (this->Temporal)
		return;

	this->OnlineCheck();
	this->RespawnShield();
	this->SelfHealing();

	double ratio = this->Techno->GetHealthPercentage();

	if (!this->AreAnimsHidden)
	{
		if (GeneralUtils::HasHealthRatioThresholdChanged(LastTechnoHealthRatio, ratio))
			UpdateIdleAnim();

		if (!this->Temporal && this->Online && (this->HP > 0 && this->Techno->Health > 0))
			this->CreateAnim();
	}

	if (this->Timers.Respawn_WHModifier.Completed())
		this->Timers.Respawn_WHModifier.Stop();

	if (this->Timers.SelfHealing_WHModifier.Completed())
		this->Timers.SelfHealing_WHModifier.Stop();

	this->LastTechnoHealthRatio = ratio;
}

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->Cloak = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->Cloak && this->IdleAnim && AnimTypeExt::ExtMap.Find(this->IdleAnim->Type)->DetachOnCloak)
		this->KillAnim();
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		if (this->Online)
			this->UpdateTint();

		this->Online = false;
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
			this->UpdateTint();

		this->Online = true;
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
bool ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->GetTechnoType();

	if (this->TechnoID == newID)
		return false;

	const auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(this->Techno->GetTechnoType());
	const auto pOldType = this->Type;
	bool allowTransfer = this->Type->AllowTransfer.Get(Attached);

	// Update shield type.
	if (!allowTransfer && !pTechnoTypeExt->ShieldType->Strength)
	{
		this->KillAnim();
		pTechnoExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
		pTechnoExt->Shield = nullptr;
		this->UpdateTint();

		return true;
	}
	else if (pTechnoTypeExt->ShieldType->Strength)
	{
		pTechnoExt->CurrentShieldType = pTechnoTypeExt->ShieldType;
	}

	const auto pNewType = pTechnoExt->CurrentShieldType;

	// Update shield properties.
	if (pNewType->Strength && this->Available)
	{
		bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		double healthRatio = this->GetHealthRatio();

		if (pOldType->GetIdleAnimType(isDamaged, healthRatio) != pNewType->GetIdleAnimType(isDamaged, healthRatio))
			this->KillAnim();

		this->HP = (int)round(
			(double)this->HP /
			(double)pOldType->Strength *
			(double)pNewType->Strength
		);
	}
	else
	{
		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		if (pNewType->Strength && !this->Available)
		{ // Resume this shield when became Available
			timer->Resume();
			this->Available = true;
		}
		else if (this->Available)
		{ // Pause this shield when became unAvailable
			timer->Pause();
			this->Available = false;
			this->KillAnim();
		}
	}

	this->TechnoID = newID;
	this->UpdateTint();

	return false;
}

void ShieldClass::SelfHealing()
{
	if (this->Timers.SelfHealing_CombatRestart.InProgress())
	{
		return;
	}
	else if (this->Timers.SelfHealing_CombatRestart.Completed())
	{
		const int rate = this->Timers.SelfHealing_WHModifier.InProgress() ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;
		this->Timers.SelfHealing.Start(rate);
		this->Timers.SelfHealing_CombatRestart.Stop();
	}

	const auto pType = this->Type;
	const auto timer = &this->Timers.SelfHealing;
	const auto timerWHModifier = &this->Timers.SelfHealing_WHModifier;

	if (timerWHModifier->Completed() && timer->InProgress())
	{
		double mult = this->SelfHealing_Rate_Warhead > 0 ? Type->SelfHealing_Rate / this->SelfHealing_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}

	const double amount = timerWHModifier->InProgress() ? this->SelfHealing_Warhead : pType->SelfHealing;
	const int rate = timerWHModifier->InProgress() ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
	const auto percentageAmount = this->GetPercentageAmount(amount);

	if (percentageAmount != 0)
	{
		if ((this->HP < this->Type->Strength || percentageAmount < 0) && timer->StartTime == -1)
			timer->Start(rate);

		if (this->HP > 0 && timer->Completed())
		{
			timer->Start(rate);
			this->HP += percentageAmount;

			this->UpdateIdleAnim();

			if (this->HP > pType->Strength)
			{
				this->HP = pType->Strength;
				timer->Stop();
			}
			else if (this->HP <= 0)
			{
				this->BreakShield();
			}
		}
	}
}

int ShieldClass::GetPercentageAmount(double iStatus)
{
	if (iStatus == 0)
		return 0;

	if (iStatus >= -1.0 && iStatus <= 1.0)
		return (int)std::round(this->Type->Strength * iStatus);

	return (int)std::trunc(iStatus);
}

void ShieldClass::BreakShield(AnimTypeClass* pBreakAnim, WeaponTypeClass* pBreakWeapon)
{
	this->HP = 0;

	if (this->Type->Respawn)
		this->Timers.Respawn.Start(Timers.Respawn_WHModifier.InProgress() ? Respawn_Rate_Warhead : this->Type->Respawn_Rate);

	this->Timers.SelfHealing.Stop();
	this->KillAnim();

	if (!this->AreAnimsHidden)
	{
		const auto pAnimType = pBreakAnim ? pBreakAnim : this->Type->BreakAnim;

		if (pAnimType)
		{
			auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location);

			pAnim->SetOwnerObject(this->Techno);
			pAnim->Owner = this->Techno->Owner;
		}
	}

	const auto pWeaponType = pBreakWeapon ? pBreakWeapon : this->Type->BreakWeapon;
	this->LastBreakFrame = Unsorted::CurrentFrame;
	this->UpdateTint();

	if (pWeaponType)
		TechnoExt::FireWeaponAtSelf(this->Techno, pWeaponType);
}

void ShieldClass::RespawnShield()
{
	const auto timer = &this->Timers.Respawn;
	const auto timerWHModifier = &this->Timers.Respawn_WHModifier;

	if (this->HP <= 0 && timer->Completed())
	{
		timer->Stop();
		double amount = timerWHModifier->InProgress() ? Respawn_Warhead : this->Type->Respawn;
		this->HP = this->GetPercentageAmount(amount);
		this->UpdateTint();
	}
	else if (timerWHModifier->Completed() && timer->InProgress())
	{
		double mult = this->Respawn_Rate_Warhead > 0 ? Type->Respawn_Rate / this->Respawn_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetRespawn(int duration, double amount, int rate, bool resetTimer)
{
	const auto timer = &this->Timers.Respawn;
	const auto timerWHModifier = &this->Timers.Respawn_WHModifier;

	bool modifierTimerInProgress = timerWHModifier->InProgress();
	this->Respawn_Warhead = amount;
	this->Respawn_Rate_Warhead = rate >= 0 ? rate : Type->Respawn_Rate;

	timerWHModifier->Start(duration);

	if (this->HP <= 0 && Respawn_Rate_Warhead >= 0 && resetTimer)
	{
		timer->Start(Respawn_Rate_Warhead);
	}
	else if (timer->InProgress() && !modifierTimerInProgress && this->Respawn_Rate_Warhead != Type->Respawn_Rate)
	{
		double mult = Type->Respawn_Rate > 0 ? this->Respawn_Rate_Warhead / Type->Respawn_Rate : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer)
{
	auto timer = &this->Timers.SelfHealing;
	auto timerWHModifier = &this->Timers.SelfHealing_WHModifier;

	bool modifierTimerInProgress = timerWHModifier->InProgress();
	this->SelfHealing_Warhead = amount;
	this->SelfHealing_Rate_Warhead = rate >= 0 ? rate : Type->SelfHealing_Rate;
	this->SelfHealing_RestartInCombat_Warhead = restartInCombat;
	this->SelfHealing_RestartInCombatDelay_Warhead = restartInCombatDelay >= 0 ? restartInCombatDelay : Type->SelfHealing_RestartInCombatDelay;

	timerWHModifier->Start(duration);

	if (resetTimer)
	{
		timer->Start(this->SelfHealing_Rate_Warhead);
	}
	else if (timer->InProgress() && !modifierTimerInProgress && this->SelfHealing_Rate_Warhead != Type->SelfHealing_Rate)
	{
		double mult = Type->SelfHealing_Rate > 0 ? this->SelfHealing_Rate_Warhead / Type->SelfHealing_Rate : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::CreateAnim()
{
	auto idleAnimType = this->GetIdleAnimType();

	if (this->Cloak && (!idleAnimType || AnimTypeExt::ExtMap.Find(idleAnimType)->DetachOnCloak))
		return;

	if (!this->IdleAnim && idleAnimType)
	{
		auto const pAnim = GameCreate<AnimClass>(idleAnimType, this->Techno->Location);

		pAnim->SetOwnerObject(this->Techno);
		AnimExt::SetAnimOwnerHouseKind(pAnim, this->Techno->Owner, nullptr, false, true);
		pAnim->RemainingIterations = 0xFFu;
		this->IdleAnim = pAnim;
	}
}

void ShieldClass::KillAnim()
{
	if (this->IdleAnim)
	{
		this->IdleAnim->UnInit();
		this->IdleAnim = nullptr;
	}
}

void ShieldClass::UpdateIdleAnim()
{
	if (this->IdleAnim && this->IdleAnim->Type != this->GetIdleAnimType())
	{
		this->KillAnim();
		this->CreateAnim();
	}
}

void ShieldClass::UpdateTint()
{
	if (this->Type->HasTint())
		this->Techno->MarkForRedraw();
}

AnimTypeClass* ShieldClass::GetIdleAnimType()
{
	if (!this->Type || !this->Techno)
		return nullptr;

	bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	return this->Type->GetIdleAnimType(isDamaged, this->GetHealthRatio());
}

bool ShieldClass::IsGreenSP()
{
	return this->Type->GetConditionYellow() * this->Type->Strength.Get() < this->HP;
}

bool ShieldClass::IsYellowSP()
{
	return this->Type->GetConditionRed() * this->Type->Strength.Get() < this->HP && this->HP <= this->Type->GetConditionYellow() * this->Type->Strength.Get();
}

bool ShieldClass::IsRedSP()
{
	return this->HP <= this->Type->GetConditionYellow() * this->Type->Strength.Get();
}

void ShieldClass::DrawShieldBar_Building(const int length, RectangleStruct* pBound)
{
	Point2D position = { 0, 0 };
	const int totalLength = DrawShieldBar_PipAmount(length);
	int frame = this->DrawShieldBar_Pip(true);

	if (totalLength > 0)
	{
		for (int frameIdx = totalLength, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			position = TechnoExt::GetBuildingSelectBracketPosition(Techno, BuildingSelectBracketPosition::Top);
			position.X -= deltaX + 6;
			position.Y -= deltaY + 3;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				frame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (totalLength < length)
	{
		for (int frameIdx = length - totalLength, deltaX = 4 * totalLength, deltaY = -2 * totalLength;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			position = TechnoExt::GetBuildingSelectBracketPosition(Techno, BuildingSelectBracketPosition::Top);
			position.X -= deltaX + 6;
			position.Y -= deltaY + 3;

			const int emptyFrame = this->Type->Pips_Building_Empty.Get(RulesExt::Global()->Pips_Shield_Building_Empty.Get(0));

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				emptyFrame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(const int length, RectangleStruct* pBound)
{
	auto position = TechnoExt::GetFootSelectBracketPosition(Techno, Anchor(HorizontalPosition::Left, VerticalPosition::Top));
	const auto pipBoard = this->Type->Pips_Background.Get(RulesExt::Global()->Pips_Shield_Background.Get(FileSystem::PIPBRD_SHP()));
	int frame;

	position.X -= 1;
	position.Y += this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta - 3;

	if (length == 8)
		frame = pipBoard->Frames > 2 ? 3 : 1;
	else
		frame = pipBoard->Frames > 2 ? 2 : 0;

	if (this->Techno->IsSelected)
	{
		position.X += length + 1 + (length == 8 ? length + 1 : 0);
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pipBoard,
			frame, &position, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		position.X -= length + 1 + (length == 8 ? length + 1 : 0);
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
	const int strength = this->Type->Strength.Get();
	const auto pipsShield = isBuilding ? this->Type->Pips_Building.Get() : this->Type->Pips.Get();
	const auto pipsGlobal = isBuilding ? RulesExt::Global()->Pips_Shield_Building.Get() : RulesExt::Global()->Pips_Shield.Get();

	CoordStruct shieldPip;

	if (pipsShield.X != -1)
		shieldPip = pipsShield;
	else
		shieldPip = pipsGlobal;

	if (this->HP > this->Type->GetConditionYellow() * strength && shieldPip.X != -1)
		return shieldPip.X;
	else if (this->HP > this->Type->GetConditionRed() * strength && (shieldPip.Y != -1 || shieldPip.X != -1))
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

ArmorType ShieldClass::GetArmorType() const
{
	if (this->Techno && this->Type->InheritArmorFromTechno)
		return this->Techno->GetTechnoType()->Armor;

	return this->Type->Armor.Get();
}

int ShieldClass::GetFramesSinceLastBroken() const
{
	return Unsorted::CurrentFrame - this->LastBreakFrame;
}

void ShieldClass::SetAnimationVisibility(bool visible)
{
	if (!this->AreAnimsHidden && !visible)
		this->KillAnim();

	this->AreAnimsHidden = !visible;
}

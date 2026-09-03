#pragma once

#include <New/Type/ShieldTypeClass.h>

#include "SpecificStructures.h"

class TechnoClass;
class WarheadTypeClass;

class ShieldClass
{
public:
	static std::vector<ShieldClass*> Array;

	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) { };
	~ShieldClass();

	int ReceiveDamage(args_ReceiveDamage* args);
	bool CanBeTargeted(WeaponTypeClass* pWeapon) const;
	bool CanBePenetrated(WarheadTypeClass* pWarhead) const;
	void BreakShield(const std::vector<AnimTypeClass*>& pBreakAnim, WeaponTypeClass* pBreakWeapon = nullptr);

	void SetRespawn(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer, std::vector<AnimTypeClass*> anim, WeaponTypeClass* weapon = nullptr);
	void SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer);
	void SetRespawnRestartInCombat();

	void KillAnim()
	{
		if (auto& pAnim = this->IdleAnim)
		{
			pAnim->UnInit();
			pAnim = nullptr;
		}
	}

	void AI_Temporal();
	void AI();

	void DrawShieldBar_Building(const int length, RectangleStruct* pBound);
	void DrawShieldBar_Other(const int length, RectangleStruct* pBound, bool isInfantry);

	double GetHealthRatio() const
	{
		return static_cast<double>(this->HP) / this->Type->Strength;
	}

	void SetHP(int amount)
	{
		this->HP = std::min(amount, this->Type->Strength.Get());
	}

	int GetHP() const
	{
		return this->HP;
	}

	bool IsActive() const
	{
		return this->HP > 0
			&& this->Online;
	}

	bool IsBrokenAndNonRespawning() const
	{
		return this->HP <= 0 && !(this->Timers.Respawn_WHModifier.InProgress() ? this->Respawn_Warhead : this->Type->Respawn);
	}

	ShieldTypeClass* GetType() const
	{
		return this->Type;
	}

	ArmorType GetArmorType(TechnoTypeClass* pTechnoType = nullptr) const;
	int GetFramesSinceLastBroken() const { return Unsorted::CurrentFrame - this->LastBreakFrame; }
	void UpdateTint();

	void SetAnimationVisibility(bool visible)
	{
		if (!this->AreAnimsHidden && !visible)
			this->KillAnim();

		this->AreAnimsHidden = !visible;
	}

	void ConvertCheck(TechnoTypeClass* pTechnoType);

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool ShieldIsBrokenTEvent(ObjectClass* pAttached);

	bool IsGreenSP() const
	{
		auto const pType = this->Type;
		return pType->GetConditionYellow() * pType->Strength.Get() < this->HP;
	}

	bool IsYellowSP() const
	{
		auto const pType = this->Type;
		const int health = this->HP;
		const int strength = pType->Strength.Get();
		return pType->GetConditionRed() * strength < health && health <= pType->GetConditionYellow() * strength;
	}

	bool IsRedSP() const
	{
		auto const pType = this->Type;
		return this->HP <= pType->GetConditionRed() * pType->Strength.Get();
	}

	static void PointerGotInvalid(void* ptr, bool removed);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	int GetPercentageAmount(double iStatus)
	{
		if (iStatus == 0)
			return 0;

		if (iStatus >= -1.0 && iStatus <= 1.0)
			return (int)std::round(this->Type->Strength * iStatus);

		return (int)std::trunc(iStatus);
	}

	void SelfHealing();
	void RespawnShield();

	void CreateAnim(ShieldTypeClass* pType, AnimTypeClass* idleAnimType = nullptr);
	void UpdateIdleAnim(ShieldTypeClass* pType, double ratio = 0.0);
	AnimTypeClass* GetIdleAnimType(ShieldTypeClass* pType, bool idleAnimSet, bool idleAnimDamagedSet, double ratio = 0.0);

	void WeaponNullifyAnim(const std::vector<AnimTypeClass*>& pHitAnim);
	void ResponseAttack();

	inline void CloakCheck();
	inline void TemporalCheck();
	void OnlineCheck();
	void EnabledByCheck();

	inline int DrawShieldBar_Pip(const bool isBuilding) const;
	inline int DrawShieldBar_PipAmount(const int length) const;

	/// Properties ///
	TechnoClass* Techno;
	TechnoTypeClass* TechnoID;
	int HP;
	AnimClass* IdleAnim;
	bool Cloak;
	bool Online;
	bool Temporal;
	bool Attached;
	bool AreAnimsHidden;
	bool IsSelfHealingEnabled;
	int BracketDelta;

	double SelfHealing_Warhead;
	int SelfHealing_Rate_Warhead;
	bool SelfHealing_RestartInCombat_Warhead;
	int SelfHealing_RestartInCombatDelay_Warhead;
	double Respawn_Warhead;
	int Respawn_Rate_Warhead;
	bool Respawn_RestartInCombat_Warhead;
	int Respawn_RestartInCombatDelay_Warhead;
	std::vector<AnimTypeClass*> Respawn_Anim_Warhead;
	WeaponTypeClass* Respawn_Weapon_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;

	struct Timers
	{
		Timers() :
			SelfHealing_CombatRestart { }
			, SelfHealing { }
			, SelfHealing_WHModifier { }
			, Respawn_CombatRestart { }
			, Respawn { }
			, Respawn_WHModifier { }
		{ }

		CDTimerClass SelfHealing_CombatRestart;
		CDTimerClass SelfHealing;
		CDTimerClass SelfHealing_WHModifier;
		CDTimerClass Respawn_CombatRestart;
		CDTimerClass Respawn;
		CDTimerClass Respawn_WHModifier;

	} Timers;
};

#pragma once

#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <Ext/TechnoType/Body.h>

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
	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);

	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer);
	void KillAnim();
	void AI_Temporal();
	void AI();

	void DrawShieldBar_Building(const int length, RectangleStruct* pBound);
	void DrawShieldBar_Other(const int length, RectangleStruct* pBound);
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
		return
			this->Available &&
			this->HP > 0 &&
			this->Online;
	}
	bool IsAvailable() const
	{
		return this->Available;
	}
	bool IsBrokenAndNonRespawning() const
	{
		return this->HP <= 0 && !this->Type->Respawn;
	}
	ShieldTypeClass* GetType() const
	{
		return this->Type;
	}
	ArmorType GetArmorType() const;
	int GetFramesSinceLastBroken() const;
	void SetAnimationVisibility(bool visible);

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool ShieldIsBrokenTEvent(ObjectClass* pAttached);

	bool IsGreenSP();
	bool IsYellowSP();
	bool IsRedSP();

	static void PointerGotInvalid(void* ptr, bool removed);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();
	int GetPercentageAmount(double iStatus);

	void RespawnShield();

	void CreateAnim();
	void UpdateIdleAnim();
	AnimTypeClass* GetIdleAnimType();

	void WeaponNullifyAnim(AnimTypeClass* pHitAnim = nullptr);
	void ResponseAttack();

	void CloakCheck();
	void OnlineCheck();
	void TemporalCheck();
	bool ConvertCheck();

	int DrawShieldBar_Pip(const bool isBuilding) const;
	int DrawShieldBar_PipAmount(const int length) const;

	void UpdateTint();

	/// Properties ///
	TechnoClass* Techno;
	TechnoTypeClass* TechnoID;
	int HP;
	AnimClass* IdleAnim;
	bool Cloak;
	bool Online;
	bool Temporal;
	bool Available;
	bool Attached;
	bool AreAnimsHidden;

	double SelfHealing_Warhead;
	int SelfHealing_Rate_Warhead;
	bool SelfHealing_RestartInCombat_Warhead;
	int SelfHealing_RestartInCombatDelay_Warhead;
	double Respawn_Warhead;
	int Respawn_Rate_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;

	struct Timers
	{
		Timers() :
			SelfHealing_CombatRestart { }
			, SelfHealing { }
			, SelfHealing_WHModifier { }
			, Respawn { }
			, Respawn_WHModifier { }
		{ }

		CDTimerClass SelfHealing_CombatRestart;
		CDTimerClass SelfHealing;
		CDTimerClass SelfHealing_WHModifier;
		CDTimerClass Respawn;
		CDTimerClass Respawn_WHModifier;

	} Timers;
};

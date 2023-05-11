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
	void SetSelfHealing(int duration, double amount, int rate, bool resetTimer);
	void KillAnim();
	void AI_Temporal();
	void AI();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	double GetHealthRatio() const;
	void SetHP(int amount);
	int GetHP() const;
	bool IsActive() const;
	bool IsAvailable() const;
	bool IsBrokenAndNonRespawning() const;
	ShieldTypeClass* GetType() const;
	ArmorType GetArmorType() const;
	int GetFramesSinceLastBroken() const;
	void SetAnimationVisibility(bool visible);

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool ShieldIsBrokenTEvent(ObjectClass* pAttached);

	virtual bool InvalidateIgnorable(void* const ptr) const;
	virtual void InvalidatePointer(void* ptr, bool bDetach);

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

	void DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	int DrawShieldBar_Pip(const bool isBuilding) const;
	int DrawShieldBar_PipAmount(int iLength) const;

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
	double Respawn_Warhead;
	int Respawn_Rate_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;

	struct Timers
	{
		Timers() :
			SelfHealing { }
			, SelfHealing_WHModifier { }
			, Respawn { }
			, Respawn_WHModifier { }
		{ }

		CDTimerClass SelfHealing;
		CDTimerClass SelfHealing_WHModifier;
		CDTimerClass Respawn;
		CDTimerClass Respawn_WHModifier;

	} Timers;
};

#pragma once

#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <Ext/TechnoType/Body.h>

class TechnoClass;
class WarheadTypeClass;

class ShieldClass
{
public:
	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) {};
	~ShieldClass() = default;

	int ReceiveDamage(args_ReceiveDamage* args);
	bool CanBeTargeted(WeaponTypeClass* pWeapon);
	bool CanBePenetrated(WarheadTypeClass* pWarhead);

	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);
	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool resetTimer);

	void KillAnim();

	void AI_Temporal();
	void AI();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void InvalidatePointer(void* ptr);

	double GetHealthRatio();
	void SetHP(int amount);
	int GetHP();
	bool IsActive();
	bool IsAvailable();
	bool IsBrokenAndNonRespawning();
	ShieldTypeClass* GetType();
	int GetFramesSinceLastBroken();

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);

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
	int DrawShieldBar_Pip(const bool isBuilding);
	int DrawShieldBar_PipAmount(int iLength);

	/// Properties ///
	TechnoClass* Techno;
	char TechnoID[0x18];
	int HP;
	AnimClass* IdleAnim;
	bool Cloak;
	bool Online;
	bool Temporal;
	bool Available;
	bool Attached;

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
			SelfHealing{ }
			, SelfHealing_Warhead { }
			, Respawn{ }
			, Respawn_Warhead { }
		{ }

		TimerStruct SelfHealing;
		TimerStruct SelfHealing_Warhead;
		TimerStruct Respawn;
		TimerStruct Respawn_Warhead;

	} Timers;
};

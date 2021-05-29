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
	ShieldClass(TechnoClass* pTechno);
	~ShieldClass() = default;

	int ReceiveDamage(args_ReceiveDamage* args);
	bool CanBeTargeted(WeaponTypeClass* pWeapon);
	void AI();
	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void InvalidatePointer(void* ptr);
	int GetHP();
	bool IsAvailable();

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T1, typename T2>
	static bool Serialize(T1 pThis, T2& Stm);

	const ShieldTypeClass* GetType();

	void SelfHealing();
	int GetPercentageAmount(double iStatus);

	void BreakShield();
	void RespawnShield();

	void CreateAnim();
	void KillAnim();

	void WeaponNullifyAnim();
	void ResponseAttack();

	void TemporalCheck();
	void ConvertCheck();

	void DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	int DrawShieldBar_Pip(const bool isBuilding);
	int DrawShieldBar_PipAmount(int iLength);

	/// Properties ///
	TechnoClass* Techno;
	char TechnoID[0x18];
	bool Available;
	int HP;
	AnimClass* IdleAnim;
	bool Temporal;

	struct Timers
	{
		Timers() :
			SelfHealing{ },
			Respawn{ }
		{ }

		TimerStruct SelfHealing;
		TimerStruct Respawn;

	} Timers;
};

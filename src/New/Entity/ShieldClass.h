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

	void KillAnim();

	void AI_Temporal();
	void AI();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void InvalidatePointer(void* ptr);

	double GetHealthRatio();
	int GetHP();
	bool IsActive();
	bool IsAvailable();

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();
	int GetPercentageAmount(double iStatus);

	void BreakShield();
	void RespawnShield();

	void CreateAnim();

	void WeaponNullifyAnim();
	void ResponseAttack();

	void CloakCheck();
	void OnlineCheck();
	void TemporalCheck();
	void ConvertCheck();

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

	ShieldTypeClass* Type;

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

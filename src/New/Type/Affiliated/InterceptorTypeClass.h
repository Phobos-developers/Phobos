#pragma once

#include <Utilities/TemplateDef.h>

class InterceptorTypeClass
{
public:

	InterceptorTypeClass() = default;

	Valueable<int> TargetingDelay { 1 };
	Valueable<AffectedHouse> CanTargetHouses { AffectedHouse::Enemies };
	Promotable<Leptons> GuardRange { Leptons(0) };
	Promotable<Leptons> MinimumGuardRange { Leptons(0) };
	Nullable<bool> GuardRange_IsCylindrical {};
	Valueable<int> Weapon { 0 };
	Nullable<WeaponTypeClass*> WeaponOverride {};
	Valueable<bool> WeaponReplaceProjectile { false };
	Valueable<bool> WeaponCumulativeDamage { false };
	Valueable<bool> KeepIntact { false };
	Nullable<bool> ApplyFirepowerMult {};
	Nullable<bool> DeleteOnIntercept {};

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};

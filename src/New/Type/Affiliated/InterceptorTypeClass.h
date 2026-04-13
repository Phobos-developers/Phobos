#pragma once

#include <Utilities/TemplateDef.h>

class InterceptorTypeClass
{
public:

	InterceptorTypeClass() = default;

	InterceptorTypeClass(TechnoTypeClass* OwnedBy);

	TechnoTypeClass* OwnerType;

	Valueable<int> TargetingDelay;
	Valueable<AffectedHouse> CanTargetHouses;
	Promotable<Leptons> GuardRange;
	Promotable<Leptons> MinimumGuardRange;
	Valueable<int> Weapon;
	Nullable<WeaponTypeClass*> WeaponOverride;
	Valueable<bool> WeaponReplaceProjectile;
	Valueable<bool> WeaponCumulativeDamage;
	Valueable<bool> KeepIntact;
	Valueable<bool> ApplyFirepowerMult;
	Nullable<bool> DeleteOnIntercept;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};

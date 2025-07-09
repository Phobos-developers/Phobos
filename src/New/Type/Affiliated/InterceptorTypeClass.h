#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class InterceptorTypeClass
{
public:

	InterceptorTypeClass() = default;

	InterceptorTypeClass(TechnoTypeClass* OwnedBy);

	TechnoTypeClass* OwnerType;

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

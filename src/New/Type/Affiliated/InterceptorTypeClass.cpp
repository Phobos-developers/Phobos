#include "InterceptorTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

InterceptorTypeClass::InterceptorTypeClass(TechnoTypeClass* OwnedBy) : OwnerType { OwnedBy }
	, CanTargetHouses { AffectedHouse::Enemies }
	, GuardRange {}
	, MinimumGuardRange {}
	, Weapon { 0 }
	, DeleteOnIntercept {}
	, WeaponOverride {}
	, WeaponReplaceProjectile { false }
	, WeaponCumulativeDamage { false }
	, KeepIntact { false }
{ }

void InterceptorTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->WeaponOverride.Read<true>(exINI, pSection, "Interceptor.WeaponOverride");
	this->WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
}

#pragma region(save/load)

template <class T>
bool InterceptorTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnerType)
		.Process(this->CanTargetHouses)
		.Process(this->GuardRange)
		.Process(this->MinimumGuardRange)
		.Process(this->Weapon)
		.Process(this->DeleteOnIntercept)
		.Process(this->WeaponOverride)
		.Process(this->WeaponReplaceProjectile)
		.Process(this->WeaponCumulativeDamage)
		.Process(this->KeepIntact)
		.Success();
}

bool InterceptorTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool InterceptorTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<InterceptorTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

#pragma once

#include <Utilities/EnumFunctions.h>
#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/ShieldTypeClass.h>

class HandlerFilterClass
{
public:
	Nullable<AffectedHouse> House;
	ValueableVector<TechnoTypeClass*> TechnoTypes;
	ValueableVector<AttachEffectTypeClass*> AttachedEffects;
	ValueableVector<ShieldTypeClass*> ShieldTypes;
	ValueableVector<SideClass*> Side;
	ValueableVector<HouseTypeClass*> Country;
	Nullable<VeterancyType> Veterancy;
	Nullable<HPPercentageType> HPPercentage;
	Nullable<bool> Passengers_HasAny;
	ValueableVector<TechnoTypeClass*> Passengers_HasType;

	static std::unique_ptr<HandlerFilterClass> Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* filterName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* filterName);

	bool Check(TechnoClass* pOwner, TechnoClass* pTarget, bool negative = false) const;

	bool IsDefined() const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

#pragma once

#include <Utilities/Template.h>
#include "TypeConvertGroup.h"

class HandlerEffectClass
{
public:
	HandlerEffectClass();

	Nullable<WeaponTypeClass*> Weapon;
	std::vector<TypeConvertGroup> Convert_Pairs;

	static std::unique_ptr<HandlerEffectClass> Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void Execute(TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool IsDefined() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);
};

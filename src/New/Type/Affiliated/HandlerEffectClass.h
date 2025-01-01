#pragma once

#include <Utilities/EnumFunctions.h>

class HandlerEffectClass
{
public:
	Nullable<WeaponTypeClass*> Weapon;

	static std::unique_ptr<HandlerEffectClass> Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void Execute(TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool IsDefined() const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

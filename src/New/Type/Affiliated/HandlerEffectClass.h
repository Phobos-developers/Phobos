#pragma once

#include <Utilities/EnumFunctions.h>
#include <New/Type/EventHandlerTypeClass.h>

class HandlerEffectClass
{
public:
	HandlerEffectClass() = default;

	HandlerEffectClass(EventHandlerTypeClass* OwnedBy);

	EventHandlerTypeClass* OwnerType;

	Nullable<WeaponTypeClass*> Weapon;

	static std::unique_ptr<HandlerEffectClass> Parse(EventHandlerTypeClass* OwnedBy, INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void Execute(TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool IsDefined() const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

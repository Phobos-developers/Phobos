#include <Ext/Techno/Body.h>
#include "HandlerEffectClass.h"
#include <Ext/WeaponType/Body.h>

HandlerEffectClass::HandlerEffectClass()
	: Weapon {}
{ }

std::unique_ptr<HandlerEffectClass> HandlerEffectClass::Parse(INI_EX & exINI, const char* pSection, const char* scopeName, const char* effectName)
{
	auto effect = std::make_unique<HandlerEffectClass>();
	effect.get()->LoadFromINI(exINI, pSection, scopeName, effectName);
	if (effect.get()->IsDefined())
	{
		return effect;
	}
	else
	{
		effect.reset();
		return nullptr;
	}
}

void HandlerEffectClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName)
{
	char tempBuffer[64];
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Weapon", scopeName, effectName);
	Weapon.Read(exINI, pSection, tempBuffer);
}

void HandlerEffectClass::Execute(TechnoClass* pOwner, TechnoClass* pTarget) const
{
	if (Weapon.isset())
	{
		WeaponTypeExt::DetonateAt(Weapon.Get(), pTarget->GetCoords(), pOwner, Weapon.Get()->Damage, pOwner->GetOwningHouse());
	}
}

bool HandlerEffectClass::IsDefined() const
{
	return Weapon.isset();
}

template<typename T>
bool HandlerEffectClass::Serialize(T& stm)
{
	return stm
		.Process(this->Weapon)
		.Success();
}

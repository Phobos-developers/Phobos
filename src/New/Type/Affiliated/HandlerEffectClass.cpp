#include "HandlerEffectClass.h"
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

HandlerEffectClass::HandlerEffectClass()
	: Weapon {}
	, Convert_Pairs {}
{ }

std::unique_ptr<HandlerEffectClass> HandlerEffectClass::Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName)
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
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s.Convert", scopeName, effectName);
	TypeConvertGroup::Parse(this->Convert_Pairs, exINI, pSection, AffectedHouse::All, tempBuffer);
}

void HandlerEffectClass::Execute(TechnoClass* pOwner, TechnoClass* pTarget) const
{
	if (Weapon.isset())
	{
		WeaponTypeExt::DetonateAt(Weapon.Get(), pTarget->GetCoords(), pOwner, Weapon.Get()->Damage, pOwner->GetOwningHouse());
	}
	if (!Convert_Pairs.empty())
	{
		TypeConvertGroup::Convert(static_cast<FootClass*>(pTarget), this->Convert_Pairs, pOwner->Owner);
	}
}

bool HandlerEffectClass::IsDefined() const
{
	return Weapon.isset()
		|| !Convert_Pairs.empty();
}

bool HandlerEffectClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool HandlerEffectClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HandlerEffectClass*>(this)->Serialize(stm);
}

template<typename T>
bool HandlerEffectClass::Serialize(T& stm)
{
	return stm
		.Process(this->Weapon)
		.Process(this->Convert_Pairs)
		.Success();
}

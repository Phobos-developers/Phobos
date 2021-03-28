#pragma once

#include "../Utilities/Template.h"
#include "../Utilities/TemplateDef.h"
#include "../Utilities/GeneralUtils.h"
#include "_Enumerator.hpp"
#include "../Phobos.CRT.h"
#include "../Phobos.h"

#include <SuperWeaponTypeClass.h>

class WeaponTypeClass;

class CrateType final : public Enumerable<CrateType> {
public:
	
	
	Valueable<SuperWeaponTypeClass*> SWs{ nullptr };
	Valueable<WeaponTypeClass*> WeaponType{ nullptr };
	Valueable<bool>SWGrant{ false };
	Valueable<int> Chance{ 0 } ;
	Valueable<AnimTypeClass*> Anim{ nullptr };
	Valueable<int> Tp{ 3 };


	CrateType(const char* pTitle);

	
	virtual ~CrateType() override;

	virtual void LoadFromINI(CCINIClass *pINI) override;
	static void LoadListSection(CCINIClass * pINI);
	
	virtual void LoadFromStream(PhobosStreamReader &Stm) override;

	virtual void SaveToStream(PhobosStreamWriter &Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};

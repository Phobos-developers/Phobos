#pragma once
#include <SuperWeaponTypeClass.h>

#include "_Enumerator.hpp"
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;
class VocClass;
class VoxClass;
class WeaponTypeClass;

class CrateTypeClass final : public Enumerable<CrateTypeClass>
{
public:
	
    ValueableIdx<SuperWeaponTypeClass> Super;
	Valueable<WeaponTypeClass*> WeaponType;
	Valueable<bool> SuperGrant;
	Valueable<int> Chance;
	Valueable<AnimTypeClass*> Anim;
	Valueable<int> Type;
	Valueable<bool> AllowWater;
    ValueableIdx<VocClass> Sound;
    ValueableIdx<VoxClass> Eva;
	ValueableVector<UnitTypeClass*> Unit;
    Valueable<int> MoneyMin;
    Valueable<int> MoneyMax;
	
    CrateTypeClass(const char* const pTitle): Enumerable<CrateTypeClass>(pTitle),
        Super(-1),
        WeaponType(nullptr),
        Chance(0),
        Anim(nullptr),
        Type(3),
        SuperGrant(false),
        AllowWater(false),
        Sound(-1),
        Eva(-1),
        Unit(),
        MoneyMin(0),
        MoneyMax(0)
    { }

	virtual ~CrateTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass *pINI) override;

	static void LoadListSection(CCINIClass * pINI);

	virtual void LoadFromStream(PhobosStreamReader &Stm) override;

	virtual void SaveToStream(PhobosStreamWriter &Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
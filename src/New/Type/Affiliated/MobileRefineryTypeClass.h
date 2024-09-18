#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class MobileRefineryTypeClass
{
public:

	MobileRefineryTypeClass() = default;

	MobileRefineryTypeClass(TechnoTypeClass* OwnedBy);

	TechnoTypeClass* OwnerType;

	Valueable<int> TransDelay;
	Valueable<float>  CashMultiplier;
	Valueable<int> AmountPerCell;
	ValueableVector<int> FrontOffset;
	ValueableVector<int> LeftOffset;
	Valueable<bool> Display;
	Valueable<AffectedHouse> Display_House;
	ValueableVector<AnimTypeClass*> Anims;
	Valueable<bool> AnimMove;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool, bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};

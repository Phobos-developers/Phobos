#pragma once
#include <Utilities/TemplateDef.h>
#include <TechnoClass.h>
class SquadManager
{
public:
	ValueableVector<TechnoClass*> Squad_Members;
	bool isSelected;

	SquadManager():
		Squad_Members { },
		isSelected { false }
	{ }

	void addTechno(TechnoClass* pTechno);
	void delTechno(TechnoClass* pTechno);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};


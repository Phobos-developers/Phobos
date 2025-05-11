#pragma once
#include <Utilities/TemplateDef.h>
#include <TechnoClass.h>
class SquadManager
{
public:
	static std::vector<SquadManager*> Array;

	ValueableVector<TechnoClass*> Squad_Members;
	bool isSelected;

	SquadManager();

	~SquadManager();
	

	void AddTechno(TechnoClass* pTechno);
	void RemoveTechno(TechnoClass* pTechno);

	void PointerGotInvalid(void* ptr, bool removed);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

template <>
struct Savegame::ObjectFactory<SquadManager>
{
	std::unique_ptr<SquadManager> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<SquadManager>();
	}
};

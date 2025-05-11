#pragma once
#include <Utilities/TemplateDef.h>
#include <TechnoClass.h>
class SquadManager
{
public:
	static std::vector<SquadManager*> Array;

	ValueableVector<TechnoClass*> Squad_Members;
	bool isSelected;

	SquadManager():
		Squad_Members { },
		isSelected { false }
	{
		SquadManager::Array.emplace_back(this);
	}

	~SquadManager()
	{
		auto it = std::find(SquadManager::Array.begin(), SquadManager::Array.end(), this);

		if (it != SquadManager::Array.end())
			SquadManager::Array.erase(it);
	}

	void addTechno(TechnoClass* pTechno);
	void delTechno(TechnoClass* pTechno);

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

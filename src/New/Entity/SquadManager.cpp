#include "SquadManager.h"

void SquadManager::addTechno(TechnoClass* pTechno)
{
	this->Squad_Members.AddUnique(pTechno);
}

void SquadManager::delTechno(TechnoClass* pTechno)
{
	/*
	int pos = this->Squad_Members.IndexOf(pTechno);
	if (pos != -1)
	{
		auto deletePos = this->Squad_Members.begin() + pos;
		this->Squad_Members.erase(deletePos, deletePos+1);
	}
	*/
	auto it = std::find(this->Squad_Members.begin(), this->Squad_Members.end(), pTechno);
	
	if (it != this->Squad_Members.end())
		this->Squad_Members.erase(it);
}

void SquadManager::PointerGotInvalid(void* ptr, bool removed)
{ }

#pragma region Save/Load

template <typename T>
bool SquadManager::Serialize(T& stm)
{
	return stm
		.Process(this->Squad_Members)
		.Process(this->isSelected)
		.Success();
};

bool SquadManager::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool SquadManager::Save(PhobosStreamWriter& stm) const
{
	return const_cast<SquadManager*>(this)->Serialize(stm);
}

#pragma endregion

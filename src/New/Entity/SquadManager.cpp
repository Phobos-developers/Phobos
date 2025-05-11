#include "SquadManager.h"

std::vector<SquadManager*> SquadManager::Array;

SquadManager::SquadManager():
	Squad_Members { },
	isSelected { false }
{
	SquadManager::Array.emplace_back(this);
}

SquadManager::~SquadManager()
{
	auto it = std::find(SquadManager::Array.begin(), SquadManager::Array.end(), this);

	if (it != SquadManager::Array.end())
		SquadManager::Array.erase(it);
}

void SquadManager::AddTechno(TechnoClass* pTechno)
{
	this->Squad_Members.AddUnique(pTechno);
}

void SquadManager::RemoveTechno(TechnoClass* pTechno)
{
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

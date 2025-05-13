#include "SquadManager.h"
#include <Ext/Techno/Body.h>

std::vector<std::unique_ptr<SquadManager>> SquadManager::Array;

SquadManager::SquadManager():
	Squad_Members { },
	isSelected { false }
{
	SquadManager::Array.emplace_back(this);
}

SquadManager::~SquadManager()
{ }

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

void SquadManager::RemoveGlobals(SquadManager* const pSquadManager) noexcept
{
	Array.erase(
		std::remove_if(Array.begin(), Array.end(),
			[&pSquadManager](const std::unique_ptr<SquadManager>& it) -> bool
			{
				return pSquadManager == it.get();
			}),
		Array.end()
	);
}

void SquadManager::PointerGotInvalid(void* ptr, bool removed)
{

}

void SquadManager::Remove(TechnoClass* const pTechno) noexcept
{
	auto pExt = TechnoExt::ExtMap.Find(pTechno);

	if (pExt->SquadManager)
	{
		pExt->SquadManager->RemoveTechno(pTechno);
		if (pExt->SquadManager->Squad_Members.size() == 1)
		{
			SquadManager::RemoveGlobals(pExt->SquadManager);
			pExt->SquadManager = nullptr;
		}
	}
}

#pragma region Save/Load

template <typename T>
bool SquadManager::Serialize(T& stm)
{
	return stm
		.Process(this->Squad_Members)
		.Process(this->isSelected)
		.Success();
		//.Success() && stm.RegisterChange(this);
};

bool SquadManager::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool SquadManager::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}

#pragma endregion

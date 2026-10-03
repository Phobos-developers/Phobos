#include "TypeConvertGroup.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Foot/Body.h>
#include <FactoryClass.h>
#include <SidebarClass.h>
#include <HouseClass.h>
#include <MouseClass.h>
#include <Utilities/AresFunctions.h>

#include <algorithm>
#include <cmath>

namespace
{
	FactoryClass* FindFactoryForObject(TechnoClass* pObject)
	{
		if (!pObject || !pObject->InLimbo)
			return nullptr;

		for (const auto pFactory : FactoryClass::Array)
		{
			if (pFactory->Object == pObject)
				return pFactory;
		}

		return nullptr;
	}

	bool IsCompatibleFactoryType(TechnoTypeClass* pToType, TechnoTypeClass* pFromType)
	{
		return pToType && pFromType
			&& pToType->WhatAmI() == pFromType->WhatAmI()
			&& pToType->Naval == pFromType->Naval;
	}

	// Resolves and invokes SidebarClass::Factory_Link to bind a factory with its corresponding cameo slot.
	bool Sidebar_Factory_Link(SidebarClass* pSidebar, FactoryClass* pFactory, AbstractType absType, int idxType)
	{
		auto const func = reinterpret_cast<bool(__thiscall*)(SidebarClass*, FactoryClass*, AbstractType, int)>(0x6A6140);
		return func(pSidebar, pFactory, absType, idxType);
	}

	// Synchronizes sidebar build cameos and production clock progress with the updated unit type.
	void SyncSidebarCameo(FactoryClass* pFactory, TechnoTypeClass* pFromType, TechnoTypeClass* pToType, int progress)
	{
		const int targetTabIdx = std::clamp(SidebarClass::GetObjectTabIdx(pToType->WhatAmI(), pToType->GetArrayIndex(), 0), 0, 3);

		if (AresFunctions::TabCameos)
		{
			AresBuildType* pOldCameo = nullptr;
			AresBuildType* pNewCameo = nullptr;

			for (int tabIdx = 0; tabIdx < 4; ++tabIdx)
			{
				auto& cameos = AresFunctions::TabCameos[tabIdx];

				for (int i = 0; i < cameos.Count; ++i)
				{
					if (cameos.Items[i].CurrentFactory == pFactory ||
						(pFromType && cameos.Items[i].ItemType == pFromType->WhatAmI() && cameos.Items[i].ItemIndex == pFromType->GetArrayIndex()))
					{
						pOldCameo = &cameos.Items[i];
					}

					if (cameos.Items[i].ItemType == pToType->WhatAmI() && cameos.Items[i].ItemIndex == pToType->GetArrayIndex())
						pNewCameo = &cameos.Items[i];
				}
			}

			if (pNewCameo)
			{
				// Transfer factory association and clock progress to the target cameo.
				pNewCameo->CurrentFactory = pFactory;
				pNewCameo->unknown_10 = 1;
				pNewCameo->Progress.Value = progress;
				pNewCameo->Progress.Start(pFactory->Production.Rate);

				if (pOldCameo && pOldCameo != pNewCameo)
				{
					pOldCameo->CurrentFactory = nullptr;
					pOldCameo->unknown_10 = 0;
					pOldCameo->Progress.Value = 0;
				}
			}
			else if (pOldCameo)
			{
				// Mutate the previous cameo entry in-place if the new cameo has not been registered yet.
				pOldCameo->ItemType = pToType->WhatAmI();
				pOldCameo->ItemIndex = pToType->GetArrayIndex();
				pOldCameo->CurrentFactory = pFactory;
				pOldCameo->unknown_10 = 1;
				pOldCameo->Progress.Value = progress;
				pOldCameo->Progress.Start(pFactory->Production.Rate);
			}
			else
			{
				// Insert a new cameo directly into the active Ares cameo vector.
				AresBuildType newCameo;
				newCameo.ItemIndex = pToType->GetArrayIndex();
				newCameo.ItemType = pToType->WhatAmI();
				newCameo.CurrentFactory = pFactory;
				newCameo.unknown_10 = 1;
				newCameo.Progress.Value = progress;
				newCameo.Progress.Start(pFactory->Production.Rate);
				AresFunctions::TabCameos[targetTabIdx].AddItem(newCameo);
			}

			// Update the active building indicator across all sidebar tabs.
			for (int tabIdx = 0; tabIdx < 4; ++tabIdx)
			{
				auto& cameos = AresFunctions::TabCameos[tabIdx];
				bool anyBuilding = false;

				for (int i = 0; i < cameos.Count; ++i)
				{
					if (cameos.Items[i].CurrentFactory != nullptr)
					{
						anyBuilding = true;
						break;
					}
				}

				SidebarClass::Instance.Tabs[tabIdx].IsBuilding = anyBuilding;
				SidebarClass::Instance.Tabs[tabIdx].NeedsRedraw = true;
			}
		}
		else
		{
			// Vanilla sidebar fallback when Ares dynamic cameo tables are not in use.
			SidebarClass::Instance.AddCameo(pToType->WhatAmI(), pToType->GetArrayIndex());
		}

		// Relink factory and mark the sidebar interface for redrawing.
		Sidebar_Factory_Link(&SidebarClass::Instance, pFactory, pToType->WhatAmI(), pToType->GetArrayIndex());
		SidebarClass::Instance.Tabs[targetTabIdx].IsBuilding = true;
		SidebarClass::Instance.Tabs[targetTabIdx].NeedsRedraw = true;
		SidebarClass::Instance.SidebarNeedsRepaint();
		MouseClass::Instance.RedrawSidebar(0);
	}

	// Migrates factory production and queued units to the target TechnoType.
	bool HandleFactoryConversion(FactoryClass* pFactory, HouseClass* pHouse, TechnoTypeClass* pFromType, TechnoTypeClass* pToType)
	{
		if (!pFactory || !pHouse || !pToType)
			return false;

		bool convertedActiveObject = false;
		bool convertedQueuedObjects = false;
		bool sidebarChanged = false;
		int currentProgress = 0;

		// Update queued build orders to the target type, refunding incompatible units.
		for (int q = pFactory->QueuedObjects.Count - 1; q >= 0; --q)
		{
			const auto pQueuedType = pFactory->QueuedObjects[q];

			if (pFromType == nullptr || pQueuedType == pFromType)
			{
				if (IsCompatibleFactoryType(pToType, pQueuedType))
				{
					pFactory->QueuedObjects[q] = pToType;
					convertedQueuedObjects = true;
				}
				else
				{
					pFactory->QueuedObjects.RemoveItem(q);
					pHouse->GiveMoney(pQueuedType->GetActualCost(pHouse));
				}

				sidebarChanged = true;
			}
		}

		// Convert active unit currently in production and adjust remaining cost balances.
		if (pFactory->Object)
		{
			const auto pFoot = abstract_cast<FootClass*>(pFactory->Object);
			const auto pCurrentType = pFoot ? pFoot->GetTechnoType() : nullptr;

			if (pCurrentType && (pFromType == nullptr || pCurrentType == pFromType))
			{
				if (IsCompatibleFactoryType(pToType, pCurrentType))
				{
					currentProgress = std::clamp(pFactory->GetProgress(), 0, 54);
					const int oldCost = pCurrentType->GetActualCost(pHouse);
					const int oldPaid = std::clamp(oldCost - pFactory->Balance, 0, oldCost);
					const int newTotalCost = pToType->GetActualCost(pHouse);
					int newBalance = newTotalCost - oldPaid;

					if (newBalance < 0)
					{
						pHouse->GiveMoney(-newBalance);
						newBalance = 0;
					}

					TechnoExt::ConvertToType(pFoot, pToType);

					pFactory->Balance = newBalance;
					pFactory->OriginalBalance = newTotalCost;

					if (pFactory->IsSuspended && pFactory->OnHold)
					{
						pFactory->Production.Rate = 0;
						pFactory->Production.Value = currentProgress;
						pFactory->IsDifferent = true;
					}
					else
					{
						pFactory->Production.Start(pFactory->GetBuildTimeFrames());
						pFactory->Production.Value = currentProgress;
						pFactory->IsDifferent = true;
					}

					convertedActiveObject = true;
					sidebarChanged = true;
				}
				else
				{
					pFactory->AbandonProduction();
					sidebarChanged = true;
				}
			}
		}

		// Resume factory production if the active unit was discarded but queued orders remain.
		if (!pFactory->Object && pFactory->QueuedObjects.Count > 0)
		{
			pFactory->StartProduction();
			convertedActiveObject = true;
			sidebarChanged = true;
		}

		// Synchronize UI and tech tree for the local human player.
		if (pHouse->IsCurrentPlayer() && sidebarChanged)
		{
			pHouse->RecheckTechTree = true;
			SyncSidebarCameo(pFactory, pFromType, pToType, currentProgress);
		}

		return convertedActiveObject || convertedQueuedObjects;
	}
}

void TypeConvertGroup::Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses] : convertPairs)
	{
		if (!toType.Get())
			continue;

		if (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner))
			continue;

		if (fromTypes.size())
		{
			const auto pType = pTargetFoot->GetTechnoType();

			for (const auto& from : fromTypes)
			{
				if (from == pType)
				{
					if (pTargetFoot->InLimbo)
					{
						if (const auto pFactory = FindFactoryForObject(pTargetFoot))
						{
							HandleFactoryConversion(pFactory, pTargetFoot->Owner, from, toType);
							return;
						}
					}

					TechnoExt::ConvertToType(pTargetFoot, toType);
					return;
				}
			}
		}
		else
		{
			if (pTargetFoot->InLimbo)
			{
				if (const auto pFactory = FindFactoryForObject(pTargetFoot))
				{
					HandleFactoryConversion(pFactory, pTargetFoot->Owner, pTargetFoot->GetTechnoType(), toType);
					return;
				}
			}

			TechnoExt::ConvertToType(pTargetFoot, toType);
			return;
		}
	}
}

void TypeConvertGroup::ConvertSW(const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses] : convertPairs)
	{
		if (!toType.Get())
			continue;

		// Update factory production lines and queued build orders for affected houses.
		std::vector<FactoryClass*> factories(FactoryClass::Array.Count);
		std::copy(FactoryClass::Array.begin(), FactoryClass::Array.end(), factories.begin());

		for (const auto pHouse : HouseClass::Array)
		{
			if (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pHouse))
				continue;

			for (const auto pFactory : factories)
			{
				if (pFactory->Owner != pHouse)
					continue;

				if (fromTypes.size())
				{
					for (const auto& from : fromTypes)
					{
						HandleFactoryConversion(pFactory, pHouse, from, toType);
					}
				}
				else
				{
					HandleFactoryConversion(pFactory, pHouse, nullptr, toType);
				}
			}
		}

		// Convert existing objects deployed in the game world.
		if (fromTypes.size())
		{
			auto copy_dvc = []<typename T>(const DynamicVectorClass<T>& dvc)
			{
				std::vector<T> vec(dvc.Count);
				std::copy(dvc.begin(), dvc.end(), vec.begin());
				return vec;
			};

			for (const auto& from : fromTypes)
			{
				const auto items = copy_dvc(TechnoTypeExt::Fetch(from)->Array);

				for (const auto pTarget : items)
				{
					const auto pTargetFoot = abstract_cast<FootClass*, true>(pTarget);

					if (!pTargetFoot || pTargetFoot->InLimbo || (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner)))
						continue;

					TechnoExt::ConvertToType(pTargetFoot, toType);
				}
			}
		}
		else
		{
			for (const auto pTargetFoot : FootClass::Array)
			{
				if (pTargetFoot->InLimbo)
					continue;

				TypeConvertGroup::Convert(pTargetFoot, convertPairs, pOwner);
			}
		}
	}

	return;
}


bool TypeConvertGroup::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TypeConvertGroup::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TypeConvertGroup*>(this)->Serialize(stm);
}

void TypeConvertGroup::Parse(std::vector<TypeConvertGroup>& list, INI_EX& exINI, const char* pSection, AffectedHouse defaultAffectHouse)
{
	for (size_t i = 0; ; ++i)
	{
		char tempBuffer[32];
		ValueableVector<TechnoTypeClass*> convertFrom;
		Nullable<TechnoTypeClass*> convertTo;
		Nullable<AffectedHouse> convertAffectsHouse;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Convert%d.From", i);
		convertFrom.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Convert%d.To", i);
		convertTo.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Convert%d.AffectedHouses", i); // Temporary solution for the INI tags renaming issue, see #2093
		convertAffectsHouse.Read(exINI, pSection, tempBuffer);
		if (convertAffectsHouse.isset())
		{
			Debug::Log("[Developer warning][%s] %s is deprecated and has been replaced by Convert%d.AffectsHouse! If both are set, the latter will be used.\n",
				pSection, tempBuffer, i);
		}
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Convert%d.AffectsHouse", i);
		convertAffectsHouse.Read(exINI, pSection, tempBuffer);

		if (!convertTo.isset())
			break;

		if (!convertAffectsHouse.isset())
			convertAffectsHouse = defaultAffectHouse;

		list.emplace_back(convertFrom, convertTo, convertAffectsHouse);
	}
	ValueableVector<TechnoTypeClass*> convertFrom;
	Nullable<TechnoTypeClass*> convertTo;
	Nullable<AffectedHouse> convertAffectsHouse;
	convertFrom.Read(exINI, pSection, "Convert.From");
	convertTo.Read(exINI, pSection, "Convert.To");
	convertAffectsHouse.Read(exINI, pSection, "Convert.AffectedHouses"); // Temporary solution for the INI tags renaming issue, see #2093
	if (convertAffectsHouse.isset())
	{
		Debug::Log("[Developer warning][%s] Convert.AffectedHouses is deprecated and has been replaced by Convert.AffectsHouse! If both are set, the latter will be used.\n", pSection);
	}
	convertAffectsHouse.Read(exINI, pSection, "Convert.AffectsHouse");
	if (convertTo.isset())
	{
		if (!convertAffectsHouse.isset())
			convertAffectsHouse = defaultAffectHouse;

		if (list.size())
			list[0] = { convertFrom, convertTo, convertAffectsHouse };
		else
			list.emplace_back(convertFrom, convertTo, convertAffectsHouse);
	}
}

template <typename T>
bool TypeConvertGroup::Serialize(T& stm)
{
	return stm
		.Process(this->FromTypes)
		.Process(this->ToType)
		.Process(this->AppliedTo)
		.Success();
}

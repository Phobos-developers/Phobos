#include "Body.h"

template<> const DWORD Extension<BuildingClass>::Canary = 0x87654321;
BuildingExt::ExtContainer BuildingExt::ExtMap;

void BuildingExt::UpdatePrimaryFactoryAI(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto BuildingExt = BuildingExt::ExtMap.Find(pThis);
	if (!BuildingExt
		|| pOwner->Type->MultiplayPassive
		|| pOwner->IsNeutral())
	{
		return;
	}

	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->GetItem(pOwner->ProducingAircraftTypeIndex);
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	DynamicVectorClass<BuildingClass*> airFactoryBuilding;
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (BuildingExt->CurrentAirFactory)
	{
		int nDocks = BuildingExt->CurrentAirFactory->Type->NumberOfDocks;
		int nOccupiedDocks = CountOccupiedDocks(BuildingExt->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = BuildingExt->CurrentAirFactory->Factory;
		else
			BuildingExt->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto pBuilding : pOwner->Buildings)
	{
		if (pBuilding->Type->Factory == AbstractType::AircraftType)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.AddItem(pBuilding);
		}
	}

	if (BuildingExt->CurrentAirFactory)
	{
		for (auto pBuilding : airFactoryBuilding)
		{
			if (pBuilding == BuildingExt->CurrentAirFactory)
			{
				BuildingExt->CurrentAirFactory->Factory = currFactory;
				BuildingExt->CurrentAirFactory->IsPrimaryFactory = true;
			}
			else
			{
				pBuilding->IsPrimaryFactory = false;

				if (pBuilding->Factory)
					pBuilding->Factory->AbandonProduction();
			}
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto pBuilding : airFactoryBuilding)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
		{
			if (!newBuilding)
			{
				newBuilding = pBuilding;
				newBuilding->Factory = currFactory;
				newBuilding->IsPrimaryFactory = true;
				BuildingExt->CurrentAirFactory = newBuilding;

				continue;
			}
		}

		pBuilding->IsPrimaryFactory = false;

		if (pBuilding->Factory)
			pBuilding->Factory->AbandonProduction();
	}

	return;
}

int BuildingExt::CountOccupiedDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return 0;

	int nOccupiedDocks = 0;

	for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
	{
		if (auto const pLink = pBuilding->GetNthLink(i))
		{
			nOccupiedDocks++;
		}
	}

	return nOccupiedDocks;
}

bool BuildingExt::HasFreeDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return false;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
			return true;
		else
			return false;
	}

	return false;
}

// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		;
}

void BuildingExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BuildingExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }

BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x45417E, BuildingClass_Load_Suffix, 0x5)
{
	BuildingExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x454244, BuildingClass_Save_Suffix, 0x7)
{
	BuildingExt::ExtMap.SaveStatic();

	return 0;
}

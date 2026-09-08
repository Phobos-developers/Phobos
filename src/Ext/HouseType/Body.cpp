#include "Body.h"

#include <Ext/Side/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Scenario/Body.h>

HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::Initialize() { }

void HouseTypeExt::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->EVATag.Read(pINI, pSection, "EVA.Tag");

	this->DropshipLoadout_StartingDropships.Read(exINI, pSection, "DropshipLoadout.StartingDropships");
	this->DropshipLoadout_AllowableUnits.Read(exINI, pSection, "DropshipLoadout.AllowableUnits");
	this->DropshipLoadout_AllowableUnitMaximums.Read(exINI, pSection, "DropshipLoadout.AllowableUnitMaximums");

	this->DropshipLoadout_AllowableUnitsLists.clear();
	this->DropshipLoadout_AllowableUnitMaximumsLists.clear();

	std::vector<int> parsedIndices;
	parsedIndices.push_back(0);

	const int keyCount = pINI->GetKeyCount(pSection);

	for (int k = 0; k < keyCount; ++k)
	{
		const char* pKeyName = pINI->GetKeyName(pSection, k);
		int idx = -1;

		if (sscanf_s(pKeyName, "DropshipLoadout.AllowableUnits%d", &idx) == 1)
		{
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.AllowableUnits%d", idx);
			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (std::find(parsedIndices.begin(), parsedIndices.end(), idx) == parsedIndices.end())
					parsedIndices.push_back(idx);
			}
		}
		else if (sscanf_s(pKeyName, "DropshipLoadout.AllowableUnitMaximums%d", &idx) == 1)
		{
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.AllowableUnitMaximums%d", idx);
			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (std::find(parsedIndices.begin(), parsedIndices.end(), idx) == parsedIndices.end())
					parsedIndices.push_back(idx);
			}
		}
	}

	for (int i : parsedIndices)
	{
		char tempBuffer[256];
		bool unitsSet = false;
		bool maxSet = false;
		std::vector<TechnoTypeClass*> unitsList;
		std::vector<int> maxList;

		if (i == 0)
		{
			// Start with default/legacy parsed values
			if (this->DropshipLoadout_AllowableUnits.size() > 0)
			{
				for (auto pUnit : this->DropshipLoadout_AllowableUnits)
				{
					unitsList.push_back(pUnit);
				}

				unitsSet = true;
			}

			if (this->DropshipLoadout_AllowableUnitMaximums.size() > 0)
			{
				for (int pUnitCount : this->DropshipLoadout_AllowableUnitMaximums)
				{
					maxList.push_back(pUnitCount);
				}

				maxSet = true;
			}

			// Override with AllowableUnits0
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.AllowableUnits0");

			if (pINI->ReadString(pSection, tempBuffer, "", Phobos::readBuffer) > 0)
			{
				ValueableVector<TechnoTypeClass*> tempUnits;
				tempUnits.Read(exINI, pSection, tempBuffer);
				unitsList.clear();

				for (auto pUnit : tempUnits)
				{
					unitsList.push_back(pUnit);
				}

				unitsSet = true;
			}

			// Override with AllowableUnitMaximums0
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.AllowableUnitMaximums0");

			if (pINI->ReadString(pSection, tempBuffer, "", Phobos::readBuffer) > 0)
			{
				ValueableVector<int> tempMaximums;
				tempMaximums.Read(exINI, pSection, tempBuffer);
				maxList.clear();

				for (auto pUnitCount : tempMaximums)
				{
					maxList.push_back(pUnitCount);
				}

				maxSet = true;
			}
		}
		else
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.AllowableUnits%d", i);

			if (pINI->ReadString(pSection, tempBuffer, "", Phobos::readBuffer) > 0)
			{
				ValueableVector<TechnoTypeClass*> tempUnits;
				tempUnits.Read(exINI, pSection, tempBuffer);

				for (auto pUnit : tempUnits)
				{
					unitsList.push_back(pUnit);
				}

				unitsSet = true;
			}

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.AllowableUnitMaximums%d", i);

			if (pINI->ReadString(pSection, tempBuffer, "", Phobos::readBuffer) > 0)
			{
				ValueableVector<int> tempMaximums;
				tempMaximums.Read(exINI, pSection, tempBuffer);

				for (auto pUnitCount : tempMaximums)
				{
					maxList.push_back(pUnitCount);
				}

				maxSet = true;
			}
		}

		if (unitsSet)
			this->DropshipLoadout_AllowableUnitsLists[i] = std::move(unitsList);

		if (maxSet)
			this->DropshipLoadout_AllowableUnitMaximumsLists[i] = std::move(maxList);
	}

	if (pINI->ReadString(pSection, "DropshipLoadout.Theme", "", Phobos::readBuffer) > 0)
		this->DropshipLoadout_Theme = pINI->ReadTheme(pSection, "DropshipLoadout.Theme", -1);

	this->DropshipLoadout_Money.Read(exINI, pSection, "DropshipLoadout.Money");
	this->DropshipLoadout_StartEVA.Read(exINI, pSection, "DropshipLoadout.StartEVA");
	this->DropshipLoadout_Carriers.Read(exINI, pSection, "DropshipLoadout.Carriers");
	this->DropshipLoadout_Carriers_SizeLimit.Read(exINI, pSection, "DropshipLoadout.Carriers.SizeLimit");
	this->DropshipLoadout_AddUnusedMoneyToPlayer.Read(exINI, pSection, "DropshipLoadout.AddUnusedMoneyToPlayer");
	this->DropshipLoadout_RememberPurchasedCargo.Read(exINI, pSection, "DropshipLoadout.RememberPurchasedCargo");

	if (pINI->ReadString(pSection, "DropshipLoadout.Palette", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_Palette = FileSystem::LoadPALFile(Phobos::readBuffer, DSurface::Hidden);

	int nStartingDropships = this->DropshipLoadout_StartingDropships.isset() ? this->DropshipLoadout_StartingDropships.Get() : (ScenarioExt::Global() ? ScenarioExt::Global()->DropshipLoadout_StartingDropships : 1);

	if (pINI->ReadString(pSection, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) != 0)
	{
		this->DropshipLoadout_BackgroundPCXPattern = Phobos::readBuffer; // keep raw pattern for runtime formatting
		char filename[260];
		_snprintf_s(filename, sizeof(filename), Phobos::readBuffer, nStartingDropships);
		this->DropshipLoadout_BackgroundPCX = PhobosPCXFile(_strdup(filename));
	}

	pINI->ReadString(pSection, "DropshipLoadout.UpArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_UpArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(pSection, "DropshipLoadout.DownArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_DownArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(pSection, "DropshipLoadout.LoadoutPCX", "", Phobos::readBuffer);
	auto  loadoutFramesPCX = GeneralUtils::GetAnimationPCX(Phobos::readBuffer);

	if (loadoutFramesPCX)
	{
		for (auto& frame : *loadoutFramesPCX)
		{
			this->DropshipLoadout_LoadoutPCX.emplace_back(std::move(frame));
		}
	}

	pINI->ReadString(pSection, "DropshipLoadout.PilotLitPCX", "", Phobos::readBuffer);
	auto  pilotlitFramesPCX = GeneralUtils::GetAnimationPCX(Phobos::readBuffer);

	if (pilotlitFramesPCX)
	{
		for (auto& frame : *pilotlitFramesPCX)
		{
			this->DropshipLoadout_PilotLitPCX.emplace_back(std::move(frame));
		}
	}

	// Sidebar click animations list (the animation that appears in the sidebar when a cameo is clicked)
	char* context = nullptr;
	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenListPCX", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_DGreenListPCX.clear();

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			this->DropshipLoadout_DGreenListPCX.emplace_back(GeneralUtils::GetAnimationPCX(cur));
		}
	}

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenAnimationsCount", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");
		this->DropshipLoadout_DGreenLocations.clear();

		for (int i = 0; i < this->DropshipLoadout_DGreenAnimationsCount.Get(0); i++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.DGreenLocation%d", i);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			this->DropshipLoadout_DGreenLocations.push_back(location);
		}
	}
	else
	{
		this->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");
	}

	// Custom Dropship Loadout coordinates
	this->DropshipLoadout_LoadoutLocation.Read(exINI, pSection, "DropshipLoadout.LoadoutLocation");
	this->DropshipLoadout_PilotLitLocation.Read(exINI, pSection, "DropshipLoadout.PilotLitLocation");
	this->DropshipLoadout_UpArrowLocation.Read(exINI, pSection, "DropshipLoadout.UpArrowLocation");
	this->DropshipLoadout_DownArrowLocation.Read(exINI, pSection, "DropshipLoadout.DownArrowLocation");

	if (pINI->ReadString(pSection, "DropshipLoadout.SidebarCameosCount", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");
		this->DropshipLoadout_SidebarCameoLocations.clear();

		for (int i = 0; i < this->DropshipLoadout_SidebarCameosCount; i++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.SidebarCameoLocation%d", i);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			this->DropshipLoadout_SidebarCameoLocations.push_back(location);
		}
	}
	else
	{
		this->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");
	}

	this->DropshipLoadout_DropshipCameosCount.Read(exINI, pSection, "DropshipLoadout.DropshipCameosCount");

	bool hasDropshipCameosConfig = false;

	if (pINI->ReadString(pSection, "DropshipLoadout.DropshipCameosCount", "", Phobos::readBuffer) > 0)
		hasDropshipCameosConfig = true;

	int maxDropshipIdx = -1;

	for (int k = 0; k < keyCount; ++k)
	{
		const char* pKeyName = pINI->GetKeyName(pSection, k);
		int dropshipIdx = -1;
		int cameoIdx = -1;

		if (sscanf_s(pKeyName, "DropshipLoadout.Dropship%d.CameoLocation%d", &dropshipIdx, &cameoIdx) == 2)
		{
			hasDropshipCameosConfig = true;
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.Dropship%d.CameoLocation%d", dropshipIdx, cameoIdx);

			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (dropshipIdx > maxDropshipIdx)
					maxDropshipIdx = dropshipIdx;
			}
		}
		else if (sscanf_s(pKeyName, "DropshipLoadout.Dropship%d.CameosCount", &dropshipIdx) == 1)
		{
			hasDropshipCameosConfig = true;
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.Dropship%d.CameosCount", dropshipIdx);

			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (dropshipIdx > maxDropshipIdx)
					maxDropshipIdx = dropshipIdx;
			}
		}
	}
	if (hasDropshipCameosConfig)
	{
		this->DropshipLoadout_DropshipCameoLocations.clear();

		int limit = nStartingDropships;

		if (maxDropshipIdx + 1 > limit)
			limit = maxDropshipIdx + 1;

		if (limit > 0)
		{
			for (int i = 0; i < limit; i++)
			{
				char countKey[256];
				_snprintf_s(countKey, sizeof(countKey), "DropshipLoadout.Dropship%d.CameosCount", i);
				int defaultCount = this->DropshipLoadout_DropshipCameosCount.Get(0) > 0 ? this->DropshipLoadout_DropshipCameosCount.Get(0) : 5;
				int cameosCount = pINI->ReadInteger(pSection, countKey, defaultCount);

				if (cameosCount < 0)
					cameosCount = 0;

				auto& locations = this->DropshipLoadout_DropshipCameoLocations.emplace_back();

				for (int j = 0; j < cameosCount; j++)
				{
					char tempBuffer[256];
					Point2D location = Point2D::Empty;

					_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.Dropship%d.CameoLocation%d", i, j);
					pINI->ReadPoint2D(location, pSection, tempBuffer, location);
					locations.push_back(location);
				}
			}
		}
	}

	this->DropshipLoadout_FixedUnits.clear();
	std::map<int, std::vector<TechnoTypeClass*>> parsedFixedUnits;
	int maxIdx = -1;

	for (int k = 0; k < keyCount; ++k)
	{
		const char* pKeyName = pINI->GetKeyName(pSection, k);
		int dropshipIdx = -1;

		if (sscanf_s(pKeyName, "DropshipLoadout.Dropship%d.FixedUnits", &dropshipIdx) == 1)
		{
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.Dropship%d.FixedUnits", dropshipIdx);

			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (pINI->ReadString(pSection, pKeyName, "", Phobos::readBuffer) > 0)
				{
					char* ctx = nullptr;
					std::vector<TechnoTypeClass*> list;

					for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &ctx); cur; cur = strtok_s(nullptr, Phobos::readDelims, &ctx))
					{
						if (auto pType = TechnoTypeClass::Find(cur))
							list.push_back(pType);
					}

					parsedFixedUnits[dropshipIdx] = std::move(list);

					if (dropshipIdx > maxIdx)
						maxIdx = dropshipIdx;
				}
			}
		}
	}

	if (maxIdx != -1)
	{
		this->DropshipLoadout_FixedUnits.resize(maxIdx + 1);

		for (auto& [idx, list] : parsedFixedUnits)
		{
			this->DropshipLoadout_FixedUnits[idx] = std::move(list);
		}
	}

	this->DropshipLoadout_InitialUnits.clear();
	std::map<int, std::vector<TechnoTypeClass*>> parsedInitialUnits;
	maxIdx = -1;

	for (int k = 0; k < keyCount; ++k)
	{
		const char* pKeyName = pINI->GetKeyName(pSection, k);
		int dropshipIdx = -1;

		if (sscanf_s(pKeyName, "DropshipLoadout.Dropship%d.InitialUnits", &dropshipIdx) == 1)
		{
			char expectedKey[256];
			_snprintf_s(expectedKey, sizeof(expectedKey), "DropshipLoadout.Dropship%d.InitialUnits", dropshipIdx);

			if (strcmp(pKeyName, expectedKey) == 0)
			{
				if (pINI->ReadString(pSection, pKeyName, "", Phobos::readBuffer) > 0)
				{
					char* ctx = nullptr;
					std::vector<TechnoTypeClass*> list;

					for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &ctx); cur; cur = strtok_s(nullptr, Phobos::readDelims, &ctx))
					{
						if (auto pType = TechnoTypeClass::Find(cur))
							list.push_back(pType);
					}

					parsedInitialUnits[dropshipIdx] = std::move(list);

					if (dropshipIdx > maxIdx)
						maxIdx = dropshipIdx;
				}
			}
		}
	}

	if (maxIdx != -1)
	{
		this->DropshipLoadout_InitialUnits.resize(maxIdx + 1);

		for (auto& [idx, list] : parsedInitialUnits)
		{
			this->DropshipLoadout_InitialUnits[idx] = std::move(list);
		}
	}

	this->DropshipLoadout_BuyClickSound.Read(exINI, pSection, "DropshipLoadout.BuyClickSound");
	this->DropshipLoadout_SellClickSound.Read(exINI, pSection, "DropshipLoadout.SellClickSound");
	this->DropshipLoadout_ArrowsClickSound.Read(exINI, pSection, "DropshipLoadout.ArrowsClickSound");
	this->DropshipLoadout_StartingDragDropSound.Read(exINI, pSection, "DropshipLoadout.StartingDragDropSound");
	this->DropshipLoadout_EndingDragDropSound.Read(exINI, pSection, "DropshipLoadout.EndingDragDropSound");
}

template <typename T>
void HouseTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->EVATag)
		.Process(this->DropshipLoadout_StartingDropships)
		.Process(this->DropshipLoadout_AllowableUnits)
		.Process(this->DropshipLoadout_AllowableUnitMaximums)
		.Process(this->DropshipLoadout_Theme)
		.Process(this->DropshipLoadout_Money)
		.Process(this->DropshipLoadout_StartEVA)
		.Process(this->DropshipLoadout_Carriers)
		.Process(this->DropshipLoadout_Carriers_SizeLimit)
		.Process(this->DropshipLoadout_AddUnusedMoneyToPlayer)
		.Process(this->DropshipLoadout_RememberPurchasedCargo)
		.Process(this->DropshipLoadout_Palette)
		.Process(this->DropshipLoadout_BackgroundPCX)
		.Process(this->DropshipLoadout_UpArrowPCX)
		.Process(this->DropshipLoadout_DownArrowPCX)
		.Process(this->DropshipLoadout_LoadoutPCX)
		.Process(this->DropshipLoadout_LoadoutLocation)
		.Process(this->DropshipLoadout_PilotLitPCX)
		.Process(this->DropshipLoadout_PilotLitLocation)
		.Process(this->DropshipLoadout_DGreenListPCX)
		.Process(this->DropshipLoadout_DGreenAnimationsCount)
		.Process(this->DropshipLoadout_DGreenLocations)
		.Process(this->DropshipLoadout_UpArrowLocation)
		.Process(this->DropshipLoadout_DownArrowLocation)
		.Process(this->DropshipLoadout_SidebarCameosCount)
		.Process(this->DropshipLoadout_SidebarCameoLocations)
		.Process(this->DropshipLoadout_DropshipCameosCount)
		.Process(this->DropshipLoadout_DropshipCameoLocations)
		.Process(this->DropshipLoadout_BuyClickSound)
		.Process(this->DropshipLoadout_SellClickSound)
		.Process(this->DropshipLoadout_ArrowsClickSound)
		.Process(this->DropshipLoadout_StartingDragDropSound)
		.Process(this->DropshipLoadout_EndingDragDropSound)
		.Process(this->DropshipLoadout_AllowableUnitsLists)
		.Process(this->DropshipLoadout_AllowableUnitMaximumsLists)
		;

	int numDropships = (int)this->DropshipLoadout_FixedUnits.size();
	Stm.Process(numDropships);

	if constexpr (std::is_same_v<T, PhobosStreamReader>)
		this->DropshipLoadout_FixedUnits.resize(numDropships);

	for (int i = 0; i < numDropships; ++i)
	{
		Stm.Process(this->DropshipLoadout_FixedUnits[i]);
	}

	int numInitialDropships = (int)this->DropshipLoadout_InitialUnits.size();
	Stm.Process(numInitialDropships);

	if constexpr (std::is_same_v<T, PhobosStreamReader>)
		this->DropshipLoadout_InitialUnits.resize(numInitialDropships);

	for (int i = 0; i < numInitialDropships; ++i)
	{
		Stm.Process(this->DropshipLoadout_InitialUnits[i]);
	}
}

void HouseTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") { }
HouseTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x511635, HouseTypeClass_CTOR_1, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x51215A, HouseTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x51214F, HouseTypeClass_LoadFromINI, 0x5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

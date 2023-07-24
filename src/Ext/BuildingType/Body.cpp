#include "Body.h"

#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	for (const auto pair : pHouseExt->BuildingCounter)
	{
		const auto& pExt = pair.first;
		const auto& nCount = pair.second;
		if (pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
		{
			fFactor *= std::pow(pExt->PowerPlantEnhancer_Factor.Get(1.0f), nCount);
			nAmount += pExt->PowerPlantEnhancer_Amount.Get(0) * nCount;
		}
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBuilding)
						++result;
				}
			}
		}
	};

	if (pPowersUp[0])
	{
		if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			checkUpgrade(pTPowersUp);
	}

	if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
	{
		for (auto pTPowersUp : pBuildingExt->PowersUp_Buildings)
			checkUpgrade(pTPowersUp);
	}

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::Initialize()
{
}

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");
	this->PowerPlantEnhancer_Buildings.Read(exINI, pSection, "PowerPlantEnhancer.PowerPlants");
	this->PowerPlantEnhancer_Amount.Read(exINI, pSection, "PowerPlantEnhancer.Amount");
	this->PowerPlantEnhancer_Factor.Read(exINI, pSection, "PowerPlantEnhancer.Factor");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
		strcpy_s(pThis->PowersUpBuilding, this->PowersUp_Buildings[0]->ID);

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_Weapon.Read(exINI, pSection, "Grinding.Weapon", true);
	this->Grinding_DisplayRefund.Read(exINI, pSection, "Grinding.DisplayRefund");
	this->Grinding_DisplayRefund_Houses.Read(exINI, pSection, "Grinding.DisplayRefund.Houses");
	this->Grinding_DisplayRefund_Offset.Read(exINI, pSection, "Grinding.DisplayRefund.Offset");


	// Ares SuperWeapons tag
	pINI->ReadString(pSection, "SuperWeapons", "", Phobos::readBuffer);
	//char* super_weapons_list = Phobos::readBuffer;
	if (strlen(Phobos::readBuffer) > 0 && SuperWeaponTypeClass::Array->Count > 0)
	{
		//DynamicVectorClass<SuperWeaponTypeClass*> objectsList;
		char* context = nullptr;

		//pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			SuperWeaponTypeClass* buffer;
			if (Parser<SuperWeaponTypeClass*>::TryParse(cur, &buffer))
			{
				//Debug::Log("DEBUG: [%s]: Parsed SW [%s]\n", pSection, cur);
				this->SuperWeapons.AddItem(buffer);
			}
			else
			{
				Debug::Log("DEBUG: [%s]: Error parsing SuperWeapons= [%s]\n", pSection, cur);
			}
		}
	}

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempBuffer[32];
		this->OccupierMuzzleFlashes.clear();
		this->OccupierMuzzleFlashes.resize(pThis->MaxNumberOccupants);

		for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
		{
			Nullable<Point2D> nMuzzleLocation;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "MuzzleFlash%d", i);
			nMuzzleLocation.Read(exArtINI, pArtSection, tempBuffer);
			this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get(Point2D::Empty);
		}
	}

	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");

	// PlacementPreview
	{
		this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
		this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
		this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
		this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
		this->PlacementPreview_Palette.LoadFromINI(pINI, pSection, "PlacementPreview.Palette");
		this->PlacementPreview_Translucency.Read(exINI, pSection, "PlacementPreview.Translucency");
	}
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		.Process(this->PowerPlantEnhancer_Buildings)
		.Process(this->PowerPlantEnhancer_Amount)
		.Process(this->PowerPlantEnhancer_Factor)
		.Process(this->SuperWeapons)
		.Process(this->OccupierMuzzleFlashes)
		.Process(this->Powered_KillSpawns)
		.Process(this->InitialStrength_Cloning)
		.Process(this->Refinery_UseStorage)
		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_DisplayRefund)
		.Process(this->Grinding_DisplayRefund_Houses)
		.Process(this->Grinding_DisplayRefund_Offset)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Shape)
		.Process(this->PlacementPreview_ShapeFrame)
		.Process(this->PlacementPreview_Offset)
		.Process(this->PlacementPreview_Remap)
		.Process(this->PlacementPreview_Palette)
		.Process(this->PlacementPreview_Translucency)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{


	return Stm.Success();
}
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x465300, BuildingTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4652ED, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46536A, BuildingTypeClass_Save_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

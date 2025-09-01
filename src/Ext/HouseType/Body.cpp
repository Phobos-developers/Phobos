#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Scenario/Body.h>

static constexpr DWORD Canary = 0x1111111A;
HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::ExtData::Initialize()
{
}

// =============================
// load / save

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	// Custom Dropship loadout images, in PCX format
	this->DropshipLoadout_StartingDropships.Read(exINI, pSection, "DropshipLoadout.StartingDropships");
	this->DropshipLoadout_AllowableUnits.Read(exINI, pSection, "DropshipLoadout.AllowableUnits");
	this->DropshipLoadout_AllowableUnitMaximums.Read(exINI, pSection, "DropshipLoadout.AllowableUnitMaximums");
	//this->DropshipLoadout_Theme.Read(exINI, pSection, "DropshipLoadout.Theme");
	if (pINI->ReadTheme(pSection, "DropshipLoadout.Theme", this->DropshipLoadout_Theme))
		this->DropshipLoadout_Theme = pINI->ReadTheme(pSection, "DropshipLoadout.Theme", this->DropshipLoadout_Theme);

	this->DropshipLoadout_Money.Read(exINI, pSection, "DropshipLoadout.Money");
	this->DropshipLoadout_StartEVA.Read(exINI, pSection, "DropshipLoadout.StartEVA");
	this->DropshipLoadout_Carriers.Read(exINI, pSection, "DropshipLoadout.Carriers");

	int nStartingDropships = this->DropshipLoadout_StartingDropships.isset() ? this->DropshipLoadout_StartingDropships : ScenarioClass::Instance->StartingDropships;

	if (pINI->ReadString(pSection, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) != 0)
	{
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
	pINI->ReadString(pSection, "DropshipLoadout.DGreenListPCX", "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		this->DropshipLoadout_DGreenListPCX.emplace_back(GeneralUtils::GetAnimationPCX(cur));
	}

	this->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");

	for (int i = 0; i < this->DropshipLoadout_DGreenAnimationsCount.Get(0); i++)
	{
		char tempBuffer[256];
		Point2D location = Point2D::Empty;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.DGreenLocation%d", i);
		pINI->ReadPoint2D(location, pSection, tempBuffer, location);
		this->DropshipLoadout_DGreenLocations.push_back(location);
	}

	// Custom Dropship Loadout coordinates
	this->DropshipLoadout_LoadoutLocation.Read(exINI, pSection, "DropshipLoadout.LoadoutLocation");
	this->DropshipLoadout_PilotLitLocation.Read(exINI, pSection, "DropshipLoadout.PilotLitLocation");
	this->DropshipLoadout_UpArrowLocation.Read(exINI, pSection, "DropshipLoadout.UpArrowLocation");
	this->DropshipLoadout_DownArrowLocation.Read(exINI, pSection, "DropshipLoadout.DownArrowLocation");

	this->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");

	for (int i = 0; i < this->DropshipLoadout_SidebarCameosCount; i++)
	{
		char tempBuffer[256];
		Point2D location = Point2D::Empty;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.SidebarCameoLocation%d", i);
		pINI->ReadPoint2D(location, pSection, tempBuffer, location);
		this->DropshipLoadout_SidebarCameoLocations.push_back(location);
	}

	this->DropshipLoadout_DropshipCameosCount.Read(exINI, pSection, "DropshipLoadout.DropshipCameosCount");

	for (int i = 0; i < nStartingDropships; i++)
	{
		std::vector<Point2D> locations;

		for (int j = 0; j < this->DropshipLoadout_DropshipCameosCount; j++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.Dropship%d.CameoLocation%d", i, j);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			locations.push_back(location);
		}

		this->DropshipLoadout_DropshipCameoLocations.push_back(locations);
	}

	this->DropshipLoadout_BuyClickSound.Read(exINI, pSection, "DropshipLoadout.BuyClickSound");
	this->DropshipLoadout_SellClickSound.Read(exINI, pSection, "DropshipLoadout.SellClickSound");
	this->DropshipLoadout_ArrowsClickSound.Read(exINI, pSection, "DropshipLoadout.ArrowsClickSound");
}

void HouseTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void HouseTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DropshipLoadout_StartingDropships)
		.Process(this->DropshipLoadout_AllowableUnits)
		.Process(this->DropshipLoadout_AllowableUnitMaximums)
		.Process(this->DropshipLoadout_Theme)
		.Process(this->DropshipLoadout_Money)
		.Process(this->DropshipLoadout_StartEVA)
		.Process(this->DropshipLoadout_Carriers)
		.Process(this->DropshipLoadout_AddUnusedMoneyToPlayer)
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
		;
}

void HouseTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseTypeExt::ExtContainer::Load(HouseTypeClass* pThis, IStream* pStm)
{
	HouseTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);
	return pData != nullptr;
};

bool HouseTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool HouseTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
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

	HouseTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x512480, HouseTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x512290, HouseTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x51246D, HouseTypeClass_Load_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x51255C, HouseTypeClass_Save_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.SaveStatic();
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

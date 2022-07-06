#include "Body.h"

#include <HouseClass.h>
#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x11111111;
SWTypeExt::ExtContainer SWTypeExt::ExtMap;


// Ares 0.A
// I don't know Ares 3.0 has add some new things in this 
bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse)
{
	//what the hell? you send nullptr?
	if (pHouse == nullptr)
		return false;

	const auto pThis = this->OwnerObject();

	if (pHouse->ControlledByHuman() && !SW_AllowPlayer)
		return false;

	if (!pHouse->ControlledByHuman() && !SW_AllowAI)
		return false;

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
		return false;

	// allow only certain houses, disallow forbidden houses
	const DWORD OwnerBits = 1UL << pHouse->Type->ArrayIndex;
	if (!(this->SW_RequiredHouses & OwnerBits) || (this->SW_ForbiddenHouses & OwnerBits))
		return false;

	// check that any aux building exist and no neg building
	auto IsTechnoPresent = [pHouse](TechnoTypeClass* pType)
	{
		return pType && pHouse->CountOwnedAndPresent(pType) > 0;
	};

	if (!SW_AuxBuildings.empty() && std::none_of(SW_AuxBuildings.begin(), SW_AuxBuildings.end(), IsTechnoPresent))
		return false;

	if (std::any_of(SW_NegBuildings.begin(), SW_NegBuildings.end(), IsTechnoPresent))
		return false;

	if (SW_AuxTechno_Any)
	{

		if (!SW_AuxTechno.empty() && std::none_of(SW_AuxTechno.begin(), SW_AuxTechno.end(), IsTechnoPresent))
			return false;
	}
	else
	{
		if (!SW_AuxTechno.empty() && !std::all_of(SW_AuxTechno.begin(), SW_AuxTechno.end(), IsTechnoPresent))
			return false;
	}

	if (SW_NegTechno_Any)
	{
		if (std::any_of(SW_NegTechno.begin(), SW_NegTechno.end(), IsTechnoPresent))
			return false;
	}
	else
	{
		if (std::all_of(SW_NegTechno.begin(), SW_NegTechno.end(), IsTechnoPresent))
			return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void SWTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Money_Amount)
		.Process(this->SW_Inhibitors)
		.Process(this->SW_AnyInhibitor)
		.Process(this->UIDescription)
		.Process(this->CameoPriority)
		.Process(this->LimboDelivery_Types)
		.Process(this->LimboDelivery_IDs)
		.Process(this->LimboDelivery_RandomWeightsData)
		.Process(this->LimboDelivery_RollChances)
		.Process(this->LimboKill_Affected)
		.Process(this->LimboKill_IDs)
		.Process(this->RandomBuffer)
		.Process(this->SW_AuxTechno)
		.Process(this->SW_NegTechno)
		.Process(this->SW_AuxTechno_Any)
		.Process(this->SW_NegTechno_Any)
		.Process(this->SW_AuxBuildings)
		.Process(this->SW_NegBuildings)
		.Process(this->SW_RequiredHouses)
		.Process(this->SW_ForbiddenHouses)
		.Process(this->SW_AlwaysGranted)
		.Process(this->SW_AllowPlayer)
		.Process(this->SW_AllowAI)
		.Process(this->SW_ShowCameo)
		.Process(this->SW_AutoFire)
		;
}

void SWTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	// from ares
	this->Money_Amount.Read(exINI, pSection, "Money.Amount");
	this->SW_Inhibitors.Read(exINI, pSection, "SW.Inhibitors");
	this->SW_AnyInhibitor.Read(exINI, pSection, "SW.AnyInhibitor");

	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	this->LimboDelivery_Types.Read(exINI, pSection, "LimboDelivery.Types");
	this->LimboDelivery_IDs.Read(exINI, pSection, "LimboDelivery.IDs");
	this->LimboDelivery_RollChances.Read(exINI, pSection, "LimboDelivery.RollChances");

	char tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LimboDelivery.RandomWeights%d", i);
		weights.Read(exINI, pSection, tempBuffer);

		if (!weights.size())
			break;

		this->LimboDelivery_RandomWeightsData.push_back(weights);
	}
	ValueableVector<int> weights;
	weights.Read(exINI, pSection, "LimboDelivery.RandomWeights");
	if (weights.size())
		this->LimboDelivery_RandomWeightsData[0] = weights;

	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");

	this->SW_AuxTechno.Read(exINI, pSection, "SW.AuxTechno");
	this->SW_NegTechno.Read(exINI, pSection, "SW.NegTechno");
	this->SW_AuxTechno_Any.Read(exINI, pSection, "SW.AuxTechno.Any");
	this->SW_NegTechno_Any.Read(exINI, pSection, "SW.NegTechno.Any");
	this->SW_AuxBuildings.Read(exINI, pSection, "SW.AuxBuildings");
	this->SW_NegBuildings.Read(exINI, pSection, "SW.NegBuildings");
	this->SW_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SW.RequiredHouses", this->SW_RequiredHouses);
	this->SW_ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "SW.ForbiddenHouses", this->SW_ForbiddenHouses);
	this->SW_AlwaysGranted.Read(exINI, pSection, "SW.AlwaysGranted");
	this->SW_AllowPlayer.Read(exINI, pSection, "SW.AllowPlayer");
	this->SW_AllowAI.Read(exINI, pSection, "SW.AllowAI");
	this->SW_ShowCameo.Read(exINI, pSection, "SW.ShowCameo");
	this->SW_AutoFire.Read(exINI, pSection, "SW.AutoFire");
}

void SWTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<SuperWeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SWTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<SuperWeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool SWTypeExt::LoadGlobals(PhobosStreamReader& Stm) {
	return Stm
		.Success();
}

bool SWTypeExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm
		.Success();
}

// =============================
// container

SWTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass") {
}

SWTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6CE6F6, SuperWeaponTypeClass_CTOR, 0x5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SWTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CE8BE, SuperWeaponTypeClass_Load_Suffix, 0x7)
{
	SWTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CE8EA, SuperWeaponTypeClass_Save_Suffix, 0x3)
{
	SWTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CEE50, SuperWeaponTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6CEE43, SuperWeaponTypeClass_LoadFromINI, 0xA)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

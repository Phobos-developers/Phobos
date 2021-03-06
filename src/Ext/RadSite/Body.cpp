#include "Body.h"

#include "../../Enum/RadTypes.h"

template<> const DWORD Extension<RadSiteClass>::Canary = 0x87654321;
RadSiteExt::ExtContainer RadSiteExt::ExtMap;

DynamicVectorClass<RadSiteExt::ExtData*> RadSiteExt::RadSiteInstance;

void RadSiteExt::CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt) {
	// use real ctor
	auto const pRadSite = GameCreate<RadSiteClass>();

	auto pRadExt = RadSiteExt::ExtMap.FindOrAllocate(pRadSite);

	pRadExt->Weapon = pWeaponExt->OwnerObject();
	pRadExt->Type = &pWeaponExt->RadType;

	pRadSite->SetBaseCell(&location);
	pRadSite->SetSpread(spread);

	RadSiteExt::SetRadLevel(pRadSite, pRadExt->Type, amount);

	pRadSite->Activate();

	if (RadSiteInstance.FindItemIndex(pRadExt) == -1) {
		RadSiteInstance.AddItem(pRadExt);
	}
}

// Rewrite because Of crashing crazy
void RadSiteExt::RadSiteAdd(RadSiteClass* pRad, int lvmax, int amount) {
	int value = pRad->RadLevel * pRad->RadTimeLeft / pRad->RadDuration;
	pRad->Deactivate();
	pRad->RadLevel = value + amount;
	pRad->RadDuration = pRad->RadLevel * lvmax;
	pRad->RadTimeLeft = pRad->RadDuration;
	pRad->Activate();
}

void RadSiteExt::SetRadLevel(RadSiteClass* pRad, RadType* Type, int amount) {
	const int mult = Type->DurationMultiple;
	pRad->RadLevel = amount;
	pRad->RadDuration = mult * amount;
	pRad->RadTimeLeft = mult * amount;
}

// helper function provided by AlexB
double RadSiteExt::GetRadLevelAt(RadSiteClass* pThis, CellStruct const& cell) {
	const Vector3D<int> base   = MapClass::Instance->GetCellAt(pThis->BaseCell)->GetCoords();
	const Vector3D<int> coords = MapClass::Instance->GetCellAt(cell)->GetCoords();

	const auto max = static_cast<double>(pThis->SpreadInLeptons);
	const auto dist = coords.DistanceFrom(base);

	return (dist > max) ? 0.0 : (max - dist) / max * pThis->RadLevel;
}

// =============================
// load / save

void RadSiteExt::ExtData::LoadFromStream(IStream* Stm) {
	char weaponID[sizeof(this->Weapon->ID)];
	PhobosStreamReader::Process(Stm, weaponID);
	this->Weapon = WeaponTypeClass::Find(weaponID);

	if (this->Weapon) {
		auto pWeaponTypeExt = WeaponTypeExt::ExtMap.FindOrAllocate(Weapon);
		if (pWeaponTypeExt) {
			this->Type = &pWeaponTypeExt->RadType;
		}
	}
}

void RadSiteExt::ExtData::SaveToStream(IStream* Stm) {
	PhobosStreamWriter::Process(Stm, this->Weapon->ID);
}

// =============================
// container

RadSiteExt::ExtContainer::ExtContainer() : Container("RadSiteClass") {};

RadSiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(65B28D, RadSiteClass_CTOR, 6)
{
	GET(RadSiteClass*, pThis, ESI);
	auto pRadSiteExt = RadSiteExt::ExtMap.FindOrAllocate(pThis);
	if (RadSiteExt::RadSiteInstance.FindItemIndex(pRadSiteExt) == -1) {
		RadSiteExt::RadSiteInstance.AddItem(pRadSiteExt);
	}
	return 0;
}

DEFINE_HOOK(65B2F4, RadSiteClass_DTOR, 5)
{
	GET(RadSiteClass*, pThis, ECX);
	auto pRadExt = RadSiteExt::ExtMap.Find(pThis);

	RadSiteExt::ExtMap.Remove(pThis);

	RadSiteExt::RadSiteInstance.Remove(pRadExt);
	return 0;
}

DEFINE_HOOK_AGAIN(65B3D0, RadSiteClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(65B450, RadSiteClass_SaveLoad_Prefix, 8)
{
	GET_STACK(RadSiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	RadSiteExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(65B43F, RadSiteClass_Load_Suffix, 7)
{
	auto pItem = RadSiteExt::ExtMap.Find(RadSiteExt::ExtMap.SavingObject);
	IStream* pStm = RadSiteExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(65B464, RadSiteClass_Save_Suffix, 5)
{
	auto pItem = RadSiteExt::ExtMap.Find(RadSiteExt::ExtMap.SavingObject);
	IStream* pStm = RadSiteExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

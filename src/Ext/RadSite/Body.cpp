#include "Body.h"

#include "../../Enum/RadTypes.h"

template<> const DWORD Extension<RadSiteClass>::Canary = 0x87654321;
RadSiteExt::ExtContainer RadSiteExt::ExtMap;

DynamicVectorClass<RadSiteExt::ExtData*> RadSiteExt::Instances;

void RadSiteExt::CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass * const pOwner) {
	// use real ctor
	auto const pRadSite = GameCreate<RadSiteClass>();

	auto pRadExt = RadSiteExt::ExtMap.FindOrAllocate(pRadSite);

	//Adding Owner to RadSite , from bullet
	if (!pWeaponExt->Rad_NoOwner && pRadExt->RadHouse != pOwner) {
		pRadExt->RadHouse = pOwner;
	}

	pRadExt->Weapon = pWeaponExt->OwnerObject();
	pRadExt->Type = &pWeaponExt->RadType;

	pRadSite->SetBaseCell(&location);
	pRadSite->SetSpread(spread);

	pRadExt->SetRadLevel(amount);

	pRadSite->Activate();

	Instances.AddUnique(pRadExt);
}

/*  Including them as EXT so it keep tracked at save/load */

// Rewrite because of crashing craziness
void RadSiteExt::ExtData::Add(int amount) {
	auto pRad = this->OwnerObject();
	int value = pRad->RadLevel * pRad->RadTimeLeft / pRad->RadDuration;
	pRad->Deactivate();
	pRad->RadLevel = value + amount;
	pRad->RadDuration = pRad->RadLevel * this->Type->DurationMultiple;
	pRad->RadTimeLeft = pRad->RadDuration;
	pRad->Activate();
}

void RadSiteExt::ExtData::SetRadLevel(int amount) {
	auto pRad = this->OwnerObject();
	const int mult = this->Type->DurationMultiple;
	pRad->RadLevel = amount;
	pRad->RadDuration = mult * amount;
	pRad->RadTimeLeft = mult * amount;
}

// helper function provided by AlexB
double RadSiteExt::ExtData::GetRadLevelAt(CellStruct const& cell) {
	auto pThis = this->OwnerObject();
	const Vector3D<int> base   = MapClass::Instance->GetCellAt(pThis->BaseCell)->GetCoords();
	const Vector3D<int> coords = MapClass::Instance->GetCellAt(cell)->GetCoords();

	const auto max = static_cast<double>(pThis->SpreadInLeptons);
	const auto dist = coords.DistanceFrom(base);

	return (dist > max) ? 0.0 : (max - dist) / max * pThis->RadLevel;
}
	
// =============================
// load / save

template <typename T>
void RadSiteExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Weapon)
		.Process(this->Type)
		;
}

void RadSiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<RadSiteClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RadSiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<RadSiteClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

RadSiteExt::ExtContainer::ExtContainer() : Container("RadSiteClass") {};

RadSiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(65B28D, RadSiteClass_CTOR, 6)
{
	GET(RadSiteClass*, pItem, ESI);
	auto pExt = RadSiteExt::ExtMap.FindOrAllocate(pItem);

	RadSiteExt::Instances.AddUnique(pExt);

	return 0;
}

DEFINE_HOOK(65B2F4, RadSiteClass_DTOR, 5)
{
	GET(RadSiteClass*, pItem, ECX);
	auto pExt = RadSiteExt::ExtMap.Find(pItem);

	RadSiteExt::ExtMap.Remove(pItem);
	RadSiteExt::Instances.Remove(pExt);

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
	RadSiteExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(65B464, RadSiteClass_Save_Suffix, 5)
{
	RadSiteExt::ExtMap.SaveStatic();
	return 0;
}
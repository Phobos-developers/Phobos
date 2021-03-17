#include "Body.h"

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

void BuildingTypeExt::ExtData::Initialize()
{

}

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
		strcpy_s(pThis->PowersUpBuilding, BuildingTypeClass::Array->GetItem(this->PowersUp_Buildings[0])->ID);
}

void BuildingTypeExt::ExtData::CompleteInitialization() {
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm) {
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm) {

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm) {


	return Stm.Success();
}
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") {
}

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(45E50C, BuildingTypeClass_CTOR, 6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(45E707, BuildingTypeClass_DTOR, 6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(465300, BuildingTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(465010, BuildingTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(4652ED, BuildingTypeClass_Load_Suffix, 7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(46536A, BuildingTypeClass_Save_Suffix, 7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(464A56, BuildingTypeClass_LoadFromINI, A)
DEFINE_HOOK(464A49, BuildingTypeClass_LoadFromINI, A)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

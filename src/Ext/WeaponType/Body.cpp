#include "Body.h"

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x22222222;
WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

void WeaponTypeExt::ExtData::Initialize()
{

};

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	{ // DiskLaser_Radius
		this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
		this->DiskLaser_Circumference = (int)(this->DiskLaser_Radius * Math::Pi * 2);
	}

	// RadType
	if (this->OwnerObject()->RadLevel > 0)
		this->RadType.Read(pINI, pSection, "RadType");
		this->Rad_NoOwner.Read(exINI, pSection, "Rad.NoOwner");
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->DiskLaser_Circumference)
        .Process(this->Rad_NoOwner)
		;
};

void WeaponTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<WeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	if (this->OwnerObject()->RadLevel > 0)
		this->RadType.LoadFromStream(Stm);
}

void WeaponTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<WeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
	if (this->OwnerObject()->RadLevel > 0)
		this->RadType.SaveToStream(Stm);
}

bool WeaponTypeExt::LoadGlobals(PhobosStreamReader& Stm) {
	return Stm
		.Process(nOldCircumference)
		.Success();
}

bool WeaponTypeExt::SaveGlobals(PhobosStreamWriter& Stm) {
	return Stm
		.Process(nOldCircumference)
		.Success();
}

// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") {
}

WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(771EE9, WeaponTypeClass_CTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(77311D, WeaponTypeClass_SDDTOR, 6)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(772EB0, WeaponTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(772CD0, WeaponTypeClass_SaveLoad_Prefix, 7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(772EA6, WeaponTypeClass_Load_Suffix, 6)
{
	WeaponTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(772F8C, WeaponTypeClass_Save, 5)
{
	WeaponTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(7729C7, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(7729D6, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK(7729B0, WeaponTypeClass_LoadFromINI, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

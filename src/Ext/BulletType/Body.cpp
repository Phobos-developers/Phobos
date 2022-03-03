#include "Body.h"

template<> const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
BulletTypeExt::ExtContainer BulletTypeExt::ExtMap;

double BulletTypeExt::GetAdjustedGravity(BulletTypeClass* pType)
{
	auto const pData = BulletTypeExt::ExtMap.Find(pType);
	auto const nGravity = pData->Gravity.Get(RulesClass::Instance->Gravity);
	return pType->Floater ? nGravity * 0.5 : nGravity;
}

BulletTypeClass* BulletTypeExt::GetDefaultBulletType()
{
	BulletTypeClass* pType = BulletTypeClass::Find(NONE_STR);

	if (pType)
		return pType;

	return GameCreate<BulletTypeClass>(NONE_STR);
}

// =============================
// load / save

void BulletTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Interceptable.Read(exINI, pSection, "Interceptable");
	this->Gravity.Read(exINI, pSection, "Gravity");
	this->Gravity_HeightFix.Read(exINI, pSection, "Gravity.HeightFix");
	this->Shrapnel_AffectsGround.Read(exINI, pSection, "Shrapnel.AffectsGround");
	this->Shrapnel_AffectsBuildings.Read(exINI, pSection, "Shrapnel.AffectsBuildings");

	INI_EX exArtINI(CCINIClass::INI_Art);

	if (strlen(pThis->ImageFile))
		pSection = pThis->ImageFile;

	this->LaserTrail_Types.Read(exArtINI, pSection, "LaserTrail.Types");
}

template <typename T>
void BulletTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Interceptable)
		.Process(this->LaserTrail_Types)
		.Process(this->Gravity)
		.Process(this->Gravity_HeightFix)
		.Process(this->Shrapnel_AffectsGround)
		.Process(this->Shrapnel_AffectsBuildings)
		;
}

void BulletTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}


// =============================
// container

BulletTypeExt::ExtContainer::ExtContainer() : Container("BulletTypeClass") { }

BulletTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);

	BulletTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ESI);

	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x46C722, BulletTypeClass_Load_Suffix, 0x4)
{
	BulletTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46C74A, BulletTypeClass_Save_Suffix, 0x3)
{
	BulletTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

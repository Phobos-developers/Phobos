#include "Body.h"

#include <ParticleTypeClass.h>

#include <Utilities/GeneralUtils.h>

template<> const DWORD Extension<ParticleTypeClass>::Canary = 0x33333333;
ParticleTypeExt::ExtContainer ParticleTypeExt::ExtMap;

void ParticleTypeExt::ExtData::Initialize() { }

// =============================
// load / save

void ParticleTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();

	// Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);
	auto pArtSection = pThis->ImageFile;

	this->LaserTrail_Types.Read(exArtINI, pArtSection, "LaserTrail.Types");
}

template <typename T>
void ParticleTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LaserTrail_Types)
		;
}
void ParticleTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ParticleTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ParticleTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

ParticleTypeExt::ExtContainer::ExtContainer() : Container("ParticleTypeClass") { }
ParticleTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644DBB, ParticleTypeClass_CTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);

	ParticleTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x644E40, ParticleTypeClass_DTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ECX);

	ParticleTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6457A0, ParticleTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix, 0x6)
{
	ParticleTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix, 0x5)
{
	ParticleTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x645414, ParticleTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x645405, ParticleTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE0);

	ParticleTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

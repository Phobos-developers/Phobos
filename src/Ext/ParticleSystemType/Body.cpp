#include "Body.h"

ParticleSystemTypeExt::ExtContainer ParticleSystemTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void ParticleSystemTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->AdjustTargetCoordsOnRotation)
		;
}

void ParticleSystemTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->AdjustTargetCoordsOnRotation.Read(exINI, pSection, "AdjustTargetCoordsOnRotation");
}

void ParticleSystemTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleSystemTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool ParticleSystemTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParticleSystemTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ParticleSystemTypeExt::ExtContainer::ExtContainer() : Container("ParticleSystemTypeClass") { }
ParticleSystemTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644215, ParticleSystemTypeClass_CTOR, 0x7)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);

	ParticleSystemTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x644986, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);

	ParticleSystemTypeExt::ExtMap.Remove(pItem);

	return 0;
}

//DEFINE_HOOK_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)// Section dont exist!
DEFINE_HOOK(0x644615, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

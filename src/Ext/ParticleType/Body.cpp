#include "Body.h"

ParticleTypeExt::ExtContainer ParticleTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void ParticleTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->Gas_MaxDriftSpeed)
		;
}

void ParticleTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->Gas_MaxDriftSpeed.Read(exINI, pSection, "Gas.MaxDriftSpeed");

	if (pThis->StateAIAdvance == 0 && pThis->StartStateAI < pThis->EndStateAI)
	{
		Debug::FatalErrorAndExit(Debug::ExitCode::BadINIUsage,
			"[%s] has StateAIAdvance=0 in conjunction with StartStateAI value less than EndStateAI.\n", pSection);
		pThis->StateAIAdvance = 1;
	}
}

void ParticleTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool ParticleTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParticleTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
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

	ParticleTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

// Late in the destructor body, right before it chains into the base destructor.
// Deliberately stacked on the exact address and size of Ares' own hook: any other
// placement in this stretch would overlap its 5-byte JMP and corrupt the patch.
DEFINE_HOOK(0x645A3B, ParticleTypeClass_DTOR, 0x7)
{
	GET(ParticleTypeClass*, pItem, ESI);

	ParticleTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK(0x6453FF, ParticleTypeClass_LoadFromINI, 0x6)
{
	GET(ParticleTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xDC, 0x4));

	ParticleTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

#include "Body.h"

ParticleTypeExt::ExtContainer ParticleTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void ParticleTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Gas_MaxDriftSpeed)
		;
}

void ParticleTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Gas_MaxDriftSpeed.Read(exINI, pSection, "Gas.MaxDriftSpeed");

	if (pThis->StateAIAdvance == 0 && pThis->StartStateAI < pThis->EndStateAI)
	{
		Debug::FatalErrorAndExit(Debug::ExitCode::BadINIUsage,
			"[%s] has StateAIAdvance=0 in conjunction with StartStateAI value less than EndStateAI.\n", pSection);
		pThis->StateAIAdvance = 1;
	}
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

DEFINE_HOOK_AGAIN(0x6457A0, ParticleTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(ParticleTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix, 0x5)
{
	ParticleTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix, 0x7)
{
	ParticleTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x6453FF, ParticleTypeClass_LoadFromINI, 0x6)
{
	GET(ParticleTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xDC, 0x4));

	ParticleTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

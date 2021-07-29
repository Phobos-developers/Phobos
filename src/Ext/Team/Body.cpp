#include "Body.h"

template<> const DWORD Extension<TeamClass>::Canary = 0x414B4B41;
TeamExt::ExtContainer TeamExt::ExtMap;

// =============================
// container

TeamExt::ExtContainer::ExtContainer() : Container("TeamClass") { }

TeamExt::ExtContainer::~ExtContainer() = default;

// =============================
// load / save

template <typename T>
void TeamExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->WaitNoTargetAttempts)
		;
}

void TeamExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TeamExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TeamExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

void TeamExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks


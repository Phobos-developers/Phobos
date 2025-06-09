#include "Body.h"

EBoltExt::ExtContainer EBoltExt::ExtMap;

// =============================
// load / save

template <typename T>
void EBoltExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Color)
		.Process(this->Disable)
		.Process(this->Arcs)
		.Process(this->BurstIndex)
		;
}

void EBoltExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<EBolt>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void EBoltExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<EBolt>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool EBoltExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool EBoltExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

void EBoltExt::ExtData::InvalidatePointer(void* ptr, bool removed)
{ }

// =============================
// container

EBoltExt::ExtContainer::ExtContainer() : Container("EBolt") { }
EBoltExt::ExtContainer::~ExtContainer() = default;

bool EBoltExt::ExtContainer::InvalidateExtDataIgnorable(void* const ptr) const
{
	return true;
}

// =============================
// container hooks

DEFINE_HOOK(0x4C1E42, EBolt_CTOR, 0x5)
{
	GET(EBolt*, pItem, EAX);

	EBoltExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pItem, ESI);

	EBoltExt::ExtMap.Remove(pItem);

	return 0;
}

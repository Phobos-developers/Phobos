#include "Body.h"

AircraftTypeExt::ExtContainer AircraftTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void AircraftTypeExt::Serialize(T& Stm)
{
}

void AircraftTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

AircraftTypeExt::ExtContainer::ExtContainer() : Container("AircraftTypeClass") { }
AircraftTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x41C8C0, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// Hooked after the base destructor call in both destructor bodies; the second site
// is the tail of the standalone body (pop/pop/retn, safe to steal - the bytes after
// it are alignment padding that is never executed).
DEFINE_HOOK_AGAIN(0x41CA96, AircraftTypeClass_DTOR, 0x3)
DEFINE_HOOK(0x41D056, AircraftTypeClass_DTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Remove(pItem);

	return 0;
}

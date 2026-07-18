#include "Body.h"

InfantryExt::ExtContainer InfantryExt::ExtMap;

// =============================
// load / save

template <typename T>
void InfantryExt::Serialize(T& Stm)
{
}

void InfantryExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") { }
InfantryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x517A60, InfantryClass_CTOR, 0xE)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK(0x517F81, InfantryClass_DTOR, 0x8)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Remove(pItem);

	return 0;
}

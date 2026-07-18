#include "Body.h"

UnitExt::ExtContainer UnitExt::ExtMap;

// =============================
// load / save

template <typename T>
void UnitExt::Serialize(T& Stm)
{
}

void UnitExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void UnitExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

UnitExt::ExtContainer::ExtContainer() : Container("UnitClass") { }
UnitExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7353D3, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x7359DA, UnitClass_DTOR, 0x9)
DEFINE_HOOK(0x735967, UnitClass_DTOR, 0x9)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Remove(pItem);

	return 0;
}

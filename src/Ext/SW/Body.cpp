#include "Body.h"

SuperExt::ExtContainer SuperExt::ExtMap;

// =============================
// load / save

template <typename T>
void SuperExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TimerRestarted)
		;
}

void SuperExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<SuperClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SuperExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<SuperClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void SuperExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	//AnnounceInvalidPointer(SuperLeader, ptr);
}

// =============================
// container

SuperExt::ExtContainer::ExtContainer() : Container("SuperClass") { }
SuperExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//Everything InitEd beside the Vector below this address

DEFINE_HOOK_AGAIN(0x6CAF32, SuperClass_CTOR, 0x6)
DEFINE_HOOK(0x6CB00A, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pThis, ESI);

	SuperExt::ExtMap.TryAllocate(pThis);

	return 0;
}

//before `test` i hope not crash the game ,..
DEFINE_HOOK(0x6CB1AD, SuperClass_DTOR, 0x6)
{
	GET(SuperClass*, pThis, ESI);

	SuperExt::ExtMap.Remove(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6CDEF0, SuperClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6CDFD0, SuperClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SuperClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CDFC4, SuperClass_Load_Suffix, 0x7)
{
	SuperExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6CDFEA, SuperClass_Save_Suffix, 0x6)
{
	SuperExt::ExtMap.SaveStatic();
	return 0;
}

#include "Body.h"


VoxelAnimTypeExt::ExtContainer VoxelAnimTypeExt::ExtMap;

void VoxelAnimTypeExt::ExtData::Initialize() {}

void VoxelAnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;
	INI_EX exINI(pINI);

	this->LaserTrail_Types.Read(exINI, pID, "LaserTrail.Types");
	this->ExplodeOnWater.Read(exINI, pID, "ExplodeOnWater");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");
	this->WakeAnim.Read(exINI, pID, "WakeAnim");
	this->SplashAnims.Read(exINI, pID, "SplashAnims");
	this->SplashAnims_PickRandom.Read(exINI, pID, "SplashAnims.PickRandom");
}

// =============================
// load / save
template <typename T>
void VoxelAnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(LaserTrail_Types)
		.Process(this->ExplodeOnWater)
		.Process(this->Warhead_Detonate)
		.Process(this->WakeAnim)
		.Process(this->SplashAnims)
		.Process(this->SplashAnims_PickRandom)
		;
}

void VoxelAnimTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<VoxelAnimTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void VoxelAnimTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<VoxelAnimTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool VoxelAnimTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool VoxelAnimTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

VoxelAnimTypeExt::ExtContainer::ExtContainer() : Container("VoxelVoxelAnimTypeClass") {}
VoxelAnimTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x74AEB0, VoxelAnimTypeClass_CTOR, 0xB)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);

	VoxelAnimTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x74BA31, VoxelAnimTypeClass_DTOR, 0x5)
{
	GET(VoxelAnimTypeClass*, pItem, ECX);

	VoxelAnimTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x74B810, VoxelAnimTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x74B8D0, VoxelAnimTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(VoxelAnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x74B8C2, VoxelAnimTypeClass_Load_Suffix, 0x5)
{
	VoxelAnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x74B8EA, VoxelAnimTypeClass_Save_Suffix, 0x5)
{
	VoxelAnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x74B607, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B561, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B54A, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B51B, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x74B4F0, VoxelAnimTypeClass_LoadFromINI, 0x5)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x4);

	VoxelAnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

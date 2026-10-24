#include "Body.h"

VoxelAnimExt::ExtContainer VoxelAnimExt::ExtMap;

void VoxelAnimExt::InitializeLaserTrails(VoxelAnimClass* pThis)
{
	const auto pThisExt = VoxelAnimExt::Fetch(pThis);

	if (pThisExt->LaserTrails.size())
		return;

	const auto pTypeExt = VoxelAnimTypeExt::Fetch(pThis->Type);
	const auto pOwner = pThis->OwnerHouse;
	pThisExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		pThisExt->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[idxTrail].get(), pOwner));
}

void VoxelAnimExt::Initialize() { }

// =============================
// load / save
template <typename T>
void VoxelAnimExt::Serialize(T& Stm)
{
	Stm
		.Process(this->LaserTrails)
		.Process(this->TrailerSpawnTimer)
		;
}

void VoxelAnimExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void VoxelAnimExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool VoxelAnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool VoxelAnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

VoxelAnimExt::ExtContainer::ExtContainer() : Container("VoxelAnimClass") { }
VoxelAnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x749951, VoxelAnimClass_CTOR, 0xC)
DEFINE_HOOK(0x74942E, VoxelAnimClass_CTOR, 0xC)
{
	GET(VoxelAnimClass*, pItem, ESI);

	VoxelAnimExt::ExtMap.TryAllocate(pItem);
	VoxelAnimExt::InitializeLaserTrails(pItem);

	return 0;
}

DEFINE_HOOK(0x7499F1, VoxelAnimClass_DTOR, 0x5)
{
	GET(VoxelAnimClass*, pItem, ECX);

	VoxelAnimExt::ExtMap.Remove(pItem);

	return 0;
}


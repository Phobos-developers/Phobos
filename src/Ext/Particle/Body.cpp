#include "Body.h"

#include <Ext/ParticleType/Body.h>

template<> const DWORD Extension<ParticleClass>::Canary = 0xAAAABBBB;
ParticleExt::ExtContainer ParticleExt::ExtMap;

void ParticleExt::InitializeLaserTrails(ParticleClass* pThis)
{
	auto pExt = ParticleExt::ExtMap.Find(pThis);

	if (pExt->LaserTrails.size())
		return;

	if (auto pTypeExt = ParticleTypeExt::ExtMap.Find(pThis->Type))
	{
		for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
			{
				pExt->LaserTrails.push_back(
					std::make_unique<LaserTrailClass>(pLaserType));
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void ParticleExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LaserTrails)
		;
}

void ParticleExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ParticleClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ParticleClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

ParticleExt::ExtContainer::ExtContainer() : Container("ParticleClass") { }

ParticleExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x62BB13, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);

	ParticleExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x62BCC0, ParticleClass_DTOR, 0x5)
{
	GET(ParticleClass*, pItem, ECX);

	ParticleExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x62D810, ParticleClass_SaveLoad_Prefix, 0xD)
DEFINE_HOOK(0x62D7A0, ParticleClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x62D803, ParticleClass_Load_Suffix, 0x5)
{
	ParticleExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x62D825, ParticleClass_Save_Suffix, 0x7)
{
	ParticleExt::ExtMap.SaveStatic();
	return 0;
}

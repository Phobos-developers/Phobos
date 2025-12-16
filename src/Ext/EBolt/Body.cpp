#include "Body.h"

#include <Ext/WeaponType/Body.h>

EBoltExt::ExtContainer EBoltExt::ExtMap;

EBolt* EBoltExt::CreateEBolt(WeaponTypeClass* pWeapon)
{
	const auto pBolt = GameCreate<EBolt>();
	const auto pBoltExt = EBoltExt::ExtMap.Find(pBolt);

	const int alternateIdx = pWeapon->IsAlternateColor ? 5 : 10;
	const int defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, alternateIdx);
	const int defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto& boltDisable = pWeaponExt->Bolt_Disable;
	const auto& boltColor = pWeaponExt->Bolt_Color;

	for (int idx = 0; idx < 3; ++idx)
	{
		if (boltDisable[idx])
			pBoltExt->Disable[idx] = true;
		else if (boltColor[idx].isset())
			pBoltExt->Color[idx] = boltColor[idx].Get();
		else
			pBoltExt->Color[idx] = Drawing::Int_To_RGB(idx < 2 ? defaultAlternate : defaultWhite);
	}

	pBoltExt->Arcs = pWeaponExt->Bolt_Arcs;
	pBolt->Lifetime = 1 << (std::clamp(pWeaponExt->Bolt_Duration.Get(), 1, 31) - 1);
	return pBolt;
}

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

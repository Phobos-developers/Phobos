#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::SetInvoker(TechnoClass* pInvoker)
{
	this->Invoker = pInvoker;
	this->InvokerHouse = pInvoker ? pInvoker->Owner : nullptr;
}

void AnimExt::ExtData::CreateAttachedSystem()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt && pTypeExt->AttachedSystem && !this->AttachedSystem)
	{
		if (auto const pSystem = GameCreate<ParticleSystemClass>(pTypeExt->AttachedSystem.Get(), pThis->Location, pThis->GetCell(), pThis, CoordStruct::Empty, nullptr))
			this->AttachedSystem = pSystem;
	}
}

void AnimExt::ExtData::DeleteAttachedSystem()
{
	if (this->AttachedSystem)
	{
		this->AttachedSystem->Owner = nullptr;
		this->AttachedSystem->UnInit();
		this->AttachedSystem = nullptr;
	}
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim);

	if (newOwner)
	{
		pAnim->Owner = newOwner;

		if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
	}

	return newOwner;
}

HouseClass* AnimExt::GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner)
{
	if (!pAnim)
		return pDefaultOwner;

	HouseClass* pTechnoOwner = nullptr;

	if (auto const pTechno = abstract_cast<TechnoClass*>(pAnim->OwnerObject))
		pTechnoOwner = pTechno->Owner;

	if (pAnim->Owner)
		return pAnim->Owner;
	else
		return  pTechnoOwner ? pTechnoOwner : pDefaultOwner;
}

void AnimExt::HandleDebrisImpact(AnimTypeClass* pExpireAnim, AnimTypeClass* pWakeAnim, Iterator<AnimTypeClass*> splashAnims, HouseClass* pOwner, WarheadTypeClass* pWarhead, int nDamage,
	CellClass* pCell, CoordStruct nLocation, bool heightFlag, bool isMeteor, bool warheadDetonate, bool explodeOnWater, bool splashAnimsPickRandom)
{
	AnimTypeClass* pWakeAnimToUse = nullptr;
	AnimTypeClass* pSplashAnimToUse = nullptr;

	if (pCell->LandType != LandType::Water || heightFlag || explodeOnWater)
	{
		if (pWarhead)
		{
			if (warheadDetonate)
			{
				WarheadTypeExt::DetonateAt(pWarhead, nLocation, nullptr, nDamage);
			}
			else
			{
				MapClass::DamageArea(nLocation, nDamage, nullptr, pWarhead, pWarhead->Tiberium, pOwner);
				MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
			}
		}

		if (pExpireAnim)
		{
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, 0x2600u, 0, 0))
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
		}
	}
	else
	{
		if (!isMeteor)
			pWakeAnimToUse = RulesClass::Instance->Wake;

		if (pWakeAnim)
			pWakeAnimToUse = pWakeAnim;

		if (splashAnims.size() > 0)
		{
			auto nIndexR = (splashAnims.size() - 1);
			auto nIndex = splashAnimsPickRandom ?
				ScenarioClass::Instance->Random.RandomRanged(0, nIndexR) : 0;

			pSplashAnimToUse = splashAnims.at(nIndex);
		}
	}

	if (pWakeAnimToUse)
	{
		if (auto const pWakeAnimCreated = GameCreate<AnimClass>(pWakeAnimToUse, nLocation, 0, 1, 0x600u, false))
			AnimExt::SetAnimOwnerHouseKind(pWakeAnimCreated, pOwner, nullptr, false);
	}

	if (pSplashAnimToUse)
	{
		if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnimToUse, nLocation, 0, 1, 0x600u, false))
			AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false);
	}
}

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathUnitFacing)
		.Process(this->FromDeathUnit)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->DeathUnitHasTurret)
		.Process(this->Invoker)
		.Process(this->InvokerHouse)
		.Process(this->AttachedSystem)
		;
}

void AnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::InitializeConstants()
{
	CreateAttachedSystem();
}

// =============================
// container

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x422126, AnimClass_CTOR, 0x5)
DEFINE_HOOK_AGAIN(0x422707, AnimClass_CTOR, 0x5)
DEFINE_HOOK(0x4228D2, AnimClass_CTOR, 0x5)
{
	GET(AnimClass*, pItem, ESI);

	AnimExt::ExtMap.TryAllocate(pItem, pItem->Fetch_ID() != -2, "Creating an animation with null Type!");

	return 0;
}

DEFINE_HOOK(0x422967, AnimClass_DTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);

	AnimExt::ExtMap.Remove(pItem);

	return 0;
}

/*Crash when Anim called with GameDelete()
DEFINE_HOOK(0x426598, AnimClass_SDDTOR, 0x7)
{
	GET(AnimClass*, pItem, ESI);

	if(AnimExt::ExtMap.Find(pItem))
	AnimExt::ExtMap.Remove(pItem);

	return 0;
}
*/

DEFINE_HOOK_AGAIN(0x425280, AnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_Load_Suffix, 0x7)
{
	AnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_Save_Suffix, 0x5)
{
	AnimExt::ExtMap.SaveStatic();
	return 0;
}

/*
DEFINE_HOOK(0x425164, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0xC, 0x8));

	if (auto pAnimExt = AnimExt::ExtMap.Find(pThis))
		pAnimExt->InvalidatePointer(target, all);

	R->EBX(0);
	return pThis->OwnerObject == target && target ? 0x425174 : 0x4251A3;
}
*/

DEFINE_JUMP(LJMP, 0x42543A, 0x425448)

DEFINE_HOOK_AGAIN(0x421EF4, AnimClass_CTOR_setD0, 0x6)
DEFINE_HOOK(0x42276D, AnimClass_CTOR_setD0, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	pThis->unknown_D0 = 0;
	return R->Origin() + 0x6;
}

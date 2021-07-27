#include "Body.h"
#include <Phobos.h>
#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>

template<> const DWORD Extension<AnimTypeClass>::Canary = 0xEEEEEEEE;
AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;

void AnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;

	INI_EX exINI(pINI);

	this->Palette.LoadFromINI(pINI, pID, "CustomPalette");
	this->CreateUnit.Read(exINI, pID, "CreateUnit");
	this->CreateUnit_Facing.Read(exINI, pID, "CreateUnit.Facing");
	this->CreateUnit_UseDeathFacings.Read(exINI, pID, "CreateUnit.UseDeathFacings");
	this->CreateUnit_useDeathTurrentFacings.Read(exINI, pID, "CreateUnit.UseDeathTurrentFacings");
	this->CreateUnit_RemapAnim.Read(exINI, pID, "CreateUnit.RemapAnim");
	this->CreateUnit_Mission.Read(exINI, pID, "CreateUnit.Mission");
	this->CreateUnit_Owner.Read(exINI, pID, "CreateUnit.Owner");
}

const void AnimTypeExt::ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller)
{
	if (!pThis)
		return;

	HouseClass* Invoker = pKiller ? pKiller->Owner : nullptr;
	auto const pType = pThis->Type;

	if (pType->DestroyAnim.Count > 0)
	{
		auto destroyanimsindex = pType->DestroyAnim.Count;
		auto const Facing = pThis->PrimaryFacing.current().value256();
		auto pAnimType = pType->DestroyAnim[ScenarioClass::Instance->Random.Random() % destroyanimsindex];
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (destroyanimsindex >= 8 && !pTypeExt->DestroyAnimRandom.Get())
		{
			if (destroyanimsindex % 2 == 0)
			{
				destroyanimsindex *= static_cast<int>(Facing / 256.0);
				pAnimType = pType->DestroyAnim[destroyanimsindex];
			}
		}

		if (pAnimType)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords()))
			{
				//auto VictimOwner = pThis->IsMindControlled() && pThis->GetOriginalOwner()
				//	? pThis->GetOriginalOwner() : pThis->Owner;

				AnimExt::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner);

				if (pTypeExt->StoreDeathFacings.Get())
				{
					if (auto const AnimExt = AnimExt::ExtMap.Find(pAnim))
					{
						AnimExt->Fromdeathunit = true;
						AnimExt->DeathUnitFacing = Facing;

						if (pThis->HasTurret())
						{
							AnimExt->DeathUnitHasTurrent = true;
							AnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.current();
						}
					}
				}
			}
		}
	}
}


template <typename T>
void AnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->CreateUnit)
		.Process(this->CreateUnit_Facing)
		.Process(this->CreateUnit_UseDeathFacings)
		.Process(this->CreateUnit_RemapAnim)
		.Process(this->CreateUnit_Mission)
		.Process(this->CreateUnit_useDeathTurrentFacings)
		.Process(this->CreateUnit_Owner)
		;
}

void AnimTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

AnimTypeExt::ExtContainer::ExtContainer() : Container("AnimTypeClass") { }
AnimTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);

	AnimTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x428970, AnimTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x428800, AnimTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(AnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x42892C, AnimTypeClass_Load_Suffix, 0x6)
DEFINE_HOOK(0x428958, AnimTypeClass_Load_Suffix, 0x6)
{
	AnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x42898A, AnimTypeClass_Save_Suffix, 0x3)
{
	AnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4287E9, AnimTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI, 0xA)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
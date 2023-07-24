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
	this->CreateUnit.Read(exINI, pID, "CreateUnit",true);
	this->CreateUnit_Facing.Read(exINI, pID, "CreateUnit.Facing");
	this->CreateUnit_InheritDeathFacings.Read(exINI, pID, "CreateUnit.InheritFacings");
	this->CreateUnit_InheritTurretFacings.Read(exINI, pID, "CreateUnit.InheritTurretFacings");
	this->CreateUnit_RemapAnim.Read(exINI, pID, "CreateUnit.RemapAnim");
	this->CreateUnit_Mission.Read(exINI, pID, "CreateUnit.Mission");
	this->CreateUnit_Owner.Read(exINI, pID, "CreateUnit.Owner");
	this->CreateUnit_RandomFacing.Read(exINI, pID, "CreateUnit.RandomFacing");
	this->CreateUnit_ConsiderPathfinding.Read(exINI, pID, "CreateUnit.ConsiderPathfinding");
	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");
	this->UseCenterCoordsIfAttached.Read(exINI, pID, "UseCenterCoordsIfAttached");
	this->Weapon.Read(exINI, pID, "Weapon", true);
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOncePerLoop.Read(exINI, pID, "Damage.ApplyOncePerLoop");
}

const void AnimTypeExt::ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller)
{
	if (!pThis)
		return;

	HouseClass* pInvoker = pKiller ? pKiller->Owner : nullptr;

	if (pThis->Type->DestroyAnim.Count > 0)
	{
		auto const facing = pThis->PrimaryFacing.Current().GetFacing<256>();
		AnimTypeClass* pAnimType = nullptr;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		if (!pTypeExt->DestroyAnim_Random.Get())
		{
			int idxAnim = 0;

			if (pThis->Type->DestroyAnim.Count >= 8)
			{
				idxAnim = pThis->Type->DestroyAnim.Count;
				if (pThis->Type->DestroyAnim.Count % 2 == 0)
					idxAnim *= static_cast<int>(facing / 256.0);
			}

			pAnimType = pThis->Type->DestroyAnim[idxAnim];
		}
		else
		{
			int const nIDx_Rand = pThis->Type->DestroyAnim.Count == 1 ?
				0 : ScenarioClass::Instance->Random.RandomRanged(0, (pThis->Type->DestroyAnim.Count - 1));
			pAnimType = pThis->Type->DestroyAnim[nIDx_Rand];

		}

		if (pAnimType)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords()))
			{
				//auto VictimOwner = pThis->IsMindControlled() && pThis->GetOriginalOwner()
				//	? pThis->GetOriginalOwner() : pThis->Owner;

				auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
				auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);

				AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner);

				pAnimExt->SetInvoker(pThis);
				pAnimExt->FromDeathUnit = true;

				if (pAnimTypeExt->CreateUnit_InheritDeathFacings.Get())
					pAnimExt->DeathUnitFacing = static_cast<short>(facing);

				if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get())
				{
					if (pThis->HasTurret())
					{
						pAnimExt->DeathUnitHasTurret = true;
						pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
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
		.Process(this->CreateUnit_InheritDeathFacings)
		.Process(this->CreateUnit_RemapAnim)
		.Process(this->CreateUnit_Mission)
		.Process(this->CreateUnit_InheritTurretFacings)
		.Process(this->CreateUnit_Owner)
		.Process(this->CreateUnit_RandomFacing)
		.Process(this->CreateUnit_ConsiderPathfinding)
		.Process(this->XDrawOffset)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->UseCenterCoordsIfAttached)
		.Process(this->Weapon)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOncePerLoop)
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

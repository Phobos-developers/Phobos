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

AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;

void AnimTypeExt::ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller)
{
	if (!pThis)
		return;

	HouseClass* pInvoker = pKiller ? pKiller->Owner : nullptr;

	if (pThis->Type->DestroyAnim.Count > 0)
	{
		auto const facing = pThis->PrimaryFacing.Current().GetDir();
		AnimTypeClass* pAnimType = nullptr;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		if (!pTypeExt->DestroyAnim_Random.Get())
		{
			int idxAnim = 0;

			if (pThis->Type->DestroyAnim.Count >= 8)
			{
				idxAnim = pThis->Type->DestroyAnim.Count - 1;
				if (pThis->Type->DestroyAnim.Count % 2 == 0)
					idxAnim = static_cast<int>(static_cast<unsigned char>(facing) / 256.0 * idxAnim);
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
			auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);

			//auto VictimOwner = pThis->IsMindControlled() && pThis->GetOriginalOwner()
			//	? pThis->GetOriginalOwner() : pThis->Owner;

			auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
			auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);

			AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner);

			pAnimExt->SetInvoker(pThis);
			pAnimExt->FromDeathUnit = true;

			if (auto const pCreateUnit = pAnimTypeExt->CreateUnitType.get())
			{
				if (pCreateUnit->InheritDeathFacings)
					pAnimExt->DeathUnitFacing = facing;

				if (pCreateUnit->InheritTurretFacings)
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

void AnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;

	INI_EX exINI(pINI);

	this->Palette.LoadFromINI(pINI, pID, "CustomPalette");
	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");
	this->UseCenterCoordsIfAttached.Read(exINI, pID, "UseCenterCoordsIfAttached");
	this->Weapon.Read<true>(exINI, pID, "Weapon");
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOncePerLoop.Read(exINI, pID, "Damage.ApplyOncePerLoop");
	this->ExplodeOnWater.Read(exINI, pID, "ExplodeOnWater");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");
	this->WakeAnim.Read(exINI, pID, "WakeAnim");
	this->SplashAnims.Read(exINI, pID, "SplashAnims");
	this->SplashAnims_PickRandom.Read(exINI, pID, "SplashAnims.PickRandom");
	this->AttachedSystem.Read<true>(exINI, pID, "AttachedSystem");
	this->AltPalette_ApplyLighting.Read(exINI, pID, "AltPalette.ApplyLighting");
	this->MakeInfantryOwner.Read(exINI, pID, "MakeInfantryOwner");
	this->ExtraShadow.Read(exINI, pID, "ExtraShadow");
	this->DetachedReport.Read(exINI, pID, "DetachedReport");
	this->VisibleTo.Read(exINI, pID, "VisibleTo");
	this->VisibleTo_ConsiderInvokerAsOwner.Read(exINI, pID, "VisibleTo.ConsiderInvokerAsOwner");
	this->RestrictVisibilityIfCloaked.Read(exINI, pID, "RestrictVisibilityIfCloaked");
	this->DetachOnCloak.Read(exINI, pID, "DetachOnCloak");
	this->ConstrainFireAnimsToCellSpots.Read(exINI, pID, "ConstrainFireAnimsToCellSpots");
	this->FireAnimDisallowedLandTypes.Read(exINI, pID, "FireAnimDisallowedLandTypes");
	this->AttachFireAnimsToParent.Read(exINI, pID, "AttachFireAnimsToParent");
	this->SmallFireCount.Read(exINI, pID, "SmallFireCount");
	this->SmallFireAnims.Read(exINI, pID, "SmallFireAnims");
	this->SmallFireChances.Read(exINI, pID, "SmallFireChances");
	this->SmallFireDistances.Read(exINI, pID, "SmallFireDistances");
	this->LargeFireCount.Read(exINI, pID, "LargeFireCount");
	this->LargeFireAnims.Read(exINI, pID, "LargeFireAnims");
	this->LargeFireChances.Read(exINI, pID, "LargeFireChances");
	this->LargeFireDistances.Read(exINI, pID, "LargeFireDistances");
	this->Crater_ReduceTiberium.Read(exINI, pID, "Crater.ReduceTiberium");

	// Parasitic types
	Nullable<TechnoTypeClass*> createUnit;
	createUnit.Read(exINI, pID, "CreateUnit");

	if (createUnit)
	{
		if (this->CreateUnitType == nullptr)
			this->CreateUnitType = std::make_unique<CreateUnitTypeClass>();

		this->CreateUnitType->LoadFromINI(pINI, pID);
	}
	else if (createUnit.isset())
	{
		this->CreateUnitType.reset();
	}
}

template <typename T>
void AnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->CreateUnitType)
		.Process(this->XDrawOffset)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->UseCenterCoordsIfAttached)
		.Process(this->Weapon)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOncePerLoop)
		.Process(this->ExplodeOnWater)
		.Process(this->Warhead_Detonate)
		.Process(this->WakeAnim)
		.Process(this->SplashAnims)
		.Process(this->SplashAnims_PickRandom)
		.Process(this->AttachedSystem)
		.Process(this->AltPalette_ApplyLighting)
		.Process(this->MakeInfantryOwner)
		.Process(this->ExtraShadow)
		.Process(this->DetachedReport)
		.Process(this->VisibleTo)
		.Process(this->VisibleTo_ConsiderInvokerAsOwner)
		.Process(this->RestrictVisibilityIfCloaked)
		.Process(this->DetachOnCloak)
		.Process(this->ConstrainFireAnimsToCellSpots)
		.Process(this->FireAnimDisallowedLandTypes)
		.Process(this->AttachFireAnimsToParent)
		.Process(this->SmallFireCount)
		.Process(this->SmallFireAnims)
		.Process(this->SmallFireChances)
		.Process(this->SmallFireDistances)
		.Process(this->LargeFireCount)
		.Process(this->LargeFireAnims)
		.Process(this->LargeFireChances)
		.Process(this->LargeFireDistances)
		.Process(this->Crater_ReduceTiberium)
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

	AnimTypeExt::ExtMap.TryAllocate(pItem);
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

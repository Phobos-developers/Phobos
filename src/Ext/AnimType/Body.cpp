#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>

AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;

void AnimTypeExt::ProcessDestroyAnims(UnitClass* pThis, HouseClass* pKiller)
{
	if (!pThis)
		return;

	auto const pType = pThis->Type;

	if (pType->DestroyAnim.Count > 0)
	{
		auto const facing = pThis->PrimaryFacing.Current().GetDir();
		AnimTypeClass* pAnimType = nullptr;
		auto const pTypeExt = TechnoTypeExt::Fetch(pType);

		if (!pTypeExt->DestroyAnim_Random.Get(RulesExt::Global()->DestroyAnim_Random))
		{
			int idxAnim = 0;

			if (pType->DestroyAnim.Count >= 8)
			{
				idxAnim = pType->DestroyAnim.Count - 1;
				if (pType->DestroyAnim.Count % 2 == 0)
					idxAnim = static_cast<int>(static_cast<unsigned char>(facing) / 256.0 * idxAnim);
			}

			pAnimType = pType->DestroyAnim[idxAnim];
		}
		else
		{
			int const nIDx_Rand = pType->DestroyAnim.Count == 1
				? 0 : ScenarioClass::Instance->Random.RandomRanged(0, (pType->DestroyAnim.Count - 1));
			pAnimType = pType->DestroyAnim[nIDx_Rand];
		}

		if (pAnimType)
		{
			auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
			auto const pInvoker = pKiller;

			//auto VictimOwner = pThis->IsMindControlled() && pThis->GetOriginalOwner()
			//	? pThis->GetOriginalOwner() : pThis->Owner;

			auto const pAnimTypeExt = AnimTypeExt::Fetch(pAnim->Type);
			auto const pAnimExt = AnimExt::Fetch(pAnim);

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

void AnimTypeExt::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;

	INI_EX exINI(pINI);

	this->Palette.LoadFromINI(pINI, pID, "CustomPalette");
	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->XDrawOffset_ApplyBracketWidth.Read(exINI, pID, "XDrawOffset.ApplyBracketWidth");
	this->XDrawOffset_InvertBracketShift.Read(exINI, pID, "XDrawOffset.InvertBracketShift");
	this->XDrawOffset_BracketAdjust.Read(exINI, pID, "XDrawOffset.BracketAdjust");
	this->XDrawOffset_BracketAdjust_Buildings.Read(exINI, pID, "XDrawOffset.BracketAdjust.Buildings");
	this->YDrawOffset_ApplyBracketHeight.Read(exINI, pID, "YDrawOffset.ApplyBracketHeight");
	this->YDrawOffset_InvertBracketShift.Read(exINI, pID, "YDrawOffset.InvertBracketShift");
	this->YDrawOffset_BracketAdjust.Read(exINI, pID, "YDrawOffset.BracketAdjust");
	this->YDrawOffset_BracketAdjust_Buildings.Read(exINI, pID, "YDrawOffset.BracketAdjust.Buildings");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");
	this->AttachedAnimPosition.Read(exINI, pID, "AttachedAnimPosition");
	this->Weapon.Read<true>(exINI, pID, "Weapon");
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOncePerLoop.Read(exINI, pID, "Damage.ApplyOncePerLoop");
	this->Damage_ApplyFirepowerMult.Read(exINI, pID, "Damage.ApplyFirepowerMult");
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
	this->Translucency.Read(exINI, pID, "Translucency", this->OwnerObject()->End);
	this->Translucency_Cloaked.Read(exINI, pID, "Translucency.Cloaked", this->OwnerObject()->End, true);
	this->ConstrainFireAnimsToCellSpots.Read(exINI, pID, "ConstrainFireAnimsToCellSpots");
	this->FireAnimDisallowedLandTypes.Read<false, true>(exINI, pID, "FireAnimDisallowedLandTypes");
	this->AttachFireAnimsToParent.Read(exINI, pID, "AttachFireAnimsToParent");
	this->SmallFireCount.Read(exINI, pID, "SmallFireCount");
	this->SmallFireAnims.Read(exINI, pID, "SmallFireAnims");
	this->SmallFireChances.Read(exINI, pID, "SmallFireChances");
	this->SmallFireDistances.Read(exINI, pID, "SmallFireDistances");
	this->LargeFireCount.Read(exINI, pID, "LargeFireCount");
	this->LargeFireAnims.Read(exINI, pID, "LargeFireAnims");
	this->LargeFireChances.Read(exINI, pID, "LargeFireChances");
	this->LargeFireDistances.Read(exINI, pID, "LargeFireDistances");
	this->Crater_DestroyTiberium.Read(exINI, pID, "Crater.DestroyTiberium");
	this->TheaterPalette.Read(exINI, pID, "TheaterPalette");
	this->Tiled_Interval.Read(exINI, pID, "Tiled.Interval");
	this->Tiled_AlignToCenter.Read(exINI, pID, "Tiled.AlignToCenter");

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
void AnimTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->CreateUnitType)
		.Process(this->XDrawOffset)
		.Process(this->XDrawOffset_ApplyBracketWidth)
		.Process(this->XDrawOffset_InvertBracketShift)
		.Process(this->XDrawOffset_BracketAdjust)
		.Process(this->XDrawOffset_BracketAdjust_Buildings)
		.Process(this->YDrawOffset_ApplyBracketHeight)
		.Process(this->YDrawOffset_InvertBracketShift)
		.Process(this->YDrawOffset_BracketAdjust)
		.Process(this->YDrawOffset_BracketAdjust_Buildings)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->AttachedAnimPosition)
		.Process(this->Weapon)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOncePerLoop)
		.Process(this->Damage_ApplyFirepowerMult)
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
		.Process(this->Translucency)
		.Process(this->Translucency_Cloaked)
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
		.Process(this->Crater_DestroyTiberium)
		.Process(this->TheaterPalette)
		.Process(this->Tiled_Interval)
		.Process(this->Tiled_AlignToCenter)
		;
}

void AnimTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	ObjectTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	ObjectTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

namespace detail
{
	template <>
	inline bool read<AttachedAnimPosition>(AttachedAnimPosition& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();
			if (_strcmpi(str, "default") == 0)
			{
				value = AttachedAnimPosition::Default;
			}
			else if (_strcmpi(str, "center") == 0 || _strcmpi(str, "centre") == 0)
			{
				value = AttachedAnimPosition::Center;
			}
			else if (_strcmpi(str, "ground") == 0)
			{
				value = AttachedAnimPosition::Ground;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "Expected attached animation position type");
				return false;
			}
			return true;
		}
		return false;
	}
}

// =============================
// container

AnimTypeExt::ExtContainer::ExtContainer() : Container("AnimTypeClass") { }
AnimTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

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

//DEFINE_HOOK_AGAIN(0x4287E9, AnimTypeClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI, 0xA)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

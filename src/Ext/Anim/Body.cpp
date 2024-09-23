#include "Body.h"

#include <GameOptionsClass.h>

#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Misc/SyncLogging.h>

AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::SetInvoker(TechnoClass* pInvoker)
{
	this->Invoker = pInvoker;
	this->InvokerHouse = pInvoker ? pInvoker->Owner : nullptr;
}

void AnimExt::ExtData::SetInvoker(TechnoClass* pInvoker, HouseClass* pInvokerHouse)
{
	this->Invoker = pInvoker;
	this->InvokerHouse = pInvokerHouse;
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
bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner, bool defaultToInvokerOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	bool makeInf = pAnim->Type->MakeInfantry > -1;
	bool createUnit = pTypeExt->CreateUnit.Get();
	auto ownerKind = OwnerHouseKind::Default;
	HouseClass* pDefaultOwner = nullptr;

	if (defaultToVictimOwner)
		pDefaultOwner = pVictim;
	else if (defaultToInvokerOwner)
		pDefaultOwner = pInvoker;

	if (makeInf)
		ownerKind = pTypeExt->MakeInfantryOwner;

	if (createUnit)
		ownerKind = pTypeExt->CreateUnit_Owner;

	auto newOwner = HouseExt::GetHouseKind(ownerKind, true, pDefaultOwner, pInvoker, pVictim);

	if (newOwner)
	{
		pAnim->Owner = newOwner;
		bool isRemappable = false;

		if (makeInf)
			isRemappable = true;

		if (createUnit)
			isRemappable = pTypeExt->CreateUnit_RemapAnim;

		if (isRemappable && !newOwner->Defeated)
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

void AnimExt::VeinAttackAI(AnimClass* pAnim)
{
	CellStruct pCoordinates = pAnim->GetMapCoords();
	CellClass* pCell = MapClass::Instance->GetCellAt(pCoordinates);
	ObjectClass* pOccupier = pCell->FirstObject;
	constexpr unsigned char fullyFlownWeedStart = 0x30; // Weeds starting from this overlay frame are fully grown
	constexpr unsigned int weedOverlayIndex = 126;

	if (!pOccupier || pOccupier->GetHeight() > 0 || pCell->OverlayTypeIndex != weedOverlayIndex
		|| pCell->OverlayData < fullyFlownWeedStart || pCell->SlopeIndex)
	{
		pAnim->UnableToContinue = true;
	}

	if (Unsorted::CurrentFrame % 2 == 0)
	{
		while (pOccupier != nullptr)
		{
			ObjectClass* pNext = pOccupier->NextObject;
			int damage = RulesClass::Instance->VeinDamage;
			TechnoClass* pTechno = abstract_cast<TechnoClass*>(pOccupier);

			if (pTechno && !pTechno->GetTechnoType()->ImmuneToVeins && !pTechno->HasAbility(Ability::VeinProof)
				&& pTechno->Health > 0 && pTechno->IsAlive && pTechno->GetHeight() <= 5)
			{
				pTechno->ReceiveDamage(&damage, 0, RulesExt::Global()->VeinholeWarhead, nullptr, false, false, nullptr);
			}

			pOccupier = pNext;
		}
	}
}

// Changes type of anim in similar fashion to Next.
void AnimExt::ChangeAnimType(AnimClass* pAnim, AnimTypeClass* pNewType, bool resetLoops, bool restart)
{
	double percentThrough = pAnim->Animation.Value / static_cast<double>(pAnim->Type->End);

	if (pNewType->End == -1)
	{
		pNewType->End = pNewType->GetImage()->Frames;

		if (pNewType->Shadow)
			pNewType->End /= 2;
	}

	if (pNewType->LoopEnd == -1)
	{
		pNewType->LoopEnd = pNewType->End;
	}

	pAnim->Type = pNewType;

	if (resetLoops)
		pAnim->RemainingIterations = static_cast<byte>(pNewType->LoopCount);

	pAnim->Accum = 0;
	pAnim->UnableToContinue = false;
	pAnim->Reverse = pNewType->Reverse;

	int rate = pNewType->Rate;

	if (pNewType->RandomRate.Min || pNewType->RandomRate.Max)
		rate = ScenarioClass::Instance->Random.RandomRanged(pNewType->RandomRate.Min, pNewType->RandomRate.Max);

	if (pNewType->Normalized)
		rate = GameOptionsClass::Instance->GetAnimSpeed(rate);

	pAnim->Animation.Start(rate, pNewType->Reverse ? -1 : 1);

	if (restart)
	{
		pAnim->Animation.Value = pNewType->Reverse ? pNewType->End : pNewType->Start;
		pAnim->Start();
	}
	else
	{
		pAnim->Animation.Value = static_cast<int>(pNewType->End * percentThrough);
	}
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
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false, true);
		}
	}
	else
	{
		if (!isMeteor)
			pWakeAnimToUse = RulesClass::Instance->Wake;

		if (pWakeAnim)
			pWakeAnimToUse = pWakeAnim;

		if (!splashAnims.empty())
		{
			auto const lastIndex = (splashAnims.size() - 1);
			auto const defaultIndex = isMeteor ? lastIndex : 0;
			auto const animIndex = splashAnimsPickRandom ?
				ScenarioClass::Instance->Random.RandomRanged(0, lastIndex) : defaultIndex;

			pSplashAnimToUse = splashAnims.at(animIndex);
		}
	}

	if (pWakeAnimToUse)
	{
		if (auto const pWakeAnimCreated = GameCreate<AnimClass>(pWakeAnimToUse, nLocation, 0, 1, 0x600u, false))
			AnimExt::SetAnimOwnerHouseKind(pWakeAnimCreated, pOwner, nullptr, false, true);
	}

	if (pSplashAnimToUse)
	{
		if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnimToUse, nLocation, 0, 1, 0x600u, false))
			AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false, true);
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
		.Process(this->ParentBuilding)
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
	// Something about creating this in constructor messes with debris anims, so it has to be done for them later.
	if (!this->OwnerObject()->HasExtras)
		CreateAttachedSystem();
}

void AnimExt::InvalidateTechnoPointers(TechnoClass* pTechno)
{
	for (auto const& pAnim : *AnimClass::Array)
	{
		auto const pExt = AnimExt::ExtMap.Find(pAnim);

		if (pExt->Invoker == pTechno)
			pExt->Invoker = nullptr;

		if ((TechnoClass*)pExt->ParentBuilding == pTechno)
			pExt->ParentBuilding = nullptr;
	}
}

void AnimExt::InvalidateParticleSystemPointers(ParticleSystemClass* pParticleSystem)
{
	for (auto const& pAnim : *AnimClass::Array)
	{
		auto const pExt = AnimExt::ExtMap.Find(pAnim);

		if (pExt->AttachedSystem == pParticleSystem)
			pExt->AttachedSystem = nullptr;
	}
}

// =============================
// container

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

namespace CTORTemp
{
	CoordStruct coords;
	unsigned int callerAddress;
}

DEFINE_HOOK(0x421EA0, AnimClass_CTOR_SetContext, 0x6)
{
	GET_STACK(CoordStruct*, coords, 0x8);
	GET_STACK(unsigned int, callerAddress, 0x0);

	CTORTemp::coords = *coords;
	CTORTemp::callerAddress = callerAddress;

	return 0;
}

DEFINE_HOOK_AGAIN(0x422126, AnimClass_CTOR, 0x5)
DEFINE_HOOK_AGAIN(0x422707, AnimClass_CTOR, 0x5)
DEFINE_HOOK(0x4228D2, AnimClass_CTOR, 0x5)
{
	GET(AnimClass*, pItem, ESI);

	if (!Phobos::IsLoadingSaveGame)
	{
		auto const callerAddress = CTORTemp::callerAddress;

		// Do this here instead of using a duplicate hook in SyncLogger.cpp
		if (!SyncLogger::HooksDisabled && pItem->UniqueID != -2)
			SyncLogger::AddAnimCreationSyncLogEvent(CTORTemp::coords, callerAddress);

		if (pItem && !pItem->Type)
		{
			Debug::Log("Attempting to create animation with null Type (Caller: %08x)!\n", callerAddress);
			return 0;
		}
	}

	AnimExt::ExtMap.Allocate(pItem);

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

// Field D0 in AnimClass is mostly unused so by removing the few uses it has it can be used to store AnimExt pointer.
DEFINE_JUMP(LJMP, 0x42543A, 0x425448)

DEFINE_HOOK_AGAIN(0x421EF4, AnimClass_CTOR_ClearD0, 0x6)
DEFINE_HOOK(0x42276D, AnimClass_CTOR_ClearD0, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	pThis->unknown_D0 = 0;
	return R->Origin() + 0x6;
}

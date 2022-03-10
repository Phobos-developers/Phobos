#include "Body.h"

#include <Ext/TechnoType/Body.h>

template<> const DWORD Extension<CaptureExt::base_type>::Canary = 0x87654121;
CaptureExt::ExtContainer CaptureExt::ExtMap;

void CaptureExt::ExtData::InitializeConstants()
{
	if (auto const pOwner = OwnerObject()->Owner)
	{
		if (auto const pManagerOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType()))
		{
			OverloadCount = pManagerOwnerExt->Overload_Count.Get(RulesClass::Instance->OverloadCount);
			OverloadFrames = pManagerOwnerExt->Overload_Frames.Get(RulesClass::Instance->OverloadFrames);
			OverloadDamage = pManagerOwnerExt->Overload_Damage.Get(RulesClass::Instance->OverloadDamage);
			OverloadDeathSound = pManagerOwnerExt->Overload_DeathSound.Get(RulesClass::Instance->MasterMindOverloadDeathSound);
		}
	}
}

// =============================
// load / save

template <typename T>
void CaptureExt::ExtData::Serialize(T& Stm)
{
	Stm
		//No need
		// will cause crash , everything wil be re-init ed later
		//.Process(OverloadCount)
		//.Process(OverloadFrames)
		//.Process(OverloadDamage)
		//.Process(OverloadDeathSound)
		;
}

void CaptureExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CaptureExt::base_type>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CaptureExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CaptureExt::base_type>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void CaptureExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool CaptureExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool CaptureExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}
// =============================
// container

CaptureExt::ExtContainer::ExtContainer() : Container("CaptureManagerClass") { };
CaptureExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x471832, CaptureManagerClass_CTOR, 0x9)
{
	GET(CaptureManagerClass* const, pThis, ESI);
	CaptureExt::ExtMap.FindOrAllocate(pThis);
	return 0;
}

DEFINE_HOOK(0x4729E1, CaptureManagerClass_DTOR, 0xD)
{
	GET(CaptureManagerClass* const, pThis, ESI);

	if (auto const& pExt = CaptureExt::ExtMap.Find(pThis))
		pExt->CleanUp();

	CaptureExt::ExtMap.Remove(pThis);
	return 0;
}

DEFINE_HOOK_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(CaptureManagerClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	CaptureExt::ExtMap.PrepareStream(pThis, pStm);

	return 0;
}

DEFINE_HOOK(0x4728CA, CaptureManagerClass_Load_Suffix, 0xA)
{
	CaptureExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
{
	CaptureExt::ExtMap.SaveStatic();
	return 0;
}

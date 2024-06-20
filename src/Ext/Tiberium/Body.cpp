#include "Body.h"

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// =============================
// load / save

template <typename T>
void TiberiumExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->MinimapColor)
		.Process(this->Overlays)
		.Process(this->Overlays_UseSlopes)
		.Process(this->Overlays_FrameCount)
		;
}

void TiberiumExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");

	this->Overlays_UseSlopes.Read(exINI, pSection, "Overlays.UseSlopes");
	this->Overlays_FrameCount.Read(exINI, pSection, "Overlays.FrameCount");
	this->Overlays.Read(exINI, pSection, "Overlays");

	if (!this->Overlays.empty())
	{
		const int overlayCount = this->Overlays.size();

		pThis->NumFrames = this->Overlays_FrameCount;
		pThis->field_EC = 0;
		pThis->NumImages = overlayCount;

		if (this->Overlays_UseSlopes)
		{
			constexpr int rampCount = 8;

			if (overlayCount <= rampCount)
			{
				Debug::Log("Tiberium %s is set to use slopes, but has only got %d frames (need at least 9), turning slopes off!", pThis->ID, overlayCount);
			}
			else
			{
				pThis->field_EC = rampCount;
				pThis->NumImages -= rampCount;
			}
		}
	}
}

void TiberiumExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TiberiumClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TiberiumExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TiberiumClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TiberiumExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TiberiumExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TiberiumExt::ExtContainer::ExtContainer() : Container("TiberiumClass") { }
TiberiumExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x721876, TiberiumClass_CTOR, 0x5)
{
	GET(TiberiumClass*, pItem, ESI);

	TiberiumExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x721888, TiberiumClass_DTOR, 0x6)
{
	GET(TiberiumClass*, pItem, ECX);

	TiberiumExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x721E80, TiberiumClass_SaveLoad_Prefix, 0x7)
DEFINE_HOOK(0x7220D0, TiberiumClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TiberiumClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TiberiumExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x72208C, TiberiumClass_Load_Suffix, 0x7)
{
	TiberiumExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x72212C, TiberiumClass_Save_Suffix, 0x5)
{
	TiberiumExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xC4, 0x4));

	TiberiumExt::ExtMap.LoadFromINI(pItem, pINI);

	if (!TiberiumExt::ExtMap.Find(pItem)->Overlays.empty())
	{
		if (auto pOverlay = TiberiumExt::ExtMap.Find(pItem)->Overlays[0])
		{
			pItem->Image = pOverlay; 
		}
	}

	return 0;
}

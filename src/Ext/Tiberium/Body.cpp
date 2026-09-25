#include "Body.h"
#include <OverlayTypeClass.h>

#include <algorithm>
#include <string>

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// =============================
// load / save

template <typename T>
void TiberiumExt::Serialize(T& Stm)
{
	Stm
		.Process(this->MinimapColor)
		.Process(this->AllowRamps)
		.Process(this->PipFrame)
		;
}

void TiberiumExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");
	this->AllowRamps.Read(exINI, pSection, "AllowRamps");
	this->PipFrame.Read(exINI, pSection, "PipFrame");

	char overlayBuf[32] = { 0 };
	pINI->ReadString(pSection, "Overlay", "", overlayBuf, sizeof(overlayBuf));

	char prefixBuf[32] = { 0 };
	pINI->ReadString(pSection, "OverlayPrefix", "", prefixBuf, sizeof(prefixBuf));

	int variety = pINI->ReadInteger(pSection, "Variety", pThis->NumImages > 0 ? pThis->NumImages : 12);
	pThis->NumImages = std::max(1, variety);

	OverlayTypeClass* pOvl = nullptr;
	if (overlayBuf[0] != '\0')
	{
		pOvl = OverlayTypeClass::Find(overlayBuf);
	}
	else if (prefixBuf[0] != '\0')
	{
		char buf[32];
		_snprintf_s(buf, sizeof(buf), "%s01", prefixBuf);
		pOvl = OverlayTypeClass::Find(buf);
		if (!pOvl)
		{
			_snprintf_s(buf, sizeof(buf), "%s1", prefixBuf);
			pOvl = OverlayTypeClass::Find(buf);
		}
	}

	if (pOvl)
	{
		pThis->Image = pOvl;
		pThis->NumFrames = 12;
	}

	if (this->AllowRamps && pThis->NumSlopes < 8)
		pThis->NumSlopes = 8;
}

void TiberiumExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TiberiumExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
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

//DEFINE_HOOK_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xC4, 0x4));

	TiberiumExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

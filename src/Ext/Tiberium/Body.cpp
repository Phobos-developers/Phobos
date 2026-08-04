#include "Body.h"

#include <OverlayTypeClass.h>

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// =============================
// load / save

template <typename T>
void TiberiumExt::Serialize(T& Stm)
{
	Stm
		.Process(this->MinimapColor)
		.Process(this->CustomImageName)
		.Process(this->CustomImageNumFrames)
		.Process(this->CustomImageNumImages)
		.Process(this->CustomImageNumSlopes)
		;
}

void TiberiumExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");

	// Load CustomImage settings
	this->CustomImageName.Read(pINI, pSection, "CustomImage");
	this->CustomImageNumFrames.Read(exINI, pSection, "CustomImage.NumFrames");
	this->CustomImageNumImages.Read(exINI, pSection, "CustomImage.NumImages");
	this->CustomImageNumSlopes.Read(exINI, pSection, "CustomImage.NumSlopes");
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

// Helper function to apply CustomImage to a TiberiumClass
static void ApplyCustomImage(TiberiumClass* pItem)
{
	if (auto pExt = TiberiumExt::TryFetch(pItem))
	{
		if (pExt->CustomImageName)
		{
			auto pOverlayType = OverlayTypeClass::Find(pExt->CustomImageName);
			if (pOverlayType)
			{
				pItem->Image = pOverlayType;

				pItem->NumFrames = pExt->CustomImageNumFrames.isset()
					? pExt->CustomImageNumFrames.Get() : 12;

				pItem->NumImages = pExt->CustomImageNumImages.isset()
					? pExt->CustomImageNumImages.Get() : 12;

				pItem->NumSlopes = pExt->CustomImageNumSlopes.isset()
					? pExt->CustomImageNumSlopes.Get() : 0;
			}
		}
	}
}

DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xC4, 0x4));

	TiberiumExt::ExtMap.LoadFromINI(pItem, pINI);

	// Apply CustomImage after game sets Image
	ApplyCustomImage(pItem);

	return 0;
}

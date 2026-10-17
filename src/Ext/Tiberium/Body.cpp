#include "Body.h"

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// =============================
// load / save

template <typename T>
void TiberiumExt::Serialize(T& Stm)
{
	Stm
		.Process(this->MinimapColor)
		.Process(this->AllowRamps)
		;
}

bool TiberiumExt::HasRampOverlays()
{
	if (!this->RampOverlays[0])
		this->AutoDetectRampOverlays();

	for (const auto pOverlay : this->RampOverlays)
	{
		if (!pOverlay)
			return false;
	}

	return true;
}

OverlayTypeClass* TiberiumExt::GetRampOverlay(int slopeOffset)
{
	if (slopeOffset >= 0 && slopeOffset < 8)
	{
		if (!this->RampOverlays[0])
			this->AutoDetectRampOverlays();

		return this->RampOverlays[slopeOffset];
	}

	return nullptr;
}

void TiberiumExt::AutoDetectRampOverlays()
{
	for (auto& pOverlay : this->RampOverlays)
		pOverlay = nullptr;

	auto pThis = this->OwnerObject();
	if (!pThis || !pThis->Image)
		return;

	// 1. Suffix-based automatic detection:
	// Extract prefix from Image->ID (e.g. "GEM01" -> "GEM")
	std::string imageId = pThis->Image->ID;
	while (!imageId.empty() && std::isdigit(static_cast<unsigned char>(imageId.back())))
		imageId.pop_back();

	if (!imageId.empty())
	{
		bool allFound = true;
		for (int i = 13; i <= 20; ++i)
		{
			char buf[64];
			sprintf_s(buf, "%s%d", imageId.c_str(), i);
			auto pFound = OverlayTypeClass::Find(buf);
			if (!pFound)
			{
				allFound = false;
				break;
			}

			this->RampOverlays[i - 13] = pFound;
		}

		if (allFound)
		{
			for (auto pOverlay : this->RampOverlays)
				pOverlay->Tiberium = true;

			if (pThis->NumSlopes < 8)
				pThis->NumSlopes = 8;

			return;
		}
	}

	// 2. Fallback to contiguous overlays if present:
	const int baseIdx = pThis->Image->ArrayIndex + pThis->NumFrames;
	if (baseIdx + 8 <= OverlayTypeClass::Array.Count)
	{
		bool allContiguousValid = true;
		for (int i = 0; i < 8; ++i)
		{
			auto pContiguous = OverlayTypeClass::Array.GetItem(baseIdx + i);
			if (!pContiguous || !pContiguous->Tiberium)
			{
				allContiguousValid = false;
				break;
			}
		}

		if (allContiguousValid)
		{
			for (int i = 0; i < 8; ++i)
				this->RampOverlays[i] = OverlayTypeClass::Array.GetItem(baseIdx + i);

			if (pThis->NumSlopes < 8)
				pThis->NumSlopes = 8;
		}
	}
}

void TiberiumExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");
	this->AllowRamps.Read(exINI, pSection, "AllowRamps");

	if (this->AllowRamps)
		this->AutoDetectRampOverlays();
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

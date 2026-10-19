#include "Body.h"
#include <OverlayTypeClass.h>

#include <cctype>
#include <string>
#include <vector>

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

	const int variety = pThis->NumImages > 0 ? pThis->NumImages : 12;
	const int startRamp = variety + 1;
	const int endRamp = variety + 8;

	auto extractPrefix = [](const char* pStr) -> std::string
	{
		if (!pStr || !pStr[0])
			return {};

		std::string prefix = pStr;
		while (!prefix.empty() && std::isdigit(static_cast<unsigned char>(prefix.back())))
			prefix.pop_back();

		return prefix;
	};

	std::vector<std::string> prefixes;
	if (auto idPrefix = extractPrefix(pThis->Image->ID); !idPrefix.empty())
		prefixes.push_back(std::move(idPrefix));

	if (auto imgPrefix = extractPrefix(pThis->Image->ImageFile); !imgPrefix.empty())
	{
		if (prefixes.empty() || _stricmp(prefixes.front().c_str(), imgPrefix.c_str()) != 0)
			prefixes.push_back(std::move(imgPrefix));
	}

	for (const auto& prefix : prefixes)
	{
		bool allFound = true;
		for (int i = startRamp; i <= endRamp; ++i)
		{
			char buf[64];
			sprintf_s(buf, "%s%02d", prefix.c_str(), i);
			auto pFound = OverlayTypeClass::Find(buf);
			if (!pFound)
			{
				sprintf_s(buf, "%s%d", prefix.c_str(), i);
				pFound = OverlayTypeClass::Find(buf);
			}

			if (!pFound)
			{
				allFound = false;
				break;
			}

			this->RampOverlays[i - startRamp] = pFound;
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

	// Fallback detection for contiguous overlays following standard Tiberian Sun indexing.
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

			return;
		}
	}

	// Ensure cardinal slope count is initialized if ramp germination is explicitly enabled.
	if (this->AllowRamps && pThis->NumSlopes < 8)
		pThis->NumSlopes = 8;
}

void TiberiumExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");
	this->AllowRamps.Read(exINI, pSection, "AllowRamps");

	if (this->AllowRamps)
	{
		this->AutoDetectRampOverlays();
		if (pThis->NumSlopes < 8)
			pThis->NumSlopes = 8;
	}
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

#include "Body.h"
#include <OverlayTypeClass.h>

#include <algorithm>
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
		.Process(this->PipFrame)
		;
}

bool TiberiumExt::HasRampOverlays()
{
	if (!this->AllowRamps)
		return false;

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
	if (this->AllowRamps && slopeOffset >= 0 && slopeOffset < 8)
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
	{
		this->AllowRamps = false;
		if (pThis)
			pThis->NumSlopes = 0;

		return;
	}

	const int variety = pThis->NumImages > 0 ? pThis->NumImages : 12;

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
	if (auto imgPrefix = extractPrefix(pThis->Image->ImageFile); !imgPrefix.empty())
		prefixes.push_back(std::move(imgPrefix));

	if (auto idPrefix = extractPrefix(pThis->Image->ID); !idPrefix.empty())
	{
		if (prefixes.empty() || _stricmp(prefixes.front().c_str(), idPrefix.c_str()) != 0)
			prefixes.push_back(std::move(idPrefix));
	}

	const int rampBases[] = { variety + 1, 13 };

	for (const int startRamp : rampBases)
	{
		const int endRamp = startRamp + 7;

		for (const auto& prefix : prefixes)
		{
			std::array<OverlayTypeClass*, 8> candidateOverlays {};
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

				if (pFound->ImageFile && pFound->ImageFile[0])
				{
					char numSuffix[16];
					sprintf_s(numSuffix, "%02d", i);
					const size_t fileLen = strlen(pFound->ImageFile);
					const size_t sufLen = strlen(numSuffix);
					if (fileLen < sufLen || _stricmp(pFound->ImageFile + fileLen - sufLen, numSuffix) != 0)
					{
						if (auto altPrefix = extractPrefix(pFound->ImageFile); !altPrefix.empty())
						{
							char altBuf[64];
							sprintf_s(altBuf, "%s%02d", altPrefix.c_str(), i);
							if (auto pAlt = OverlayTypeClass::Find(altBuf))
								pFound = pAlt;
						}
					}
				}

				candidateOverlays[i - startRamp] = pFound;
			}

			if (allFound)
			{
				this->RampOverlays = candidateOverlays;
				for (auto pOverlay : this->RampOverlays)
					pOverlay->Tiberium = true;

				if (pThis->NumSlopes < 8)
					pThis->NumSlopes = 8;

				return;
			}
		}
	}

	// Fallback detection for contiguous overlays following standard Tiberian Sun indexing.
	const int baseIdx = pThis->Image->ArrayIndex + variety;
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

	// If any of the 8 ramp overlay pieces is missing, safely disable ramp support.
	this->AllowRamps = false;
	pThis->NumSlopes = 0;
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

	if (overlayBuf[0] == '\0' && prefixBuf[0] == '\0' && !pThis->Image)
	{
		strncpy_s(prefixBuf, pSection, _TRUNCATE);
		_strupr_s(prefixBuf);
	}

	const int variety = pINI->ReadInteger(pSection, "Variety", pThis->NumImages > 0 ? pThis->NumImages : 12);
	pThis->NumImages = std::max(1, variety);

	const int numFrames = pINI->ReadInteger(pSection, "NumFrames", pThis->NumFrames > 0 ? pThis->NumFrames : 12);
	pThis->NumFrames = std::max(1, numFrames);

	OverlayTypeClass* pOvl = nullptr;
	if (overlayBuf[0] != '\0')
	{
		pOvl = OverlayTypeClass::Find(overlayBuf);
		if (pOvl)
		{
			pOvl->Tiberium = true;
			for (int i = 1; i < pThis->NumImages; ++i)
			{
				if (const auto pNext = OverlayTypeClass::Array.GetItemOrDefault(pOvl->ArrayIndex + i))
					pNext->Tiberium = true;
			}
		}
	}
	else if (prefixBuf[0] != '\0')
	{
		for (int i = 1; i <= pThis->NumImages; ++i)
		{
			char buf[32];
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s%02d", prefixBuf, i);
			auto pFound = OverlayTypeClass::Find(buf);
			if (!pFound)
			{
				_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s%d", prefixBuf, i);
				pFound = OverlayTypeClass::Find(buf);
			}

			if (pFound)
			{
				pFound->Tiberium = true;
				if (i == 1)
					pOvl = pFound;
			}
		}
	}

	if (pOvl)
		pThis->Image = pOvl;

	if (this->AllowRamps)
	{
		this->AutoDetectRampOverlays();
	}
	else
	{
		for (auto& pOverlay : this->RampOverlays)
			pOverlay = nullptr;

		pThis->NumSlopes = 0;
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

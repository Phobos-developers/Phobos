#include <Phobos.h>

#include <StringTable.h>

#include <Utilities/Macro.h>

#include <deque>
#include <string_view>
#include <unordered_map>

// StringTable::ReadFile normalizes every CSF label value before storing it:
// runs of consecutive spaces are collapsed into one, leading/trailing spaces
// and the spaces adjacent to newlines or tabs are removed (the loop at 0x734B4B
// rewrites the decoded value in place). The raw values are captured here
// unconditionally while a CSF is parsed and handed out by StringTable::LoadString
// instead of the engine's normalized copies when
// Phobos::Config::PreserveCSFWhitespace ([Phobos] PreserveCSFWhitespace in
// RA2MD.INI, read in OptionsClass::LoadSettings) is enabled.
//
// Raw values are kept in a deque so that a string already returned to the
// engine stays valid even if its label is overridden by a CSF loaded later.

namespace CSFTextExt
{
	struct CaseInsensitiveHash
	{
		using is_transparent = void;

		size_t operator()(std::string_view value) const noexcept
		{
			// FNV-1a over an ASCII case-folded string: the engine itself looks
			// labels up with strcmpi.
			size_t hash = 2166136261u;

			for (char ch : value)
			{
				auto c = static_cast<unsigned char>(ch);

				if (c >= 'A' && c <= 'Z')
					c = static_cast<unsigned char>(c - 'A' + 'a');

				hash = (hash ^ c) * 16777619u;
			}

			return hash;
		}
	};

	struct CaseInsensitiveEqual
	{
		using is_transparent = void;

		bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
		{
			return lhs.size() == rhs.size() && _strnicmp(lhs.data(), rhs.data(), lhs.size()) == 0;
		}
	};

	static std::deque<std::wstring> RawValues;
	static std::unordered_map<std::string, size_t, CaseInsensitiveHash, CaseInsensitiveEqual> RawValueIndices;
	static int CurrentLabelIndex = -1;

	void Clear()
	{
		RawValues.clear();
		RawValueIndices.clear();
		CurrentLabelIndex = -1;
	}

	void CaptureRawValue(REGISTERS* R)
	{
		if (!StringTable::Labels || CurrentLabelIndex < 0 || CurrentLabelIndex >= StringTable::LabelCount)
			return;

		auto const pLabel = &StringTable::Labels[CurrentLabelIndex];

		// StringTable::LoadString only ever returns the first value of a label,
		// the remaining ones are never used by the engine.
		if (static_cast<int>(R->EBP()) / 4 != pLabel->FirstValueIndex)
			return;

		auto const pRawValue = reinterpret_cast<const wchar_t*>(0x00B17F24);

		RawValues.emplace_back(pRawValue);
		RawValueIndices[pLabel->Name] = RawValues.size() - 1;
	}

	const wchar_t* GetRawValue(const char* pLabel)
	{
		if (!pLabel)
			return nullptr;

		auto const it = RawValueIndices.find(std::string_view { pLabel });

		if (it == RawValueIndices.end())
			return nullptr;

		return RawValues[it->second].c_str();
	}
}

// EBP is the byte offset of the current label in StringTable::Labels at this
// point; it is overwritten with the value index right before the value loop.
DEFINE_HOOK(0x734A88, StringTable_ReadFile_TrackCurrentLabel, 0x6)
{
	CSFTextExt::CurrentLabelIndex = static_cast<int>(R->EBP()) / static_cast<int>(sizeof(CSFLabel));
	return 0;
}

DEFINE_HOOK(0x734B4B, StringTable_ReadFile_CaptureRawValue, 0x5)
{
	CSFTextExt::CaptureRawValue(R);
	return 0;
}

DEFINE_HOOK(0x734D30, StringTable_Unload_ClearRawValues, 0x5)
{
	CSFTextExt::Clear();
	return 0;
}

DEFINE_HOOK(0x734E70, StringTable_LoadString_UseRawValue, 0x5)
{
	// Callers that ask for the extra ("WRTS") data keep the engine's own path.
	if (!Phobos::Config::PreserveCSFWhitespace || R->ESI() != 0)
		return 0;

	if (auto const pRawValue = CSFTextExt::GetRawValue(reinterpret_cast<const char*>(R->EBX())))
	{
		R->EAX(pRawValue);
		return 0x734F0F;
	}

	return 0;
}

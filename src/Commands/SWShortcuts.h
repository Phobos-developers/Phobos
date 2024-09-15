#pragma once

#include "Commands.h"

#include <Utilities/GeneralUtils.h>
#include <Misc/TacticalButtons.h>

// Super Weapon Sidebar Keyboard shortcut command class
template<size_t KeyIndex>
class SWShortcutsCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t KeyIndex>
inline const char* SWShortcutsCommandClass<KeyIndex>::GetName() const
{
	class to_string_t
	{
	public:
		char buffer[27];

	public:
		constexpr to_string_t() noexcept
			: buffer { "SW Sidebar Shortcuts Num 0" }
		{
			buffer[25] = KeyIndex + '0';
		}

		constexpr operator char* () noexcept { return buffer; }
	};
	static to_string_t name;
	return name;
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIName() const
{
	class to_string_t
	{
	public:
		wchar_t buffer[18];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Quick Select SW 0" }
		{
			buffer[16] = KeyIndex + '0';
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t name;
	return StringTable::TryFetchString("TXT_SW_XX_FORWARD", name);
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIDescription() const
{
	class to_string_t
	{
	public:
		wchar_t buffer[31];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Select No.0 SW in left sidebar" }
		{
			buffer[10] = KeyIndex + '0';
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t name;
	return StringTable::TryFetchString("TXT_SW_XX_FORWARD_DESC", name);
}

template<size_t KeyIndex>
inline void SWShortcutsCommandClass<KeyIndex>::Execute(WWKey eInput) const
{
	TacticalButtonClass::TriggerButtonForSW(KeyIndex);
}

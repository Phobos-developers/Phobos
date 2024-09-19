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
	if (KeyIndex > 0)
	{
		class to_string_t
		{
		public:
			char buffer[29];

		public:
			constexpr to_string_t() noexcept
				: buffer { "" }
			{
				sprintf_s(buffer, "SW Sidebar Shortcuts Num %2d", KeyIndex);
			}

			constexpr operator char* () noexcept { return buffer; }
		};
		static to_string_t name;
		return name;
	}

	return "SW Sidebar Display";
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIName() const
{
	if (KeyIndex > 0)
	{
		class to_string_t
		{
		public:
			wchar_t buffer[20];

		public:
			constexpr to_string_t() noexcept
				: buffer { L"" }
			{
				swprintf_s(buffer, L"Quick Select SW %2d", KeyIndex);
			}

			constexpr operator wchar_t* () noexcept { return buffer; }
		};
		static to_string_t name;
		return StringTable::TryFetchString("TXT_SW_XX_FORWARD", name);
	}

	return GeneralUtils::LoadStringUnlessMissing("TXT_SW_XX_FORWARD", L"SW sidebar display");
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIDescription() const
{
	if (KeyIndex > 0)
	{
		class to_string_t
		{
		public:
			wchar_t buffer[34];

		public:
			constexpr to_string_t() noexcept
				: buffer { L"" }
			{
				swprintf_s(buffer, L"Select No.%02d SW in left sidebar", KeyIndex);
			}

			constexpr operator wchar_t* () noexcept { return buffer; }
		};
		static to_string_t name;
		return StringTable::TryFetchString("TXT_SW_XX_FORWARD_DESC", name);
	}

	return GeneralUtils::LoadStringUnlessMissing("TXT_SW_XX_FORWARD_DESC", L"Switch between visible/invisible modes for exclusive SW sidebar");
}

template<size_t KeyIndex>
inline void SWShortcutsCommandClass<KeyIndex>::Execute(WWKey eInput) const
{
	if (KeyIndex > 0)
	{
		TacticalButtonsClass::Instance.KeyboardCall = true;
		TacticalButtonsClass::Instance.SWSidebarTrigger(KeyIndex);
		return;
	}

	TacticalButtonsClass::Instance.SWSidebarSwitch();
}

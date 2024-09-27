#pragma once
#include "Commands.h"

#include <Ext/Sidebar/Body.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>

template<size_t Index>
class FireTacticalSWCommandClass : public CommandClass
{
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t Index>
inline const char* FireTacticalSWCommandClass<Index>::GetName() const
{
	class to_string_t
	{
	public:
		char buffer[17];

	public:
		constexpr to_string_t() noexcept
			: buffer { "FireTacticalSW" }
		{
			size_t idx = 14;
			buffer[idx++] = Index + '0';
			buffer[idx++] = '\0';
		}

		constexpr operator char* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return ret;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIName() const
{
	class to_string_t
	{
	public:
		wchar_t buffer[20];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Fire tactical SW " }
		{
			size_t idx = 17;
			buffer[idx++] = Index + '0';
			buffer[idx++] = '\0';
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX", ret);
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIDescription() const
{
	class to_string_t
	{
	public:
		wchar_t buffer[20];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Fire tactical SW " }
		{
			size_t idx = 17;
			buffer[idx++] = Index + '0';
			buffer[idx++] = '\0';
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX_DESC", ret);
}

template<size_t Index>
inline void FireTacticalSWCommandClass<Index>::Execute(WWKey eInput) const
{
	if (!SWSidebarClass::IsEnabled())
		return;

	const auto column = SWSidebarClass::Instance.Columns.front();

	if (!column)
		return;

	const auto& buttons = column->Buttons;

	if (buttons.size() > Index)
		buttons[Index]->LaunchSuper();
}

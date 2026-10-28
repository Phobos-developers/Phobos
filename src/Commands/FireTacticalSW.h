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
	_snprintf_s(Phobos::readBuffer, Phobos::readLength, "FireTacticalSW%d", Index + 1);
	return Phobos::readBuffer;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIName() const
{
	const wchar_t* csfString = StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX", L"Fire Super Weapon %d");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Index + 1);
	return Phobos::wideBuffer;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIDescription() const
{
	const wchar_t* csfString = StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX_DESC", L"Fires the Super Weapon at position %d in the Super Weapon sidebar.");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Index + 1);
	return Phobos::wideBuffer;
}

template<size_t Index>
inline void FireTacticalSWCommandClass<Index>::Execute(WWKey eInput) const
{
	if (!SWSidebarClass::IsEnabled())
		return;

	const auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return;

	const auto& buttons = columns.front()->Buttons;

	if (Index < buttons.size())
		buttons[Index]->LaunchSuper();
}

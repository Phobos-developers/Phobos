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
	_snprintf_s(Phobos::readBuffer, Phobos::readLength, "FireTacticalSW%d", Index);
	return Phobos::readBuffer;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIName() const
{
	_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, L"Fire Super Weapon %d", Index);
	return StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX", Phobos::wideBuffer);
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t Index>
inline const wchar_t* FireTacticalSWCommandClass<Index>::GetUIDescription() const
{
	_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, L"Fires Super Weapon %d.", Index);
	return StringTable::TryFetchString("TXT_FIRE_TACTICAL_SW_XX_DESC", Phobos::wideBuffer);
}

template<size_t Index>
inline void FireTacticalSWCommandClass<Index>::Execute(WWKey eInput) const
{
	if (!SWSidebarClass::IsEnabled())
		return;

	const auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return;

	const auto& buttons = columns.front()->Buttons;

	if (buttons.size() > Index)
		buttons[Index]->LaunchSuper();
}

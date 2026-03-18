#include "BuildLastOfTab.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/House/Body.h>
#include <HouseClass.h>
#include <EventClass.h>
#include <SidebarClass.h>

static constexpr const char* BuildLastTabNames[3] =
{
	"RebuildStructure",
	"RebuildDefense",
	"RebuildInfantry",
};

static constexpr const char* BuildLastTabDescKeys[3] =
{
	"RebuildStructure_Desc",
	"RebuildDefense_Desc",
	"RebuildInfantry_Desc",
};

static constexpr const wchar_t* BuildLastTabUINames[3] =
{
	L"Rebuild Structure",
	L"Rebuild Defense",
	L"Rebuild Infantry",
};

static constexpr const wchar_t* BuildLastTabUIDescs[3] =
{
	L"Re-queue the last produced Power/Resources building.",
	L"Re-queue the last produced Defense/Combat building.",
	L"Re-queue the last produced Infantry unit.",
};

template<int TabIndex>
const char* BuildLastOfTabCommandClass<TabIndex>::GetName() const
{
	return BuildLastTabNames[TabIndex];
}

template<int TabIndex>
const wchar_t* BuildLastOfTabCommandClass<TabIndex>::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing(BuildLastTabNames[TabIndex], BuildLastTabUINames[TabIndex]);
}

template<int TabIndex>
const wchar_t* BuildLastOfTabCommandClass<TabIndex>::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

template<int TabIndex>
const wchar_t* BuildLastOfTabCommandClass<TabIndex>::GetUIDescription() const
{
	static_assert(TabIndex >= 0 && TabIndex < 3, "TabIndex out of range");
	return GeneralUtils::LoadStringUnlessMissing(BuildLastTabDescKeys[TabIndex], BuildLastTabUIDescs[TabIndex]);
}

template<int TabIndex>
void BuildLastOfTabCommandClass<TabIndex>::Execute(WWKey eInput) const
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
		return;

	auto const pExt = HouseExt::ExtMap.Find(pPlayer);
	if (!pExt)
		return;

	auto const typeIndex = pExt->LastBuiltPerTab[TabIndex];
	if (typeIndex < 0)
		return;

	// Focus the sidebar to the corresponding tab.
	if (SidebarClass::Instance.IsSidebarActive)
	{
		SidebarClass::Instance.ActiveTabIndex = TabIndex;
		SidebarClass::Instance.SidebarNeedsRepaint();
	}

	auto const rtti = pExt->LastBuiltRTTIPerTab[TabIndex];
	auto const isNaval = pExt->LastBuiltIsNavalPerTab[TabIndex];

	EventClass::OutList.Add(EventClass(
		pPlayer->ArrayIndex,
		EventType::Produce,
		static_cast<int>(rtti),
		typeIndex,
		isNaval ? TRUE : FALSE
	));
}

// Explicit instantiations
template class BuildLastOfTabCommandClass<0>;
template class BuildLastOfTabCommandClass<1>;
template class BuildLastOfTabCommandClass<2>;

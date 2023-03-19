#include <Misc/NewSidebarClass.h>

#include <TechnoTypeClass.h>
#include <Unsorted.h>
#include <VoxClass.h>

bool NewSidebarClass::NewStripClass::IsOnSidebar(AbstractType type, int id)
{
	for (int i = 0; i < this->CameoCount; ++i)
	{
		auto cameo = this->Cameos[i];
		if (cameo.ItemType == type && cameo.ItemIndex == id)
			return true;
	}
	return false;
}

// 0x6A8420
bool NewSidebarClass::NewStripClass::RTTICheck(AbstractType type1, int id1, AbstractType type2, int id2)
{
	char rtti;
	auto pTechnoType1 = TechnoTypeClass::GetByTypeAndIndex(type1, id1);
	auto pTechnoType2 = TechnoTypeClass::GetByTypeAndIndex(type2, id2);

	if (type2 == AbstractType::None)
		return true;

	if (type1 == AbstractType::Special || type1 == AbstractType::Super || (rtti = 0, type1 == AbstractType::SuperWeaponType))
		rtti = 1;

	auto v10 = type2 == AbstractType::Special || type2 == AbstractType::Super || type2 == AbstractType::SuperWeaponType;
	auto heap_id = v10;

	if (rtti != 0)
	{
		if (v10)
		{
			...
		}
	}

	...
}

// 0x6A8710
int NewSidebarClass::NewStripClass::AddCameo(AbstractType type, int id)
{
	int v4 = 0;
	int result = ++this->CameoCount;
	if (result <= 0)
		return 0;

	for (auto cameo = this->Cameos; !this->rtticheck(type, id, cameo->ItemType, cameo->ItemIndex); ++cameo)
	{
		result = this->CameoCount;
		if (++v4 >= result)
			return result;
	}
	int v8 = this->CameoCount - 1;
	...
}

int NewSidebarClass::GetTab(AbstractType type, int id)
{
	switch (type)
	{
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		return TabInfantry;
	case AbstractType::Unit:
	case AbstractType::UnitType:
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		return TabVehicles;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		return ObjectTypeClass::IsBuildCat5(type, id) == 5 ? TabDefences : TabBuildings;
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		return TabSuperWeapons;
	default:
		return -1;
	}
}

//////////////////////////////////////////////////////

// ???
void NewSidebarClass::SidebarNeedsRepaint(int mode)
{
	this->SidebarNeedsRedraw = true;
	this->SidebarBackgroundNeedsRedraw = true;
	this->Tabs[this->ActiveTabIndex].AllowedToDraw = true;
	this->Tabs[this->ActiveTabIndex].NeedsRedraw = true;
	this->RedrawSidebar(mode);
	NewSidebarClass::Draw(1);
}

// 0x6A60A0
void NewSidebarClass::StripNeedsRepaint(int tab)
{
	if (tab == ActiveTabIndex)
	{
		this->SidebarNeedsRedraw = true;
		this->Tabs[tab].NeedsRedraw = true;
		this->RedrawSidebar(0);
	}
}

// 0x6A6300
bool NewSidebarClass::AddCameo(AbstractType type, int id)
{
	const constexpr int MaxCameoPerTab = 75;

	// some kind of debug - who named it like that????
	if (Unsorted::ArmageddonMode)
		return false;

	// not sure what it does exactly
	bool someKindOfToggle = false;
	int v4 = 0;
	int* v5 = &Tabs[0].CameoCount;
	while (*v5 <= 0)
	{
		++v4;
		v5 += 0x3E5;
		if (v4 >= 4)
			someKindOfToggle = true;
	}

	int tab = GetTab(type, id);
	if (this->Tabs[tab].CameoCount > MaxCameoPerTab)
		return false;

	// don't add if already on sidebar
	if (this->Tabs[tab].IsOnSidebar(type, id))
		return false;

	if (!Unsorted::IKnowWhatImDoing && type != AbstractType::Special)
		VoxClass::Play("EVA_NewConstructionOptions", -1, -1);

	this->Tabs[tab].AddCameo(type, id);

	...
}

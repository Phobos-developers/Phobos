#include <Misc/NewSidebarClass.h>

#include <SuperWeaponTypeClass.h>
#include <TechnoTypeClass.h>
#include <HouseClass.h>
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
// cameo sorting function?
bool NewSidebarClass::NewStripClass::CameoComparatorStuff(AbstractType type1, int id1, AbstractType type2, int id2)
{
	if (type2 == AbstractType::None)
		return true;

	auto pTechnoType1 = TechnoTypeClass::GetByTypeAndIndex(type1, id1);
	auto pTechnoType2 = TechnoTypeClass::GetByTypeAndIndex(type2, id2);

	bool isSuper1 = type1 == AbstractType::Special || type1 == AbstractType::Super || type1 == AbstractType::SuperWeaponType;
	bool isSuper2 = type2 == AbstractType::Special || type2 == AbstractType::Super || type2 == AbstractType::SuperWeaponType;
	// sort by recharge time, then alphabetically
	if (isSuper1 && isSuper2)
	{
		auto pSWType1 = SuperWeaponTypeClass::Array->GetItem(id1);
		auto pSWType2 = SuperWeaponTypeClass::Array->GetItem(id2);
		if (pSWType1->RechargeTime < pSWType2->RechargeTime)
			return true;
		else if (pSWType1->RechargeTime > pSWType2->RechargeTime)
			return false;
		return wcscmp(pSWType1->UIName, pSWType2->UIName) <= 0;
	}
	// sort by side?
	else if (!isSuper1 && !isSuper2)
	{
		int sideIdx = HouseClass::CurrentPlayer->Type->SideIndex;
		if (sideIdx == pTechnoType1->AIBasePlanningSide)
			if (sideIdx != pTechnoType2->AIBasePlanningSide)
				return true;
		else if (sideIdx == pTechnoType2->AIBasePlanningSide)
			return false;
	}

	bool isVehicle1 = type1 == AbstractType::UnitType || type1 == AbstractType::AircraftType;
	bool isAircraft1 = isVehicle1 && pTechnoType1->ConsideredAircraft;
	bool isNaval1 = isVehicle1 && pTechnoType1->Naval;
	bool isGround1 = !isAircraft1 && !isNaval1;
	bool isVehicle2 = type2 == AbstractType::UnitType || type2 == AbstractType::AircraftType;
	bool isAircraft2 = isVehicle2 && pTechnoType2->ConsideredAircraft;
	bool isNaval2 = isVehicle2 && pTechnoType2->Naval;
	bool isGround2 = !isAircraft2 && !isNaval2;

	if (isSuper1)
	{
		if (isAircraft2 || isNaval2 || isGround2)
			return true;
		// goto 74
	}
	if (isGround1)
	{
		if (isSuper2)
			return false;
		if (isAircraft2)
			return true;
	}
	else
	{
		if (!isAircraft1)
		{
			if (isNaval1)
			{
				if (isSuper2)
					return false;
				if (isGround2 || isAircraft2)
					return false;
			}
			// goto 74
		}
		if (isSuper2)
			return false;
		if (isGround2)
			return false;
	}
	if (isNaval2)
		return false;
	// 74
	if (pTechnoType1->TechLevel < pTechnoType2->TechLevel)
		return true;
	if (pTechnoType1->TechLevel > pTechnoType2->TechLevel)
		return false;

	return wcscmp(pTechnoType1->UIName, pTechnoType2->UIName) <= 0;
}

// 0x6A8710
int NewSidebarClass::NewStripClass::AddCameo(AbstractType type, int id)
{
	int v4 = 0;
	int result = ++this->CameoCount;
	if (result <= 0)
		return 0;

	for (auto cameo = this->Cameos; !this->CameoComparatorStuff(type, id, cameo->ItemType, cameo->ItemIndex); ++cameo)
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

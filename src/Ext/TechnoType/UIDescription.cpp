#include <Helpers/Macro.h>
#include <TechnoTypeClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <SidebarClass.h>
#include <StringTable.h>
#include <wchar.h>
#include "Body.h"

/*DEFINE_HOOK(6A9779, TechnoType_UIDescription, B)
{
	

	GET(TechnoTypeClass*, pThis, EBX);	
	LEA_STACK(const wchar_t**, tooltipStr, 0x474);

	const wchar_t* name = pThis->UIName;
	const wchar_t* description = TechnoTypeExt::ExtMap.Find(pThis)->UIDescription;
	
	if (wcslen(name) == 0)
	{
		*tooltipStr = description;
	}
	else if (wcslen(description) == 0)
	{
		*tooltipStr = name;
	}
	else
	{
		wchar_t* newString = L"";
		wcscpy(newString, name);
		wcscat(newString, L"\n");
		wcscat(newString, description);
		*tooltipStr = newString;
	}

	return 0x6A9784;
}*/

// taken from Ares bugfixes partially
DEFINE_HOOK(6A9343, Cameo_ExtendedToolTip, 9)
{
	bool HideObjectName = *reinterpret_cast<byte*>(0x884B8C);

	GET(TechnoTypeClass*, Object, ESI);

	const wchar_t* Format;
	const wchar_t* UIName = Object->UIName;
	int Cost = Object->GetActualCost(HouseClass::Player);

	if (true || Phobos::UI::ExtendedToolTips)
	{
		const wchar_t* UIDescription = L"Lorem ipsum dolor sit amet"; //TechnoTypeExt::ExtMap.Find(Object)->UIDescription;

		int Power = 0;
		if (Object->WhatAmI() == AbstractType::BuildingType) {
			BuildingTypeClass* ObjectAsBuildingType = static_cast<BuildingTypeClass*>(Object);
			Power = ObjectAsBuildingType->PowerBonus - ObjectAsBuildingType->PowerDrain;
		}

		int Time = 0;

		// String building

		SidebarClass::TooltipBuffer[0] = NULL;

		if (!HideObjectName && UIName != NULL && wcslen(UIName) != 0)
		{
			wcscat(SidebarClass::TooltipBuffer, UIName);
			wcscat(SidebarClass::TooltipBuffer, L"\n");
		}

		Phobos::wideBuffer[0] = NULL;
		_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
			L"%s%d", Phobos::UI::CostLabel, Cost);
		wcscat(SidebarClass::TooltipBuffer, Phobos::wideBuffer);
		
		if (Power)
		{
			Phobos::wideBuffer[0] = NULL;
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L" %s%d", Phobos::UI::PowerLabel, Power);
			wcscat(SidebarClass::TooltipBuffer, Phobos::wideBuffer);
		}
		
		if (Time)
		{
			Phobos::wideBuffer[0] = NULL;
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L" %s%d", Phobos::UI::TimeLabel, Time);
			wcscat(SidebarClass::TooltipBuffer, Phobos::wideBuffer);
		}

		if (UIDescription != NULL && wcslen(UIDescription) != 0)
		{
			wcscat(SidebarClass::TooltipBuffer, L"\n");
			wcscat(SidebarClass::TooltipBuffer, UIDescription);
		}
	}
	else
	{
		if (HideObjectName)
		{
			Format = StringTable::LoadString("TXT_MONEY_FORMAT_1");
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, Cost);
		}
		else
		{
			Format = StringTable::LoadString("TXT_MONEY_FORMAT_2");
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, UIName, Cost);
		}
	}
	SidebarClass::TooltipBuffer[SidebarClass::TooltipLength - 1] = 0;

	return 0x6A93D9;
}



/*DEFINE_HOOK(6A977E, TechnoType_AppendUIDescription, 6)
{
	GET(TechnoTypeClass*, pThis, EBX);
	GET(const wchar_t*, pName, EAX);

	const wchar_t* description = L"Hey now";

	//if (wcslen(pName) == 0)
	//{
		R->EAX(description);
	//}
	/*else if (wcslen(description) != 0)
	{
		wchar_t* newString = L"";
		wcscpy(newString, pName);
		wcscat(newString, L"\n");
		wcscat(newString, description);
		R->EAX(newString);
	}

	R->ECX(pThis);
	return 0x6A9784;
}*/

#include <Helpers/Macro.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <SidebarClass.h>
#include <StringTable.h>
#include <wchar.h>
#include "../Ext/TechnoType/Body.h"

#define TOOLTIP_BUFFER_LENGTH 1024
wchar_t ToolTip_ExtendedBuffer[TOOLTIP_BUFFER_LENGTH];
bool ToolTip_DrawExBuffer = false;

// taken from Ares bugfixes partially
DEFINE_HOOK(6A9343, TechnoType_ExtendedToolTip, 9)
{
	// TODO implement buildtime calculation
	GET(TechnoTypeClass*, pThis, ESI);
	bool hideName = *reinterpret_cast<byte*>(0x884B8C);
	
	const wchar_t* uiName = pThis->UIName;
	int cost = pThis->GetActualCost(HouseClass::Player);

	if (Phobos::UI::ExtendedToolTips) {
		ToolTip_ExtendedBuffer[0] = NULL;

		// append UIName label
		if (!hideName && uiName && wcslen(uiName) != 0) {
			wcscat_s(ToolTip_ExtendedBuffer, uiName);
			wcscat_s(ToolTip_ExtendedBuffer, L"\n");
		}

		// append Cost label
		{
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L"%ls%d", Phobos::UI::CostLabel, cost);
			wcscat_s(ToolTip_ExtendedBuffer, Phobos::wideBuffer);
		}

		// append PowerBonus label
		if (pThis->WhatAmI() == AbstractType::BuildingType) {
			BuildingTypeClass* ObjectAsBuildingType = static_cast<BuildingTypeClass*>(pThis);
			int Power = ObjectAsBuildingType->PowerBonus - ObjectAsBuildingType->PowerDrain;

			if (Power) {
				_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
					L" %ls%+d", Phobos::UI::PowerLabel, Power);
				wcscat_s(ToolTip_ExtendedBuffer, Phobos::wideBuffer);
			}
		}

		// append Time label
		/*
		int Time = 0;
		if (Time) {
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L" %ls%d", Phobos::UI::TimeLabel, Time);
			wcscat_s(ToolTip_ExtendedBuffer, Phobos::wideBuffer);
		}
		*/

		// append UIDescription label
		const wchar_t* uiDesc = TechnoTypeExt::ExtMap.Find(pThis)->UIDescription;
		if (uiDesc && wcslen(uiDesc) != 0) {
			wcscat_s(ToolTip_ExtendedBuffer, L"\n");
			wcscat_s(ToolTip_ExtendedBuffer, uiDesc);
		}

		ToolTip_ExtendedBuffer[TOOLTIP_BUFFER_LENGTH - 1] = 0;
		ToolTip_DrawExBuffer = true;
		R->EAX(ToolTip_ExtendedBuffer);

		return 0x6A93DE;

	} else {
		// "Vanilla" Ares code
		const wchar_t* formatStr;
		if (hideName) {
			formatStr = StringTable::LoadString("TXT_MONEY_FORMAT_1");
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1,
				formatStr, cost);
		}
		else {
			formatStr = StringTable::LoadString("TXT_MONEY_FORMAT_2");
			_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1,
				formatStr, uiName, cost);
		}

		SidebarClass::TooltipBuffer[SidebarClass::TooltipLength - 1] = 0;

		return 0x6A93B2;
	}
}

DEFINE_HOOK(478EE1, ToolTip_ExtendedBuffer_Draw, 6)
{
	if (ToolTip_DrawExBuffer) {
		R->EDI(ToolTip_ExtendedBuffer);
	}
	return 0;
}

DEFINE_HOOK(6AC210, ToolTip_ExtendedBuffer_Shutdown, 6)
{
	ToolTip_DrawExBuffer = false;
	return 0;
}

DEFINE_HOOK(724B85, ToolTip_Fix_QWER_bug, 0)
{
	R->ESI(R->ECX());	//mov esi, ecx
	return 0x724B8B;
}

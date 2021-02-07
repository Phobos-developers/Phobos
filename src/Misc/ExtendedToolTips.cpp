#include "ExtendedToolTips.h"

bool ExtToolTip::isCameo = false;
bool ExtToolTip::slaveDraw = false;
bool ExtToolTip::_UseExtBuffer = false;
wchar_t ExtToolTip::_ExtBuffer[TOOLTIP_BUFFER_LENGTH] = L"";
bool ExtToolTip::addSpace = false;
bool ExtToolTip::addNewLine = false;

void CreateHelpText(AbstractType itemType, int itemIndex)
{
	AbstractTypeClass* pAbstract = nullptr;
	SuperWeaponTypeClass* pSW = nullptr;
	SWTypeExt::ExtData* pSWExt = nullptr;
	TechnoTypeClass* pTechno = nullptr;
	BuildingTypeClass* pBuilding = nullptr;

	if (itemType == AbstractType::Special) {
		pSW = SuperWeaponTypeClass::Array->GetItem(itemIndex);
		pAbstract = pSW;
		pSWExt = SWTypeExt::ExtMap.Find(pSW);
	}
	else {
		pTechno = ObjectTypeClass::GetTechnoType(itemType, itemIndex);
		if (pTechno->WhatAmI() == AbstractType::BuildingType) {
			pBuilding = static_cast<BuildingTypeClass*>(pTechno);
		}
		pAbstract = pTechno;
	}

	ExtToolTip::Clear_Separator();

	// append UIName label
	const wchar_t* uiName = pAbstract->UIName;
	if (!CCToolTip::HideName && uiName && uiName[0] != 0) {
		ExtToolTip::Append_NewLineLater();
		ExtToolTip::Append(uiName);
	}

	// append Cost label
	const int cost = pSWExt ? -pSWExt->Money_Amount : pTechno->GetActualCost(HouseClass::Player);
	if (pTechno || cost > 0) {
		_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
			L"%ls%d", Phobos::UI::CostLabel, cost);

		ExtToolTip::Apply_Separator();
		ExtToolTip::Append(Phobos::wideBuffer);
		ExtToolTip::Append_SpaceLater();
	}

	// append PowerBonus label
	if (pBuilding) {
		const int Power = pBuilding->PowerBonus - pBuilding->PowerDrain;

		if (Power) {
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L" %ls%+d", Phobos::UI::PowerLabel, Power);

			ExtToolTip::Apply_Separator();
			ExtToolTip::Append_SpaceLater();
			ExtToolTip::Append(Phobos::wideBuffer);
		}
	}

	// append Time label
	const long rechargeTime = pSW ? pSW->RechargeTime : 0;
	if (rechargeTime) {
		const int sec = (rechargeTime / 15) % 60;
		const int min = (rechargeTime / 15) / 60;

		_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
			L"%ls%02d:%02d", Phobos::UI::TimeLabel, min, sec);

		ExtToolTip::Apply_Separator();
		ExtToolTip::Append_NewLineLater();

		ExtToolTip::Append(Phobos::wideBuffer);
	}

	// append UIDescription 
	const wchar_t* uiDesc = pSWExt ? pSWExt->UIDescription : TechnoTypeExt::ExtMap.Find(pTechno)->UIDescription;
	if (uiDesc && uiDesc[0] != 0) {
		if (uiDesc && wcslen(uiDesc) != 0) {
			ExtToolTip::Apply_SeparatorAsNewLine();
			ExtToolTip::Append(uiDesc);
		}
	}
}

// =============================
// hooks

DEFINE_HOOK(6A9319, ExtendedToolTip_HelpText, 5) {
	if (!Phobos::UI::ExtendedToolTips) {
		return 0;
	}

	GET(const int*, ptr, EAX);
	auto itemIndex = ptr[22];
	auto itemType = (AbstractType)ptr[23];

	ExtToolTip::ClearBuffer();
	ExtToolTip::isCameo = true;

	CreateHelpText(itemType, itemIndex);

	ExtToolTip::UseExtBuffer();
	R->EAX(Phobos::wideBuffer); // Here you need to pass any non-empty string
	return 0x6A93DE;
}

DEFINE_HOOK(478E10, CCToolTip__Draw1, 0)
{
	GET(CCToolTip*, pThis, ECX);
	GET_STACK(bool, drawOnSidebar, 4);

	if (!drawOnSidebar || ExtToolTip::isCameo) { // !onSidebar or (onSidebar && ExtToolTip::onCameo)
		ExtToolTip::isCameo = false;
		ExtToolTip::slaveDraw = false;
		ExtToolTip::ClearBuffer();

		pThis->Adjust();	//this function re-create CCToolTip
	}

	if (pThis->manager.ActiveTooltip) {
		if (!drawOnSidebar) {
			ExtToolTip::slaveDraw = ExtToolTip::isCameo;
		}
		pThis->drawOnSidebar = drawOnSidebar;
		pThis->Draw2();
	}
	return 0x478E25;
}

DEFINE_HOOK(478E4A, CCToolTip__Draw2_SetSurface, 6)
{
	if (ExtToolTip::slaveDraw) {
		R->ESI(DSurface::Composite);
		return 0x478ED3;
	}
	return 0;
}

DEFINE_HOOK(478F52, CCToolTip__Draw2_SetX, 8)
{
	if (ExtToolTip::slaveDraw) {
		R->EAX(R->EAX() + DSurface::Sidebar->GetWidth());
	}
	return 0;
}

DEFINE_HOOK(478EE1, CCToolTip__Draw2_SetBuffer, 6)
{
	ExtToolTip::SetBuffer(R);
	return 0;
}

DEFINE_HOOK(478EF8, CCToolTip__Draw2_SetWidth, 5)
{
	if (ExtToolTip::isCameo) {
		auto ViewBounds = reinterpret_cast<RectangleStruct*>(0x886FB0);
		R->EAX(ViewBounds->Width);
	}
	return 0;
}

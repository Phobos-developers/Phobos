#include "ExtendedToolTips.h"

#include <AircraftClass.h>
#include <InfantryClass.h>

void ExtToolTip::CreateHelpText(AbstractType itemType, int itemIndex)
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
		ExtToolTip::Append_SpaceLater();

		ExtToolTip::Append(Phobos::wideBuffer);
	}

	// append PowerBonus label
	if (pBuilding) {
		const int Power = pBuilding->PowerBonus - pBuilding->PowerDrain;

		if (Power) {
			_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
				L"%ls%+d", Phobos::UI::PowerLabel, Power);

			ExtToolTip::Apply_Separator();
			ExtToolTip::Append_SpaceLater();

			ExtToolTip::Append(Phobos::wideBuffer);
		}
	}

	// append Time label
	long rechargeTime = 0;
	if (pSW)
	{
		rechargeTime = pSW->RechargeTime;
	}
	else if(pTechno)
	{
		// TechnoClass::Time_To_Build() accept a TechnoClass and but only uses its Type,
		// RTTIType and House. That's why we can just create a fake buffer and send it
		// to the function. For the Type, we need to hack their vtables, so the following
		// checks are necessary.
		// Author : secsome - 2021/12/08
		static char pTrick[0x6C8]; // Just big enough to hold all types
		switch (pTechno->WhatAmI())
		{
		case AbstractType::BuildingType:
			*reinterpret_cast<int*>(pTrick) = 0x7E3EBC; // BuildingClass::`vtable`
			reinterpret_cast<BuildingClass*>(pTrick)->Type = (BuildingTypeClass*)pTechno;
			break;
		case AbstractType::AircraftType:
			*reinterpret_cast<int*>(pTrick) = 0x7E22A4; // AircraftClass::`vtable`
			reinterpret_cast<AircraftClass*>(pTrick)->Type = (AircraftTypeClass*)pTechno;
			break;
		case AbstractType::InfantryType:
			*reinterpret_cast<int*>(pTrick) = 0x7EB058; // InfantryClass::`vtable`
			reinterpret_cast<InfantryClass*>(pTrick)->Type = (InfantryTypeClass*)pTechno;
			break;
		case AbstractType::UnitType:
			*reinterpret_cast<int*>(pTrick) = 0x7F5C70; // UnitClass::`vtable`
			reinterpret_cast<UnitClass*>(pTrick)->Type = (UnitTypeClass*)pTechno;
			break;
		}

		// TechnoTypeClass only has 4 final classes : 
		// BuildingTypeClass, AircraftTypeClass, InfantryTypeClass and UnitTypeClass
		// It has to be these four classes, otherwise pTechno will just be nullptr
		reinterpret_cast<TechnoClass*>(pTrick)->Owner = HouseClass::Player;
		rechargeTime = reinterpret_cast<TechnoClass*>(pTrick)->TimeToBuild();
	}

	if (rechargeTime > 0) {
		const int sec = (rechargeTime / 15) % 60;
		const int min = (rechargeTime / 15) / 60;

		_snwprintf_s(Phobos::wideBuffer, Phobos::readLength, Phobos::readLength - 1,
			L"%ls%02d:%02d", Phobos::UI::TimeLabel, min, sec);

		ExtToolTip::Apply_Separator();
		ExtToolTip::Append_NewLineLater();

		ExtToolTip::Append(Phobos::wideBuffer);
	}

	// append UIDescription
	if (Phobos::Config::ToolTipDescriptions) {
		auto uiDesc = pSWExt ? pSWExt->UIDescription : TechnoTypeExt::ExtMap.Find(pTechno)->UIDescription;
		if (!uiDesc.Get().empty()) {
			ExtToolTip::Apply_SeparatorAsNewLine();
			ExtToolTip::Append(uiDesc.Get().Text);
		}
	}
}

// =============================
// hooks

DEFINE_HOOK(0x6A9316, ExtendedToolTip_HelpText, 0x6)
{
	ExtToolTip::isCameo = true;

	if (!Phobos::UI::ExtendedToolTips) {
		return 0;
	}

	GET(const int*, ptr, EAX);
	auto const itemIndex = ptr[22];
	auto const itemType = (AbstractType)ptr[23];

	ExtToolTip::ClearBuffer();

	ExtToolTip::CreateHelpText(itemType, itemIndex);

	ExtToolTip::UseExtBuffer();
	R->EAX(ExtToolTip::pseudoBuff); // Here you need to pass any non-empty string
	return 0x6A93DE;
}

DEFINE_HOOK(0x478E10, CCToolTip__Draw1, 0x0)
{
	GET(CCToolTip*, pThis, ECX);
	GET_STACK(bool, bFullRedraw, 0x4);

	if (!bFullRedraw || ExtToolTip::isCameo) { // !onSidebar or (onSidebar && ExtToolTip::isCameo)
		ExtToolTip::isCameo = false;
		ExtToolTip::slaveDraw = false;
		ExtToolTip::ClearBuffer();

		pThis->ToolTipManager::Process();	//this function re-create CCToolTip
	}

	if (pThis->CurrentToolTip) {
		if (!bFullRedraw) {
			ExtToolTip::slaveDraw = ExtToolTip::isCameo;
		}
		pThis->FullRedraw = bFullRedraw;
		pThis->Draw2(pThis->CurrentToolTipData);
	}
	return 0x478E25;
}

DEFINE_HOOK(0x478E4A, CCToolTip__Draw2_SetSurface, 0x6)
{
	if (ExtToolTip::slaveDraw) {
		R->ESI(DSurface::Composite());
		return 0x478ED3;
	}
	return 0;
}

DEFINE_HOOK(0x478EE1, CCToolTip__Draw2_SetBuffer, 0x6)
{
	ExtToolTip::SetBuffer(R);
	return 0;
}

DEFINE_HOOK(0x478EF8, CCToolTip__Draw2_SetMaxWidth, 0x5)
{
	if (ExtToolTip::isCameo) {
		
		if (Phobos::UI::MaxToolTipWidth > 0) {
			R->EAX(Phobos::UI::MaxToolTipWidth);
		}
		else {
			auto const ViewBounds = reinterpret_cast<RectangleStruct*>(0x886FB0);
			R->EAX(ViewBounds->Width);
		}
	}
	return 0;
}

DEFINE_HOOK(0x478F52, CCToolTip__Draw2_SetX, 0x8)
{
	if (ExtToolTip::slaveDraw) {
		R->EAX(R->EAX() + DSurface::Sidebar->GetWidth());
	}
	return 0;
}

DEFINE_HOOK(0x478F77, CCToolTip__Draw2_SetY, 0x6)
{
	if (ExtToolTip::isCameo) {
		auto const ViewBounds = reinterpret_cast<RectangleStruct*>(0x886FB0);
		LEA_STACK(RectangleStruct*, Rect, STACK_OFFS(0x3C, 0x20));

		int const maxHeight = ViewBounds->Height - 32;

		if (Rect->Height > maxHeight)
			Rect->Y += maxHeight - Rect->Height;

		if (Rect->Y < 0)
			Rect->Y = 0;
	}
	return 0;
}

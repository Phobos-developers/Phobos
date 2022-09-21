#include <Helpers/Macro.h>

#include "PhobosToolTip.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>

#include <GameOptionsClass.h>
#include <CCToolTip.h>
#include <BitFont.h>
#include <BitText.h>

#include <Ext/Side/Body.h>
#include <Ext/Surface/Body.h>

#include <sstream>
#include <iomanip>

PhobosToolTip PhobosToolTip::Instance;

inline bool PhobosToolTip::IsEnabled() const
{
	return Phobos::UI::ExtendedToolTips;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(TechnoTypeExt::ExtData* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(SWTypeExt::ExtData* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

inline int PhobosToolTip::GetBuildTime(TechnoTypeClass* pType) const
{
	static char pTrick[0x6C8]; // Just big enough to hold all types
	switch (pType->WhatAmI())
	{
	case AbstractType::BuildingType:
		*reinterpret_cast<int*>(pTrick) = 0x7E3EBC; // BuildingClass::`vtable`
		reinterpret_cast<BuildingClass*>(pTrick)->Type = (BuildingTypeClass*)pType;
		break;
	case AbstractType::AircraftType:
		*reinterpret_cast<int*>(pTrick) = 0x7E22A4; // AircraftClass::`vtable`
		reinterpret_cast<AircraftClass*>(pTrick)->Type = (AircraftTypeClass*)pType;
		break;
	case AbstractType::InfantryType:
		*reinterpret_cast<int*>(pTrick) = 0x7EB058; // InfantryClass::`vtable`
		reinterpret_cast<InfantryClass*>(pTrick)->Type = (InfantryTypeClass*)pType;
		break;
	case AbstractType::UnitType:
		*reinterpret_cast<int*>(pTrick) = 0x7F5C70; // UnitClass::`vtable`
		reinterpret_cast<UnitClass*>(pTrick)->Type = (UnitTypeClass*)pType;
		break;
	}

	// TechnoTypeClass only has 4 final classes :
	// BuildingTypeClass, AircraftTypeClass, InfantryTypeClass and UnitTypeClass
	// It has to be these four classes, otherwise pType will just be nullptr
	reinterpret_cast<TechnoClass*>(pTrick)->Owner = HouseClass::CurrentPlayer;
	int nTimeToBuild = reinterpret_cast<TechnoClass*>(pTrick)->TimeToBuild();
	// 54 frames at least
	return nTimeToBuild < 54 ? 54 : nTimeToBuild;
}

inline int PhobosToolTip::GetPower(TechnoTypeClass* pType) const
{
	if (auto const pBldType = abstract_cast<BuildingTypeClass*>(pType))
		return pBldType->PowerBonus - pBldType->PowerDrain;

	return 0;
}

inline const wchar_t* PhobosToolTip::GetBuffer() const
{
	return this->TextBuffer.c_str();
}

void PhobosToolTip::HelpText(BuildType& cameo)
{
	if (cameo.ItemType == AbstractType::Special)
		this->HelpText(SuperWeaponTypeClass::Array->GetItem(cameo.ItemIndex));
	else
		this->HelpText(ObjectTypeClass::GetTechnoType(cameo.ItemType, cameo.ItemIndex));
}

void PhobosToolTip::HelpText(TechnoTypeClass* pType)
{
	if (!pType)
		return;

	auto const pData = TechnoTypeExt::ExtMap.Find(pType);

	int nBuildTime = this->GetBuildTime(pType);
	int nSec = nBuildTime / 15 % 60;
	int nMin = nBuildTime / 15 / 60 /* % 60*/;
	// int nHour = pType->RechargeTime / 15 / 60 / 60;

	int cost = pType->GetActualCost(HouseClass::CurrentPlayer);

	std::wostringstream oss;
	oss << pType->UIName << L"\n"
		<< (cost < 0 ? L"+" : L"")
		<< Phobos::UI::CostLabel << std::abs(cost) << L" "
		<< Phobos::UI::TimeLabel
		// << std::setw(2) << std::setfill(L'0') << nHour << L":"
		<< std::setw(2) << std::setfill(L'0') << nMin << L":"
		<< std::setw(2) << std::setfill(L'0') << nSec;

	if (auto const nPower = this->GetPower(pType))
	{
		oss << L" " << Phobos::UI::PowerLabel;
		if (nPower > 0)
			oss << L"+";
		oss << std::setw(1) << nPower;
	}

	if (auto pDesc = this->GetUIDescription(pData))
		oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
}

void PhobosToolTip::HelpText(SuperWeaponTypeClass* pType)
{
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	std::wostringstream oss;
	oss << pType->UIName;
	bool showCost = false;

	if (int nCost = std::abs(pData->Money_Amount))
	{
		oss << L"\n";

		if (pData->Money_Amount > 0)
			oss << '+';

		oss << Phobos::UI::CostLabel << nCost;
		showCost = true;
	}

	if (pType->RechargeTime > 0)
	{
		if (!showCost)
			oss << L"\n";

		int nSec = pType->RechargeTime / 15 % 60;
		int nMin = pType->RechargeTime / 15 / 60 /* % 60*/;
		// int nHour = pType->RechargeTime / 15 / 60 / 60;

		oss << (showCost ? L" " : L"") << Phobos::UI::TimeLabel
			// << std::setw(2) << std::setfill(L'0') << nHour << L":"
			<< std::setw(2) << std::setfill(L'0') << nMin << L":"
			<< std::setw(2) << std::setfill(L'0') << nSec;
	}

	if (auto pDesc = this->GetUIDescription(pData))
		oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
}

// Hooks

DEFINE_HOOK(0x6A9316, SidebarClass_StripClass_HelpText, 0x6)
{
	PhobosToolTip::Instance.IsCameo = true;

	if (!PhobosToolTip::Instance.IsEnabled())
		return 0;

	GET(StripClass*, pThis, EAX);
	PhobosToolTip::Instance.HelpText(pThis->Cameos[0]); // pStrip->Cameos[nID] in fact
	R->EAX(L"X");
	return 0x6A93DE;
}

// TODO: reimplement CCToolTip::Draw2 completely

DEFINE_HOOK(0x478EE1, CCToolTip_Draw2_SetBuffer, 0x6)
{
	if (PhobosToolTip::Instance.IsEnabled() && PhobosToolTip::Instance.IsCameo)
		R->EDI(PhobosToolTip::Instance.GetBuffer());
	return 0;
}

DEFINE_HOOK(0x478E10, CCToolTip_Draw1, 0x0)
{
	GET(CCToolTip*, pThis, ECX);
	GET_STACK(bool, bFullRedraw, 0x4);

	// !onSidebar or (onSidebar && ExtToolTip::IsCameo)
	if (!bFullRedraw || PhobosToolTip::Instance.IsCameo)
	{
		PhobosToolTip::Instance.IsCameo = false;
		PhobosToolTip::Instance.SlaveDraw = false;

		pThis->ToolTipManager::Process();	//this function re-create CCToolTip
	}

	if (pThis->CurrentToolTip)
	{
		if (!bFullRedraw)
			PhobosToolTip::Instance.SlaveDraw = PhobosToolTip::Instance.IsCameo;

		pThis->FullRedraw = bFullRedraw;
		pThis->DrawText(pThis->CurrentToolTipData);
	}
	return 0x478E25;
}

DEFINE_HOOK(0x478E4A, CCToolTip_Draw2_SetSurface, 0x6)
{
	if (PhobosToolTip::Instance.SlaveDraw)
	{
		R->ESI(DSurface::Composite());
		return 0x478ED3;
	}
	return 0;
}

DEFINE_HOOK(0x478EF8, CCToolTip_Draw2_SetMaxWidth, 0x5)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		if (Phobos::UI::MaxToolTipWidth > 0)
			R->EAX(Phobos::UI::MaxToolTipWidth);
		else
			R->EAX(DSurface::ViewBounds->Width);

	}
	return 0;
}

DEFINE_HOOK(0x478F52, CCToolTip_Draw2_SetX, 0x8)
{
	if (PhobosToolTip::Instance.SlaveDraw)
		R->EAX(R->EAX() + DSurface::Sidebar->GetWidth());

	return 0;
}

DEFINE_HOOK(0x478F77, CCToolTip_Draw2_SetY, 0x6)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		LEA_STACK(RectangleStruct*, Rect, STACK_OFFS(0x3C, 0x20));

		int const maxHeight = DSurface::ViewBounds->Height - 32;

		if (Rect->Height > maxHeight)
			Rect->Y += maxHeight - Rect->Height;

		if (Rect->Y < 0)
			Rect->Y = 0;
	}
	return 0;
}

// If tooltip rectangle width is constrained, make sure
// there is a padding zone so text isn't drawn into border
DEFINE_HOOK(0x479029, CCToolTip_Draw2_SetPadding, 0x5)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		if (Phobos::UI::MaxToolTipWidth > 0)
			R->EDX(R->EDX() - 5);
	}

	return 0;
}

void __declspec(naked) _CCToolTip_Draw2_FillRect_RET()
{
	ADD_ESP(8); // We need to handle origin two push here...
	JMP(0x478FE1);
}
DEFINE_HOOK(0x478FDC, CCToolTip_Draw2_FillRect, 0x5)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		GET(SurfaceExt*, pThis, ESI);
		LEA_STACK(RectangleStruct*, pRect, STACK_OFFS(0x44, 0x10));

		// Should we make some SideExt items as static to improve the effeciency?
		// Though it might not be a big improvement... - secsome
		const int nPlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
		if (auto const pSide = SideClass::Array->GetItemOrDefault(nPlayerSideIndex))
		{
			if (auto const pData = SideExt::ExtMap.Find(pSide))
			{
				// Could this flag be lazy?
				SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true;

				pThis->FillRectTrans(pRect,
					pData->ToolTip_Background_Color.GetEx(&RulesExt::Global()->ToolTip_Background_Color),
					pData->ToolTip_Background_Opacity.Get(RulesExt::Global()->ToolTip_Background_Opacity)
				);

				if (Phobos::Config::ToolTipBlur)
					pThis->BlurRect(*pRect, pData->ToolTip_Background_BlurSize.Get(RulesExt::Global()->ToolTip_Background_BlurSize));

				return (int)_CCToolTip_Draw2_FillRect_RET;
			}
		}
	}

	return 0;
}

// TODO in the future
//
//DEFINE_HOOK(0x478E30, CCToolTip_Draw2, 0x7)
//{
//	GET(CCToolTip*, pThis, ECX);
//	GET_STACK(ToolTipManagerData*, pManagerData, 0x4);
//
//	DSurface* pSurface = nullptr;
//
//	RectangleStruct bounds = pManagerData->Dimension;
//
//	if (GameOptionsClass::Instance->SidebarSide == 1)
//	{
//		int nR = DSurface::ViewBounds->X + DSurface::ViewBounds->Width;
//		if (bounds.X + pManagerData->Dimension.Width <= nR)
//			pSurface = DSurface::Composite;
//		else
//		{
//			if (!pThis->FullRedraw || bounds.X < nR)
//				return 0x479048;
//			pSurface = DSurface::Sidebar;
//			bounds.X -= nR;
//			*reinterpret_cast<bool*>(0xB0B518) = true;
//		}
//	}
//	else
//	{
//		int nR = DSurface::SidebarBounds->X + DSurface::SidebarBounds->Width;
//		if (bounds.X < nR)
//		{
//			if (!pThis->FullRedraw || bounds.X + pManagerData->Dimension.Width >= nR)
//				return 0x479048;
//			pSurface = DSurface::Sidebar;
//			*reinterpret_cast<bool*>(0xB0B518) = true;
//		}
//		else
//		{
//			pSurface = DSurface::Composite;
//			bounds.X -= nR;
//		}
//	}
//
//	if (pSurface)
//	{
//		BitFont::Instance->GetTextDimension(
//			PhobosToolTip::Instance.GetBuffer(), bounds.Width, bounds.Height,
//			Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : DSurface::WindowBounds->Width);
//
//		if (pManagerData->Dimension.Width + bounds.X > pSurface->GetWidth())
//			bounds.X = pSurface->GetWidth() - pManagerData->Dimension.Width;
//
//		bounds.Width = pManagerData->Dimension.Width;
//		bounds.Height += 4;
//
//		BitFont::Instance->field_41 = 1;
//		BitFont::Instance->SetBounds(&bounds);
//		BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB2DWORD(191, 98, 10));
//
//		BitText::Instance->DrawText(BitFont::Instance, pSurface, PhobosToolTip::Instance.GetBuffer(),
//			bounds.X + 4, bounds.Y + 2, bounds.Width, bounds.Height, 0, 0, 0);
//	}
//
//	return 0x479048;
//}


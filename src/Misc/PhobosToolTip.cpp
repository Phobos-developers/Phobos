#include "PhobosToolTip.h"

#include <GameOptionsClass.h>
#include <CCToolTip.h>
#include <FPSCounter.h>

#include <Ext/Side/Body.h>
#include <Ext/Surface/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>
#include <New/Type/ResourceTypeClass.h>

#include <sstream>
#include <iomanip>

PhobosToolTip PhobosToolTip::Instance;

inline bool PhobosToolTip::IsEnabled() const
{
	return Phobos::UI::ExtendedToolTips;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(TechnoTypeExt* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(SWTypeExt* pData) const
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
		VTable::Set(pTrick, BuildingClass::AbsVTable);
		reinterpret_cast<BuildingClass*>(pTrick)->Type = (BuildingTypeClass*)pType;
		break;
	case AbstractType::AircraftType:
		VTable::Set(pTrick, AircraftClass::AbsVTable);
		reinterpret_cast<AircraftClass*>(pTrick)->Type = (AircraftTypeClass*)pType;
		break;
	case AbstractType::InfantryType:
		VTable::Set(pTrick, InfantryClass::AbsVTable);
		reinterpret_cast<InfantryClass*>(pTrick)->Type = (InfantryTypeClass*)pType;
		break;
	case AbstractType::UnitType:
		VTable::Set(pTrick, UnitClass::AbsVTable);
		reinterpret_cast<UnitClass*>(pTrick)->Type = (UnitTypeClass*)pType;
		break;
	}

	// TechnoTypeClass only has 4 final classes :
	// BuildingTypeClass, AircraftTypeClass, InfantryTypeClass and UnitTypeClass
	// It has to be these four classes, otherwise pType will just be nullptr
	reinterpret_cast<TechnoClass*>(pTrick)->Owner = HouseClass::CurrentPlayer;
	const int nTimeToBuild = reinterpret_cast<TechnoClass*>(pTrick)->TimeToBuild();
	// 54 frames at least
	return std::max(54, nTimeToBuild);
}

inline int PhobosToolTip::GetPower(TechnoTypeClass* pType) const
{
	switch (pType->WhatAmI())
	{
	case AbstractType::AircraftType:
	case AbstractType::InfantryType:
	case AbstractType::UnitType:
		{
			if (!Phobos::Config::UnitPowerDrain)
				return 0;

			const auto pExt = TechnoTypeExt::Fetch(pType);
			return pExt->Power;
		}
	case AbstractType::BuildingType:
		{
			const auto pBldType = static_cast<BuildingTypeClass*>(pType);
			const auto [enhancedPower, extraPower] = BuildingTypeExt::GetEnhancedPower(pBldType, pBldType->PowerBonus, HouseClass::CurrentPlayer);
			return enhancedPower + extraPower - pBldType->PowerDrain;
		}
	default:
		return 0;
	}
}

inline const wchar_t* PhobosToolTip::GetBuffer() const
{
	return this->TextBuffer.c_str();
}

void PhobosToolTip::HelpText(BuildType& cameo)
{
	if (cameo.ItemType == AbstractType::Special)
		this->HelpText_Super(cameo.ItemIndex);
	else
		this->HelpText_Techno(ObjectTypeClass::GetTechnoType(cameo.ItemType, cameo.ItemIndex));
}

inline static int TickTimeToSeconds(int tickTime)
{
	if (!Phobos::Config::RealTimeTimers)
		return tickTime / 15;

	if (Phobos::Config::RealTimeTimers_Adaptive
		|| GameOptionsClass::Instance.GameSpeed == 0
		|| (Phobos::Misc::CustomGS && !SessionClass::IsMultiplayer()))
	{
		return tickTime / std::max((int)FPSCounter::CurrentFrameRate, 1);
	}

	return tickTime / (60 / GameOptionsClass::Instance.GameSpeed);
}

void PhobosToolTip::HelpText_Techno(TechnoTypeClass* pType)
{
	if (!pType)
		return;

	auto const pData = TechnoTypeExt::Fetch(pType);

	const int nBuildTime = TickTimeToSeconds(this->GetBuildTime(pType));
	const int nSec = nBuildTime % 60;
	const int nMin = nBuildTime / 60;

	const int cost = pType->GetActualCost(HouseClass::CurrentPlayer);

	std::wostringstream oss;
	oss << pType->UIName << L"\n";

	bool hasPreceding = false;

	if (cost != 0)
	{
		oss << Phobos::UI::CostLabel << std::abs(cost);
		hasPreceding = true;
	}

	for (size_t i = 0; i < pData->ResourceCosts.size(); ++i)
	{
		const int resCost = pData->ResourceCosts[i];
		if (resCost > 0 && i < ResourceTypeClass::Array.size())
		{
			const auto pResource = ResourceTypeClass::Array[i].get();
			if (pResource)
			{
				if (hasPreceding)
					oss << L" ";

				const wchar_t* label = pResource->Display_Label.Get();
				const bool useSpace = pResource->Display_Label_UseSpace.Get();
				if (label && *label)
				{
					if (pResource->Display_Label_InvertPosition.Get())
						oss << resCost << (useSpace ? L" " : L"") << label;
					else
						oss << label << (useSpace ? L" " : L"") << resCost;
				}
				else
				{
					oss << resCost;
				}

				hasPreceding = true;
			}
		}
	}

	if (hasPreceding)
		oss << L" ";

	oss << Phobos::UI::TimeLabel
		<< std::setw(2) << std::setfill(L'0') << nMin << L":"
		<< std::setw(2) << std::setfill(L'0') << nSec;

	if (auto const nPower = this->GetPower(pType))
	{
		oss << L" " << Phobos::UI::PowerLabel;
		if (nPower > 0)
			oss << L"+";
		oss << std::setw(1) << nPower;
	}

	if (auto const pDesc = this->GetUIDescription(pData))
		oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
}

void PhobosToolTip::HelpText_Super(int swidx)
{
	auto const pSuper = HouseClass::CurrentPlayer->Supers.Items[swidx];
	auto const pType = pSuper->Type;
	auto const pData = SWTypeExt::Fetch(pType);

	std::wostringstream oss;
	oss << pType->UIName;
	bool lineStarted = false;

	if (const int nCost = pData->Money_Amount)
	{
		oss << L"\n";
		lineStarted = true;

		// Negative money amount means granting money upon launch (+)
		if (nCost < 0)
			oss << L"+";

		oss << Phobos::UI::CostLabel << std::abs(nCost);
	}

	for (size_t i = 0; i < pData->SW_ResourceAmounts.size(); ++i)
	{
		const int nAmount = pData->SW_ResourceAmounts[i];
		if (nAmount != 0)
		{
			if (!lineStarted)
			{
				oss << L"\n";
				lineStarted = true;
			}
			else
			{
				oss << L" ";
			}

			const auto pResource = (i < ResourceTypeClass::Array.size()) ? ResourceTypeClass::Array[i].get() : nullptr;
			const wchar_t* label = pResource ? pResource->Display_Label.Get() : L"";
			const bool useSpace = pResource ? pResource->Display_Label_UseSpace.Get() : true;

			// Negative resource amount means granting resource upon launch (+)
			const bool isGrant = (nAmount < 0);

			if (pResource && pResource->Display_Label_InvertPosition.Get())
			{
				if (isGrant)
					oss << L"+";
				oss << std::abs(nAmount) << (useSpace ? L" " : L"") << label;
			}
			else
			{
				if (isGrant)
					oss << L"+";
				oss << label << (useSpace ? L" " : L"") << std::abs(nAmount);
			}
		}
	}

	const int rechargeTime = TickTimeToSeconds(pSuper->GetRechargeTime());
	if (rechargeTime > 0)
	{
		if (!lineStarted)
		{
			oss << L"\n";
			lineStarted = true;
		}
		else
		{
			oss << L" ";
		}

		const int nSec = rechargeTime % 60;
		const int nMin = rechargeTime / 60;

		oss << Phobos::UI::TimeLabel
			<< std::setw(2) << std::setfill(L'0') << nMin << L":"
			<< std::setw(2) << std::setfill(L'0') << nSec;
	}

	auto const& sw_ext = HouseExt::Fetch(HouseClass::CurrentPlayer)->SuperExts[swidx];
	const int sw_shots = pData->SW_Shots;
	const int remain_shots = pData->SW_Shots - sw_ext.ShotCount;
	if (sw_shots > 0)
	{
		if (!lineStarted)
		{
			oss << L"\n";
			lineStarted = true;
		}
		else
		{
			oss << L" ";
		}

		wchar_t buffer[64];
		swprintf_s(buffer, Phobos::UI::SWShotsFormat, remain_shots, sw_shots);
		oss << buffer;
	}

	if (auto const pDesc = this->GetUIDescription(pData))
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

DEFINE_HOOK(0x4AE51E, DisplayClass_GetToolTip_HelpText, 0x6)
{
	enum { ApplyToolTip = 0x4AE69D };

	if (SWSidebarClass::IsEnabled())
	{
		const auto& swSidebar = SWSidebarClass::Instance;

		if (const auto button = swSidebar.CurrentButton)
		{
			PhobosToolTip::Instance.IsCameo = true;

			if (PhobosToolTip::Instance.IsEnabled())
			{
				PhobosToolTip::Instance.HelpText_Super(button->SuperIndex);
				R->EAX(PhobosToolTip::Instance.GetBuffer());
			}
			else
			{
				const auto pSuper = HouseClass::CurrentPlayer->Supers[button->SuperIndex];
				R->EAX(pSuper->Type->UIName);
			}

			return ApplyToolTip;
		}
		else if (swSidebar.CurrentColumn || (swSidebar.ToggleButton && swSidebar.ToggleButton->IsHovering))
		{
			R->EAX(0);
			return ApplyToolTip;
		}
	}

	return 0;
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
	GET_STACK(const bool, bFullRedraw, 0x4);

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
		R->ESI(DSurface::Composite);
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
			R->EAX(DSurface::ViewBounds.Width);
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
		LEA_STACK(RectangleStruct*, Rect, STACK_OFFSET(0x3C, -0x20));

		int const maxHeight = DSurface::ViewBounds.Height - 32;

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
	GET(SurfaceExt*, pThis, ESI);
	LEA_STACK(RectangleStruct*, pRect, STACK_OFFSET(0x44, -0x10));

	const bool isCameo = PhobosToolTip::Instance.IsCameo;

	if (isCameo && Phobos::UI::AnchoredToolTips
		&& PhobosToolTip::Instance.IsEnabled()
		&& Phobos::Config::ToolTipDescriptions)
	{
		LEA_STACK(LTRBStruct*, pTextRect, STACK_OFFSET(0x44, -0x20));

		if (const auto pButton = SWSidebarClass::Instance.CurrentButton)
		{
			// Being too far away may actually be bad
			/*
			const auto& columns = SWSidebarClass::Instance.Columns;
			const int size = columns.size();
			const int y = pButton->Y + 3;

			auto pColumn = columns.back();
			if (columns.size() > 1 && y > (pColumn->Y + pColumn->Height))
				pColumn = columns[size - 2];

			const int x = pColumn->X + pColumn->Width + 2;
			*/
			GET_STACK(const int, textHeight, STACK_OFFSET(0x44, -0x28));

			const auto pColumn = SWSidebarClass::Instance.Columns[pButton->ColumnIndex];
			const int x = pColumn->X + pColumn->Width + 2;
			const int y = std::clamp(pButton->Y + 3, 0, DSurface::ViewBounds.Height - textHeight);
			pRect->X = x;
			pTextRect->Right += (x - pTextRect->Left);
			pTextRect->Left = x;
			pRect->Y = y;
			pTextRect->Bottom += (y - pTextRect->Top);
			pTextRect->Top = y;
		}
		else
		{
			const int x = DSurface::SidebarBounds.X - pRect->Width - 2;
			pRect->X = x;
			pTextRect->Left = x;
			pRect->Y -= 40;
			pTextRect->Top -= 40;
		}
	}
	else if (const auto pButton = SWSidebarClass::Instance.CurrentButton)
	{
		LEA_STACK(LTRBStruct*, pTextRect, STACK_OFFSET(0x44, -0x20));
		GET_STACK(const int, textHeight, STACK_OFFSET(0x44, -0x28));

		const int x = pButton->X + pButton->Width;
		const int y = std::clamp(pButton->Y + 43, 0, DSurface::ViewBounds.Height - textHeight);
		pRect->X = x;
		pTextRect->Right += (x - pTextRect->Left);
		pTextRect->Left = x;
		pRect->Y = y;
		pTextRect->Bottom += (y - pTextRect->Top);
		pTextRect->Top = y;
	}

	// Should we make some SideExt items as static to improve the effeciency?
	// Though it might not be a big improvement... - secsome
	const int nPlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (auto const pSide = SideClass::Array.GetItemOrDefault(nPlayerSideIndex))
	{
		if (auto const pData = SideExt::TryFetch(pSide))
		{
			// Could this flag be lazy?
			if (isCameo)
				SidebarClass::Instance.SidebarBackgroundNeedsRedraw = true;

			pThis->FillRectTrans(pRect,
				pData->ToolTip_Background_Color.GetEx(&RulesExt::Global()->ToolTip_Background_Color),
				pData->ToolTip_Background_Opacity.Get(RulesExt::Global()->ToolTip_Background_Opacity)
			);

			if (Phobos::Config::ToolTipBlur)
				pThis->BlurRect(*pRect, pData->ToolTip_Background_BlurSize.Get(RulesExt::Global()->ToolTip_Background_BlurSize));

			return (int)_CCToolTip_Draw2_FillRect_RET;
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
//	if (GameOptionsClass::Instance.SidebarSide == 1)
//	{
//		int nR = DSurface::ViewBounds.X + DSurface::ViewBounds.Width;
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
//		int nR = DSurface::SidebarBounds.X + DSurface::SidebarBounds.Width;
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
//			Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : DSurface::WindowBounds.Width);
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

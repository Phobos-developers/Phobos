#include "ResourceTypeClass.h"

#include <HouseClass.h>
#include <SideClass.h>
#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Surface.h>
#include <Drawing.h>
#include <FileSystem.h>
#include <PCX.h>

template<>
const char* Enumerable<ResourceTypeClass>::GetMainSection()
{
	return "ResourceTypes";
}

ResourceTypeClass::ResourceTypeClass(const char* pTitle) : Enumerable<ResourceTypeClass>(pTitle)
	, Display_Label {}
	, Display_Label_InvertPosition { false }
	, Display_Label_UseSpace { pTitle ? (_strcmpi(pTitle, "Money") != 0 && _strcmpi(pTitle, "Power") != 0 && _strcmpi(pTitle, "Harvesters") != 0 && _strcmpi(pTitle, "Weeds") != 0) : true }
	, Display_Color {}
	, Display_Condition { (pTitle && (_strcmpi(pTitle, "Harvesters") == 0 || _strcmpi(pTitle, "Weeds") == 0)) ? ResourceDisplayCondition::Never : ResourceDisplayCondition::Always }
	, Display_Offset {}
	, InitialValue { 0 }
	, RequiresCollector { false }
	, Bounty_Enabled { false }
	, Bounty_DefaultValue { 0 }
	, Bounty_DefaultFriendlyValue { 0 }
	, Bounty_CanUseStandardPoints { false }
	, Bounty_MoneyConversion { 100 }
	, Crate_Amount { 0 }
	, Crate_Sound {}
	, Crate_Anim {}
	, Display_Power_Mode { ResourcePowerDisplayMode::NetAndTotal }
	, Display_Power_ColorGreen {}
	, Display_Power_ColorYellow {}
	, Display_Power_ColorRed {}
	, Display_Power_ColorGrey {}
	, Display_Harvester_Mode { ResourceHarvesterDisplayMode::ActiveAndTotal }
	, Display_Harvester_ColorGreen {}
	, Display_Harvester_ColorYellow {}
	, Display_Harvester_ColorRed {}
{
}

bool ResourceTypeClass::HasMoneyResource()
{
	for (const auto& pResource : ResourceTypeClass::Array)
	{
		if (pResource && pResource->IsMoneyResource())
			return true;
	}
	return false;
}

bool ResourceTypeClass::HasPowerResource()
{
	for (const auto& pResource : ResourceTypeClass::Array)
	{
		if (pResource && pResource->IsPowerResource())
			return true;
	}
	return false;
}

bool ResourceTypeClass::HasHarvesterResource()
{
	for (const auto& pResource : ResourceTypeClass::Array)
	{
		if (pResource && pResource->IsHarvesterResource())
			return true;
	}
	return false;
}

bool ResourceTypeClass::HasWeedsResource()
{
	for (const auto& pResource : ResourceTypeClass::Array)
	{
		if (pResource && pResource->IsWeedsResource())
			return true;
	}
	return false;
}

bool ResourceTypeClass::ShouldSkipWestwoodCredits()
{
	if (!ResourceTypeClass::HasMoneyResource())
		return false;

	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer || pPlayer->Defeated)
		return false;

	auto pSideExt = SideExt::Fetch(SideClass::Array.GetItem(pPlayer->SideIndex));
	if (!pSideExt)
		return false;

	const bool hasExplicitSidebarTypes = !pSideExt->Sidebar_ResourceTypes_Types.empty();
	const auto sideAnchor = pSideExt->Display_ResourceTypes_Anchor.Get();

	if (hasExplicitSidebarTypes)
	{
		for (const auto idx : pSideExt->Sidebar_ResourceTypes_Types)
		{
			if (idx >= 0 && idx < static_cast<int>(ResourceTypeClass::Array.size()))
			{
				if (ResourceTypeClass::Array[idx]->IsMoneyResource())
					return true;
			}
		}
		return (sideAnchor != ResourceDisplayAnchor::Sidebar);
	}

	return (sideAnchor != ResourceDisplayAnchor::Sidebar);
}

void ResourceTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;
	if (this->IsMoneyResource() || this->IsPowerResource() || this->IsHarvesterResource() || this->IsWeedsResource())
	{
		this->Display_Label_UseSpace = false;
	}

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Display_Label.Read(exINI, section, "Display.Label");
	this->Display_Label_InvertPosition.Read(exINI, section, "Display.Label.InvertPosition");
	this->Display_Label_UseSpace.Read(exINI, section, "Display.Label.UseSpace");
	this->Display_Color.Read(exINI, section, "Display.Color");
	this->Display_Condition.Read(exINI, section, "Display.Condition");
	if (this->IsHarvesterResource() && this->Display_Condition.Get() == ResourceDisplayCondition::HasCollector)
	{
		this->Display_Condition = ResourceDisplayCondition::Always;
	}
	this->Display_Offset.Read(exINI, section, "Display.Offset");

	this->InitialValue.Read(exINI, section, "InitialValue");
	this->RequiresCollector.Read(exINI, section, "RequiresCollector");

	this->Bounty_Enabled.Read(exINI, section, "Bounty.Enabled");
	this->Bounty_DefaultValue.Read(exINI, section, "Bounty.DefaultValue");
	this->Bounty_DefaultFriendlyValue.Read(exINI, section, "Bounty.DefaultFriendlyValue");
	this->Bounty_CanUseStandardPoints.Read(exINI, section, "Bounty.CanUseStandardPoints");
	this->Bounty_MoneyConversion.Read(exINI, section, "Bounty.MoneyConversion");

	this->Crate_Amount.Read(exINI, section, "Crate.Amount");
	this->Crate_Sound.Read(exINI, section, "Crate.Sound");
	this->Crate_Anim.Read(exINI, section, "Crate.Anim");

	this->Display_Power_Mode.Read(exINI, section, "Display.Power.Mode");
	this->Display_Power_Mode.Read(exINI, section, "Display.Power.Format");

	this->Display_Power_ColorGreen.Read(exINI, section, "Display.Power.ColorGreen");
	this->Display_Power_ColorGreen.Read(exINI, section, "Display.ColorGreen");
	this->Display_Power_ColorYellow.Read(exINI, section, "Display.Power.ColorYellow");
	this->Display_Power_ColorYellow.Read(exINI, section, "Display.ColorYellow");
	this->Display_Power_ColorRed.Read(exINI, section, "Display.Power.ColorRed");
	this->Display_Power_ColorRed.Read(exINI, section, "Display.ColorRed");
	this->Display_Power_ColorGrey.Read(exINI, section, "Display.Power.ColorGrey");
	this->Display_Power_ColorGrey.Read(exINI, section, "Display.ColorGrey");

	this->Display_Harvester_Mode.Read(exINI, section, "Display.Harvesters.Mode");
	this->Display_Harvester_Mode.Read(exINI, section, "Display.Harvesters.Format");

	this->Display_Harvester_ColorGreen.Read(exINI, section, "Display.Harvesters.ColorGreen");
	this->Display_Harvester_ColorGreen.Read(exINI, section, "Display.ColorGreen");
	this->Display_Harvester_ColorYellow.Read(exINI, section, "Display.Harvesters.ColorYellow");
	this->Display_Harvester_ColorYellow.Read(exINI, section, "Display.ColorYellow");
	this->Display_Harvester_ColorRed.Read(exINI, section, "Display.Harvesters.ColorRed");
	this->Display_Harvester_ColorRed.Read(exINI, section, "Display.ColorRed");

	bool showTotal = true;
	if (exINI.ReadBool(section, "Display.Power.ShowTotal", &showTotal))
	{
		this->Display_Power_Mode = showTotal ? ResourcePowerDisplayMode::NetAndTotal : ResourcePowerDisplayMode::Net;
	}
}

template <typename T>
void ResourceTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Display_Label)
		.Process(this->Display_Label_InvertPosition)
		.Process(this->Display_Label_UseSpace)
		.Process(this->Display_Color)
		.Process(this->Display_Condition)
		.Process(this->Display_Offset)
		.Process(this->InitialValue)
		.Process(this->RequiresCollector)
		.Process(this->Bounty_Enabled)
		.Process(this->Bounty_DefaultValue)
		.Process(this->Bounty_DefaultFriendlyValue)
		.Process(this->Bounty_CanUseStandardPoints)
		.Process(this->Bounty_MoneyConversion)
		.Process(this->Crate_Amount)
		.Process(this->Crate_Sound)
		.Process(this->Crate_Anim)
		.Process(this->Display_Power_Mode)
		.Process(this->Display_Power_ColorGreen)
		.Process(this->Display_Power_ColorYellow)
		.Process(this->Display_Power_ColorRed)
		.Process(this->Display_Power_ColorGrey)
		.Process(this->Display_Harvester_Mode)
		.Process(this->Display_Harvester_ColorGreen)
		.Process(this->Display_Harvester_ColorYellow)
		.Process(this->Display_Harvester_ColorRed);
}

void ResourceTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ResourceTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void ResourceTypeClass::DrawResourceHUD(DSurface* pSurface, bool isSidebar)
{
	if (!pSurface)
		return;

	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer || pPlayer->Defeated)
		return;

	auto pHouseExt = HouseExt::Fetch(pPlayer);
	auto pSideExt = SideExt::Fetch(SideClass::Array.GetItem(pPlayer->SideIndex));

	const auto sideAnchor = pSideExt ? pSideExt->Display_ResourceTypes_Anchor.Get() : ResourceDisplayAnchor::TopRight;
	const auto sideOrientation = pSideExt ? pSideExt->Display_ResourceTypes_Orientation.Get() : ResourceDisplayOrientation::Vertical;
	const Point2D sideBaseOffset = pSideExt ? pSideExt->Display_ResourceTypes_BaseOffset.Get() : Point2D::Empty;
	const int spacing = pSideExt ? pSideExt->Display_ResourceTypes_Spacing.Get() : 14;
	const bool resourcesInside = pSideExt && pSideExt->Display_ResourceTypes_Background_Horizontal_ResourcesInside.Get();
	const bool hasExplicitSidebarTypes = pSideExt && !pSideExt->Sidebar_ResourceTypes_Types.empty();

	if (isSidebar)
	{
		if (sideAnchor != ResourceDisplayAnchor::Sidebar && !hasExplicitSidebarTypes)
			return;
	}
	else
	{
		if (sideAnchor == ResourceDisplayAnchor::Sidebar)
			return;
	}

	RectangleStruct vRect = pSurface->GetRect();

	int baseX = 0;
	int baseY = 0;

	const int marginX = Phobos::Config::MessageDisplayInCenter ? 28 : 10;
	const int screenW = pSurface->GetWidth();
	const int screenH = pSurface->GetHeight();

	if (isSidebar)
	{
		int defaultBaseX = 20;
		if (pSideExt->Sidebar_ResourceTypes_Align.Get() == TextAlign::Right)
			defaultBaseX = 140;
		else if (pSideExt->Sidebar_ResourceTypes_Align.Get() == TextAlign::Center)
			defaultBaseX = pSurface->GetWidth() / 2;

		const Point2D sideOffset = pSideExt->Sidebar_ResourceTypes_Offset.Get();
		baseX = (sideOffset.X != 0 ? sideOffset.X : defaultBaseX) + (sideAnchor == ResourceDisplayAnchor::Sidebar ? sideBaseOffset.X : 0);
		baseY = 2 + sideOffset.Y + (sideAnchor == ResourceDisplayAnchor::Sidebar ? sideBaseOffset.Y : 0);
	}
	else
	{
		switch (sideAnchor)
		{
		case ResourceDisplayAnchor::TopLeft:
			baseX = marginX + sideBaseOffset.X;
			baseY = 5 + sideBaseOffset.Y;
			break;
		case ResourceDisplayAnchor::TopRight:
			baseX = screenW - marginX + sideBaseOffset.X;
			baseY = 5 + sideBaseOffset.Y;
			break;
		case ResourceDisplayAnchor::TopCenter:
			baseX = (screenW / 2) + sideBaseOffset.X;
			baseY = 5 + sideBaseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomLeft:
			baseX = marginX + sideBaseOffset.X;
			baseY = screenH - 20 + sideBaseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomRight:
			baseX = screenW - marginX + sideBaseOffset.X;
			baseY = screenH - 20 + sideBaseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomCenter:
			baseX = (screenW / 2) + sideBaseOffset.X;
			baseY = screenH - 20 + sideBaseOffset.Y;
			break;
		default:
			baseX = marginX + sideBaseOffset.X;
			baseY = 5 + sideBaseOffset.Y;
			break;
		}
	}

	int stackIndex = 0;
	int runningRightX = baseX;
	int runningLeftX = baseX;

	TextAlign effectiveAlign;
	if (isSidebar)
	{
		effectiveAlign = pSideExt ? pSideExt->Sidebar_ResourceTypes_Align.Get() : TextAlign::Left;
	}
	else
	{
		if (pSideExt && pSideExt->Display_ResourceTypes_Align.isset())
			effectiveAlign = pSideExt->Display_ResourceTypes_Align.Get();
		else if (sideAnchor == ResourceDisplayAnchor::TopCenter || sideAnchor == ResourceDisplayAnchor::BottomCenter)
			effectiveAlign = TextAlign::Center;
		else if (sideAnchor == ResourceDisplayAnchor::TopRight || sideAnchor == ResourceDisplayAnchor::BottomRight)
			effectiveAlign = TextAlign::Right;
		else
			effectiveAlign = TextAlign::Left;
	}

	const bool isRightAlign = (effectiveAlign == TextAlign::Right);
	const bool isCenterAlign = (effectiveAlign == TextAlign::Center);
	const bool isRightAnchor = (sideAnchor == ResourceDisplayAnchor::TopRight || sideAnchor == ResourceDisplayAnchor::BottomRight);
	const bool isCenterAnchor = (sideAnchor == ResourceDisplayAnchor::TopCenter || sideAnchor == ResourceDisplayAnchor::BottomCenter);
	const bool isBottomAnchor = !isSidebar && (sideAnchor == ResourceDisplayAnchor::BottomLeft || sideAnchor == ResourceDisplayAnchor::BottomRight || sideAnchor == ResourceDisplayAnchor::BottomCenter);

	const size_t count = (isSidebar && hasExplicitSidebarTypes) ? pSideExt->Sidebar_ResourceTypes_Types.size() : ResourceTypeClass::Array.size();

	int bgWidth = 0;
	int bgHeight = 0;
	BSurface* pPCX = pSideExt ? pSideExt->Display_ResourceTypes_Background_PCX.GetSurface() : nullptr;
	SHPStruct* pSHP = pSideExt ? pSideExt->Display_ResourceTypes_Background_SHP.Get() : nullptr;

	if (pPCX)
	{
		bgWidth = pPCX->GetWidth();
		bgHeight = pPCX->GetHeight();
	}
	else if (pSHP)
	{
		bgWidth = pSHP->Width;
		bgHeight = pSHP->Height;
	}

	Point2D const bgOffset = pSideExt ? pSideExt->Display_ResourceTypes_Background_Offset.Get() : Point2D::Empty;
	Point2D bgPos;
	if (isRightAnchor)
		bgPos.X = (baseX - bgWidth) + bgOffset.X;
	else if (isCenterAnchor)
		bgPos.X = (baseX - bgWidth / 2) + bgOffset.X;
	else
		bgPos.X = baseX + bgOffset.X;

	bgPos.Y = (isBottomAnchor ? (baseY - bgHeight) : baseY) + bgOffset.Y;

	bool bgDrawn = false;
	auto DrawBackgroundOnce = [&]()
	{
		if (bgDrawn || isSidebar || (bgWidth == 0 && bgHeight == 0) || !pSideExt)
			return;
		bgDrawn = true;

		if (pPCX)
		{
			RectangleStruct drawRect { bgPos.X, bgPos.Y, bgWidth, bgHeight };
			PCX::Instance.BlitToSurface(&drawRect, pSurface, pPCX);
		}
		else if (pSHP)
		{
			ConvertClass* pConvert = pSideExt->Display_ResourceTypes_Background_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
			int const frame = pSHP->Frames > 1 ? (Unsorted::CurrentFrame % pSHP->Frames) : 0;

			pSurface->DrawSHP
			(
				pConvert,
				pSHP,
				frame,
				&bgPos,
				&vRect,
				BlitterFlags::None,
				0,
				0,
				ZGradient::Ground,
				1000,
				0,
				nullptr,
				0,
				0,
				0
			);
		}
	};

	std::vector<int> visibleResourceIndices;
	int dynamicSidebarCount = 0;
	int dynamicTacticalCount = 0;

	for (size_t iter = 0; iter < count; ++iter)
	{
		const int i = (isSidebar && hasExplicitSidebarTypes) ? pSideExt->Sidebar_ResourceTypes_Types[iter] : static_cast<int>(iter);
		if (i < 0 || i >= static_cast<int>(ResourceTypeClass::Array.size()))
			continue;

		const auto pResource = ResourceTypeClass::Array[i].get();
		if (!pResource || !pHouseExt->IsResourceEnabled(i))
			continue;

		// If drawing tactical HUD (!isSidebar), skip any resource that is already being drawn on the sidebar by this side's explicit list
		if (!isSidebar && hasExplicitSidebarTypes)
		{
			bool alreadyInSidebar = false;
			for (const auto sIdx : pSideExt->Sidebar_ResourceTypes_Types)
			{
				if (sIdx == i)
				{
					alreadyInSidebar = true;
					break;
				}
			}
			if (alreadyInSidebar)
				continue;
		}

		if (pResource->Display_Condition.Get() == ResourceDisplayCondition::Never)
			continue;

		// If on sidebar without an explicit Sidebar.ResourceTypes.Types list, Money remains at its default Westwood position
		if (isSidebar && !hasExplicitSidebarTypes && pResource->IsMoneyResource())
			continue;

		bool hasCollector = false;
		if (pResource->IsPowerResource())
		{
			hasCollector = pPlayer->Buildings.Count > 0;
		}
		else if (pResource->IsHarvesterResource())
		{
			hasCollector = HouseExt::TotalHarvesterCount(pPlayer) > 0;
		}
		else if (pResource->IsWeedsResource())
		{
			const bool hasCustomCollector = (i < static_cast<int>(pHouseExt->ResourceCollectorCounts.size())) && (pHouseExt->ResourceCollectorCounts[i] > 0);
			hasCollector = hasCustomCollector || (pPlayer->OwnedWeed.GetTotalAmount() > 0) || (HouseExt::ActiveHarvesterCount(pPlayer) > 0);
		}
		else if (pResource->IsMoneyResource())
		{
			const bool hasCustomCollector = (i < static_cast<int>(pHouseExt->ResourceCollectorCounts.size())) && (pHouseExt->ResourceCollectorCounts[i] > 0);
			hasCollector = hasCustomCollector || (HouseExt::ActiveHarvesterCount(pPlayer) > 0);
		}
		else
		{
			hasCollector = (i < static_cast<int>(pHouseExt->ResourceCollectorCounts.size())) && (pHouseExt->ResourceCollectorCounts[i] > 0);
		}

		if (!hasCollector)
		{
			if (pResource->Display_Condition.Get() == ResourceDisplayCondition::GreaterThanZero)
			{
				if (pResource->IsPowerResource())
				{
					if ((pPlayer->PowerOutput - pPlayer->PowerDrain) <= 0)
						continue;
				}
				else if (pResource->IsHarvesterResource())
				{
					if (HouseExt::TotalHarvesterCount(pPlayer) <= 0)
						continue;
				}
				else if (pResource->IsWeedsResource())
				{
					if (pPlayer->OwnedWeed.GetTotalAmount() <= 0)
						continue;
				}
				else if (pHouseExt->GetResourceAmount(i) <= 0)
				{
					continue;
				}
			}

			if (pResource->Display_Condition.Get() == ResourceDisplayCondition::HasCollector)
				continue;
		}

		visibleResourceIndices.push_back(i);
		if (isSidebar)
		{
			if (!pResource->Display_Offset.isset())
				++dynamicSidebarCount;
		}
		else
		{
			if (!pResource->Display_Offset.isset())
				++dynamicTacticalCount;
		}
	}

	int dynamicSidebarIdx = 0;
	int dynamicTacticalIdx = 0;

	for (const int i : visibleResourceIndices)
	{
		const auto pResource = ResourceTypeClass::Array[i].get();

		DrawBackgroundOnce();

		wchar_t counter[0x40];
		const wchar_t* label = pResource->Display_Label.Get();
		ColorStruct clrToolTip;

		if (pResource->IsPowerResource())
		{
			if (!label || !*label)
				label = L"\u26a1";

			const int output = pPlayer->PowerOutput;
			const int drain = pPlayer->PowerDrain;
			const int net = output - drain;

			wchar_t powerVal[0x40];
			switch (pResource->Display_Power_Mode.Get())
			{
			case ResourcePowerDisplayMode::NetAndTotal:
				swprintf_s(powerVal, L"%d / %d", net, output);
				break;
			case ResourcePowerDisplayMode::Net:
				swprintf_s(powerVal, L"%d", net);
				break;
			case ResourcePowerDisplayMode::DrainAndTotal:
				swprintf_s(powerVal, L"%d / %d", drain, output);
				break;
			case ResourcePowerDisplayMode::Drain:
				swprintf_s(powerVal, L"%d", drain);
				break;
			case ResourcePowerDisplayMode::Total:
				swprintf_s(powerVal, L"%d", output);
				break;
			}

			if (label && *label)
			{
				const bool useSpace = pResource->Display_Label_UseSpace.Get();
				if (pResource->Display_Label_InvertPosition.Get())
					swprintf_s(counter, useSpace ? L"%ls %ls" : L"%ls%ls", powerVal, label);
				else
					swprintf_s(counter, useSpace ? L"%ls %ls" : L"%ls%ls", label, powerVal);
			}
			else
			{
				wcscpy_s(counter, powerVal);
			}

			const bool hasSpecificPowerColors = pResource->Display_Power_ColorGreen.isset()
				|| pResource->Display_Power_ColorYellow.isset()
				|| pResource->Display_Power_ColorRed.isset()
				|| pResource->Display_Power_ColorGrey.isset();

			if (pResource->Display_Color.isset() && !hasSpecificPowerColors)
			{
				clrToolTip = pResource->Display_Color.Get();
			}
			else
			{
				if (pPlayer->PowerBlackoutTimer.InProgress())
				{
					clrToolTip = pResource->Display_Power_ColorGrey.Get(ColorStruct { 128, 128, 128 });
				}
				else
				{
					const double percent = output != 0
						? (double)drain / (double)output
						: (drain != 0 ? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow);

					if (percent < Phobos::UI::PowerDelta_ConditionYellow)
					{
						clrToolTip = pResource->Display_Power_ColorGreen.Get(ColorStruct { 0, 255, 0 });
					}
					else if (LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed))
					{
						clrToolTip = pResource->Display_Power_ColorYellow.Get(ColorStruct { 255, 255, 0 });
					}
					else
					{
						clrToolTip = pResource->Display_Power_ColorRed.Get(ColorStruct { 255, 0, 0 });
					}
				}
			}
		}
		else if (pResource->IsHarvesterResource())
		{
			if (!label || !*label)
				label = L"\u26cf";

			const int nActive = HouseExt::ActiveHarvesterCount(pPlayer);
			const int nTotal = HouseExt::TotalHarvesterCount(pPlayer);
			const double nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;

			wchar_t harvesterVal[0x40];
			switch (pResource->Display_Harvester_Mode.Get())
			{
			case ResourceHarvesterDisplayMode::ActiveAndTotal:
				swprintf_s(harvesterVal, L"%d / %d", nActive, nTotal);
				break;
			case ResourceHarvesterDisplayMode::Active:
				swprintf_s(harvesterVal, L"%d", nActive);
				break;
			case ResourceHarvesterDisplayMode::Total:
				swprintf_s(harvesterVal, L"%d", nTotal);
				break;
			}

			if (label && *label)
			{
				const bool useSpace = pResource->Display_Label_UseSpace.Get();
				if (pResource->Display_Label_InvertPosition.Get())
					swprintf_s(counter, useSpace ? L"%ls %ls" : L"%ls%ls", harvesterVal, label);
				else
					swprintf_s(counter, useSpace ? L"%ls %ls" : L"%ls%ls", label, harvesterVal);
			}
			else
			{
				wcscpy_s(counter, harvesterVal);
			}

			const bool hasSpecificHarvesterColors = pResource->Display_Harvester_ColorGreen.isset()
				|| pResource->Display_Harvester_ColorYellow.isset()
				|| pResource->Display_Harvester_ColorRed.isset();

			if (pResource->Display_Color.isset() && !hasSpecificHarvesterColors)
			{
				clrToolTip = pResource->Display_Color.Get();
			}
			else
			{
				if (nActive >= nTotal || nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow)
				{
					clrToolTip = pResource->Display_Harvester_ColorGreen.Get(ColorStruct { 0, 255, 0 });
				}
				else if (nPercentage > Phobos::UI::HarvesterCounter_ConditionRed)
				{
					clrToolTip = pResource->Display_Harvester_ColorYellow.Get(ColorStruct { 255, 255, 0 });
				}
				else
				{
					clrToolTip = pResource->Display_Harvester_ColorRed.Get(ColorStruct { 255, 0, 0 });
				}
			}
		}
		else if (pResource->IsWeedsResource())
		{
			const int weedAmount = static_cast<int>(pPlayer->OwnedWeed.GetTotalAmount());
			if (label && *label)
			{
				const bool useSpace = pResource->Display_Label_UseSpace.Get();
				if (pResource->Display_Label_InvertPosition.Get())
					swprintf_s(counter, useSpace ? L"%d %ls" : L"%d%ls", weedAmount, label);
				else
					swprintf_s(counter, useSpace ? L"%ls %d" : L"%ls%d", label, weedAmount);
			}
			else
			{
				swprintf_s(counter, L"%d", weedAmount);
			}

			clrToolTip = pResource->Display_Color.Get(pSideExt ? pSideExt->Sidebar_ResourceTypes_Color.Get(Drawing::TooltipColor) : Drawing::TooltipColor);
		}
		else
		{
			if (pResource->IsMoneyResource() && (!label || !*label))
				label = L"$";

			const int amount = pHouseExt->GetResourceAmount(i);
			if (label && *label)
			{
				const bool useSpace = pResource->Display_Label_UseSpace.Get();
				if (pResource->Display_Label_InvertPosition.Get())
					swprintf_s(counter, useSpace ? L"%d %ls" : L"%d%ls", amount, label);
				else
					swprintf_s(counter, useSpace ? L"%ls %d" : L"%ls%d", label, amount);
			}
			else
			{
				swprintf_s(counter, L"%d", amount);
			}

			clrToolTip = pResource->Display_Color.Get(pSideExt->Sidebar_ResourceTypes_Color.Get(Drawing::TooltipColor));
		}

		Point2D vPos;
		if (pResource->Display_Offset.isset())
		{
			vPos = pResource->Display_Offset.Get();
		}
		else if (isSidebar)
		{
			if (dynamicSidebarCount <= 1)
			{
				vPos.X = baseX;
				vPos.Y = baseY;
			}
			else
			{
				int posX = 0;
				if (effectiveAlign == TextAlign::Left)
					posX = (dynamicSidebarIdx * screenW) / dynamicSidebarCount + 10;
				else if (effectiveAlign == TextAlign::Right)
					posX = ((dynamicSidebarIdx + 1) * screenW) / dynamicSidebarCount - 10;
				else
					posX = (2 * dynamicSidebarIdx + 1) * screenW / (2 * dynamicSidebarCount);

				const Point2D sideOffset = pSideExt->Sidebar_ResourceTypes_Offset.Get();
				vPos.X = posX + sideOffset.X;
				vPos.Y = baseY;
				++dynamicSidebarIdx;
			}
		}
		else if (bgWidth > 0 && resourcesInside && sideOrientation == ResourceDisplayOrientation::Horizontal)
		{
			auto const dim = Drawing::GetTextDimensions(counter, { 0, 0 }, 0, 2, 0);

			if (dynamicTacticalCount <= 1)
			{
				if (effectiveAlign == TextAlign::Left)
					vPos.X = bgPos.X + 10;
				else if (effectiveAlign == TextAlign::Right)
					vPos.X = (bgPos.X + bgWidth) - dim.Width - 10;
				else
					vPos.X = bgPos.X + (bgWidth - dim.Width) / 2;
			}
			else
			{
				int posX = 0;
				if (effectiveAlign == TextAlign::Left)
					posX = bgPos.X + (dynamicTacticalIdx * bgWidth) / dynamicTacticalCount + 10;
				else if (effectiveAlign == TextAlign::Right)
					posX = bgPos.X + ((dynamicTacticalIdx + 1) * bgWidth) / dynamicTacticalCount - dim.Width - 10;
				else
				{
					int slotCenter = bgPos.X + (2 * dynamicTacticalIdx + 1) * bgWidth / (2 * dynamicTacticalCount);
					posX = slotCenter - (dim.Width / 2);
				}

				vPos.X = posX;
				++dynamicTacticalIdx;
			}

			vPos.Y = bgPos.Y + (bgHeight > dim.Height ? (bgHeight - dim.Height) / 2 : 0);
		}
		else
		{
			auto const dim = Drawing::GetTextDimensions(counter, { 0, 0 }, 0, 2, 0);

			if (sideOrientation == ResourceDisplayOrientation::Horizontal)
			{
				if (isRightAlign)
				{
					vPos.X = runningRightX - dim.Width;
					vPos.Y = baseY;
					runningRightX -= (dim.Width + spacing);
				}
				else if (isCenterAlign)
				{
					vPos.X = runningLeftX - (dim.Width / 2);
					vPos.Y = baseY;
					runningLeftX += (dim.Width + spacing);
				}
				else
				{
					vPos.X = runningLeftX;
					vPos.Y = baseY;
					runningLeftX += (dim.Width + spacing);
				}
			}
			else
			{
				if (isRightAlign)
					vPos.X = baseX - dim.Width;
				else if (isCenterAlign)
					vPos.X = baseX - (dim.Width / 2);
				else
					vPos.X = baseX;

				vPos.Y = isBottomAnchor ? (baseY - bgHeight > 0 ? baseY - stackIndex * spacing : baseY) : (baseY + stackIndex * spacing);
				++stackIndex;
			}
		}

		if (isSidebar)
		{
			auto const TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
					| static_cast<int>(effectiveAlign));
			pSurface->DrawText(counter, &vRect, &vPos, Drawing::RGB_To_Int(clrToolTip), 0, TextFlags);
		}
		else
		{
			Point2D loc { vPos.X, vPos.Y };
			pSurface->DrawText(counter, &loc, Drawing::RGB_To_Int(clrToolTip));
		}
	}
}

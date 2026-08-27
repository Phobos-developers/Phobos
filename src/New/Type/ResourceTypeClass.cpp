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

Valueable<ResourceDisplayOrientation> ResourceTypeClass::Global_Display_Orientation { ResourceDisplayOrientation::Vertical };
Valueable<ResourceDisplayAnchor> ResourceTypeClass::Global_Display_Anchor { ResourceDisplayAnchor::TopRight };
Valueable<Point2D> ResourceTypeClass::Global_Display_BaseOffset { Point2D::Empty };
Valueable<int> ResourceTypeClass::Global_Display_Spacing { 14 };
Nullable<TextAlign> ResourceTypeClass::Global_Display_Align {};
PhobosPCXFile ResourceTypeClass::Global_Display_Background_PCX {};
Valueable<SHPStruct*> ResourceTypeClass::Global_Display_Background_SHP { nullptr };
CustomPalette ResourceTypeClass::Global_Display_Background_Palette {};
Valueable<Point2D> ResourceTypeClass::Global_Display_Background_Offset { Point2D::Empty };

ResourceTypeClass::ResourceTypeClass(const char* pTitle) : Enumerable<ResourceTypeClass>(pTitle)
	, Display_Label {}
	, Display_Label_InvertPosition { false }
	, Display_Label_UseSpace { pTitle ? (_strcmpi(pTitle, "Money") != 0 && _strcmpi(pTitle, "Power") != 0) : true }
	, Display_Color {}
	, Display_Condition { (pTitle && _strcmpi(pTitle, "Power") == 0) ? ResourceDisplayCondition::Never : ResourceDisplayCondition::Always }
	, Display_Offset {}
	, InitialValue { 0 }
	, RequiresCollector { false }
	, Bounty_Enabled { false }
	, Bounty_DefaultValue { 0 }
	, Bounty_DefaultFriendlyValue { 0 }
	, Bounty_CanUseStandardPoints { false }
	, Bounty_MoneyConversion { 0 }
	, Crate_Amount { 0 }
	, Crate_Sound {}
	, Crate_Anim {}
	, Display_Power_Mode { ResourcePowerDisplayMode::NetAndTotal }
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

bool ResourceTypeClass::ShouldSkipWestwoodCredits()
{
	if (!ResourceTypeClass::HasMoneyResource())
		return false;

	const auto globalAnchor = ResourceTypeClass::Global_Display_Anchor.Get();
	if (globalAnchor != ResourceDisplayAnchor::Sidebar)
		return true;

	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer || pPlayer->Defeated)
		return false;

	auto pSideExt = SideExt::Fetch(SideClass::Array.GetItem(pPlayer->SideIndex));
	if (pSideExt && !pSideExt->Sidebar_ResourceTypes_Types.empty())
	{
		for (const auto idx : pSideExt->Sidebar_ResourceTypes_Types)
		{
			if (idx >= 0 && idx < static_cast<int>(ResourceTypeClass::Array.size()))
			{
				if (ResourceTypeClass::Array[idx]->IsMoneyResource())
					return true;
			}
		}
	}

	return false;
}

void ResourceTypeClass::LoadGlobalsFromINI(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	Global_Display_Orientation.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Orientation");
	Global_Display_Anchor.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Anchor");
	Global_Display_BaseOffset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.BaseOffset");
	Global_Display_Spacing.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Spacing");
	Global_Display_Align.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Align");

	Global_Display_Background_PCX.Read(pINI, GameStrings::AudioVisual, "Display.ResourceTypes.BackgroundPCX");
	Global_Display_Background_PCX.Read(pINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.PCX");

	Global_Display_Background_SHP.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background");
	Global_Display_Background_SHP.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.SHP");
	Global_Display_Background_SHP.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.Shape");

	Global_Display_Background_Offset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.Offset");
	Global_Display_Background_Offset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.BackgroundOffset");
	Global_Display_Background_Offset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.BackgroundPCX.Offset");
	Global_Display_Background_Offset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.BackgroundPCXOffset");
	Global_Display_Background_Palette.LoadFromINI(pINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.Palette");
	Global_Display_Background_Palette.LoadFromINI(pINI, GameStrings::AudioVisual, "Display.ResourceTypes.Background.SHP.Palette");
}

void ResourceTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;
	if (this->IsMoneyResource() || this->IsPowerResource())
	{
		this->Display_Label_UseSpace = false;
	}

	if (this->IsPowerResource())
	{
		this->Display_Condition = ResourceDisplayCondition::Never;
	}

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Display_Label.Read(exINI, section, "Display.Label");
	this->Display_Label_InvertPosition.Read(exINI, section, "Display.Label.InvertPosition");
	this->Display_Label_UseSpace.Read(exINI, section, "Display.Label.UseSpace");
	this->Display_Color.Read(exINI, section, "Display.Color");
	this->Display_Condition.Read(exINI, section, "Display.Condition");
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
		.Process(this->Display_Power_Mode);
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

	const auto globalAnchor = ResourceTypeClass::Global_Display_Anchor.Get();

	// If global anchor is Sidebar: only draw during sidebar pass.
	// If global anchor is NOT Sidebar (TopLeft, TopRight, etc.): only draw during tactical game screen pass.
	if (globalAnchor == ResourceDisplayAnchor::Sidebar)
	{
		if (!isSidebar)
			return;
	}
	else
	{
		if (isSidebar)
			return;
	}

	RectangleStruct vRect = pSurface->GetRect();

	int baseX = 0;
	int baseY = 0;
	const Point2D baseOffset = ResourceTypeClass::Global_Display_BaseOffset.Get();

	const int marginX = Phobos::Config::MessageDisplayInCenter ? 28 : 10;
	const int screenW = pSurface->GetWidth();
	const int screenH = pSurface->GetHeight();

	if (isSidebar)
	{
		baseX = 20 + pSideExt->Sidebar_ResourceTypes_Offset.Get().X + baseOffset.X;
		baseY = 2 + pSideExt->Sidebar_ResourceTypes_Offset.Get().Y + baseOffset.Y;
	}
	else
	{
		switch (globalAnchor)
		{
		case ResourceDisplayAnchor::TopLeft:
			baseX = marginX + baseOffset.X;
			baseY = 5 + baseOffset.Y;
			break;
		case ResourceDisplayAnchor::TopRight:
			baseX = screenW - marginX + baseOffset.X;
			baseY = 5 + baseOffset.Y;
			break;
		case ResourceDisplayAnchor::TopCenter:
			baseX = (screenW / 2) + baseOffset.X;
			baseY = 5 + baseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomLeft:
			baseX = marginX + baseOffset.X;
			baseY = screenH - 20 + baseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomRight:
			baseX = screenW - marginX + baseOffset.X;
			baseY = screenH - 20 + baseOffset.Y;
			break;
		case ResourceDisplayAnchor::BottomCenter:
			baseX = (screenW / 2) + baseOffset.X;
			baseY = screenH - 20 + baseOffset.Y;
			break;
		default:
			baseX = marginX + baseOffset.X;
			baseY = 5 + baseOffset.Y;
			break;
		}
	}

	int const spacing = ResourceTypeClass::Global_Display_Spacing.Get();
	int stackIndex = 0;
	int runningRightX = baseX;
	int runningLeftX = baseX;

	TextAlign effectiveAlign;
	if (ResourceTypeClass::Global_Display_Align.isset())
	{
		effectiveAlign = ResourceTypeClass::Global_Display_Align.Get();
	}
	else
	{
		if (isSidebar)
			effectiveAlign = pSideExt->Sidebar_ResourceTypes_Align.Get();
		else if (globalAnchor == ResourceDisplayAnchor::TopCenter || globalAnchor == ResourceDisplayAnchor::BottomCenter)
			effectiveAlign = TextAlign::Center;
		else if (globalAnchor == ResourceDisplayAnchor::TopRight || globalAnchor == ResourceDisplayAnchor::BottomRight)
			effectiveAlign = TextAlign::Right;
		else
			effectiveAlign = TextAlign::Left;
	}

	const bool isRightAlign = (effectiveAlign == TextAlign::Right);
	const bool isCenterAlign = (effectiveAlign == TextAlign::Center);
	const bool isBottomAnchor = !isSidebar && (globalAnchor == ResourceDisplayAnchor::BottomLeft || globalAnchor == ResourceDisplayAnchor::BottomRight || globalAnchor == ResourceDisplayAnchor::BottomCenter);

	const bool hasExplicitSidebarTypes = !pSideExt->Sidebar_ResourceTypes_Types.empty();
	const size_t count = (isSidebar && hasExplicitSidebarTypes) ? pSideExt->Sidebar_ResourceTypes_Types.size() : ResourceTypeClass::Array.size();

	bool bgDrawn = false;
	auto DrawBackgroundOnce = [&]()
	{
		if (bgDrawn || isSidebar)
			return;
		bgDrawn = true;

		int bgWidth = 0;
		int bgHeight = 0;
		BSurface* pPCX = ResourceTypeClass::Global_Display_Background_PCX.GetSurface();
		SHPStruct* pSHP = ResourceTypeClass::Global_Display_Background_SHP.Get();

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
		else
		{
			return;
		}

		Point2D const bgOffset = ResourceTypeClass::Global_Display_Background_Offset.Get();
		Point2D bgPos;
		if (isRightAlign)
			bgPos.X = (baseX - bgWidth) + bgOffset.X;
		else if (isCenterAlign)
			bgPos.X = (baseX - bgWidth / 2) + bgOffset.X;
		else
			bgPos.X = baseX + bgOffset.X;

		bgPos.Y = (isBottomAnchor ? (baseY - bgHeight) : baseY) + bgOffset.Y;

		if (pPCX)
		{
			RectangleStruct drawRect { bgPos.X, bgPos.Y, bgWidth, bgHeight };
			PCX::Instance.BlitToSurface(&drawRect, pSurface, pPCX);
		}
		else if (pSHP)
		{
			ConvertClass* pConvert = ResourceTypeClass::Global_Display_Background_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
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

	for (size_t iter = 0; iter < count; ++iter)
	{
		const int i = (isSidebar && hasExplicitSidebarTypes) ? pSideExt->Sidebar_ResourceTypes_Types[iter] : static_cast<int>(iter);
		if (i < 0 || i >= static_cast<int>(ResourceTypeClass::Array.size()))
			continue;

		const auto pResource = ResourceTypeClass::Array[i].get();
		if (!pResource || !pHouseExt->IsResourceEnabled(i))
			continue;

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
				else if (pHouseExt->GetResourceAmount(i) <= 0)
				{
					continue;
				}
			}

			if (pResource->Display_Condition.Get() == ResourceDisplayCondition::HasCollector)
				continue;
		}

		DrawBackgroundOnce();

		wchar_t counter[0x40];
		const wchar_t* label = pResource->Display_Label.Get();
		ColorStruct clrToolTip;

		if (pResource->IsPowerResource())
		{
			if (!label || !*label)
				label = Phobos::UI::PowerLabel;

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

			if (pResource->Display_Color.isset())
			{
				clrToolTip = pResource->Display_Color.Get();
			}
			else
			{
				if (pPlayer->PowerBlackoutTimer.InProgress())
				{
					clrToolTip = pSideExt->Sidebar_PowerDelta_ColorGrey.Get();
				}
				else
				{
					const double percent = output != 0
						? (double)drain / (double)output
						: (drain != 0 ? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow);

					clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
						? pSideExt->Sidebar_PowerDelta_ColorGreen.Get()
						: LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
						? pSideExt->Sidebar_PowerDelta_ColorYellow.Get()
						: pSideExt->Sidebar_PowerDelta_ColorRed.Get();
				}
			}
		}
		else
		{
			if (pResource->IsMoneyResource() && (!label || !*label))
				label = Phobos::UI::CostLabel;

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
		else
		{
			auto const dim = Drawing::GetTextDimensions(counter, { 0, 0 }, 0, 2, 0);

			if (ResourceTypeClass::Global_Display_Orientation.Get() == ResourceDisplayOrientation::Horizontal)
			{
				if (isRightAlign)
				{
					vPos.X = runningRightX - dim.Width;
					vPos.Y = baseY;
					runningRightX -= (dim.Width + spacing);
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

				vPos.Y = isBottomAnchor ? (baseY - stackIndex * spacing) : (baseY + stackIndex * spacing);
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

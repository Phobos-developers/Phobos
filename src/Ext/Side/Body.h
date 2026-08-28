#pragma once
#include <SideClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ResourceTypeClass.h>

class SideExt final : public AbstractTypeExt
{
public:
	using base_type = SideClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = SideExt;

	static constexpr DWORD Canary = 0x05B10501;

public:
	// typed owner accessor
	SideClass* OwnerObject() const
	{
		return static_cast<SideClass*>(this->GetAttachedObject());
	}

	Valueable<int> ArrayIndex;
	Valueable<bool> Sidebar_GDIPositions;
	Valueable<int> IngameScore_WinTheme;
	Valueable<int> IngameScore_LoseTheme;
	Valueable<Point2D> Sidebar_HarvesterCounter_Offset;
	Valueable<bool> Sidebar_HarvesterCounter_HideMaxValue;
	Valueable<bool> Sidebar_HarvesterCounter_OnlyMaxValue;
	Nullable<ColorStruct> Sidebar_HarvesterCounter_ColorGreen;
	Valueable<ColorStruct> Sidebar_HarvesterCounter_ColorYellow;
	Valueable<ColorStruct> Sidebar_HarvesterCounter_ColorRed;
	Valueable<Point2D> Sidebar_WeedsCounter_Offset;
	Nullable<ColorStruct> Sidebar_WeedsCounter_Color;
	Valueable<Point2D> Sidebar_ProducingProgress_Offset;
	Valueable<Point2D> Sidebar_PowerDelta_Offset;
	Valueable<ColorStruct> Sidebar_PowerDelta_ColorGreen;
	Valueable<ColorStruct> Sidebar_PowerDelta_ColorYellow;
	Valueable<ColorStruct> Sidebar_PowerDelta_ColorRed;
	Valueable<ColorStruct> Sidebar_PowerDelta_ColorGrey;
	Valueable<TextAlign> Sidebar_PowerDelta_Align;
	Valueable<Point2D> Sidebar_ResourceTypes_Offset;
	Nullable<ColorStruct> Sidebar_ResourceTypes_Color;
	Valueable<TextAlign> Sidebar_ResourceTypes_Align;
	ValueableIdxVector<ResourceTypeClass> Sidebar_ResourceTypes_Types;
	Nullable<ColorStruct> ToolTip_Background_Color;
	Nullable<int> ToolTip_Background_Opacity;
	Nullable<float> ToolTip_Background_BlurSize;
	Valueable<int> BriefingTheme;
	ValueableIdx<ColorScheme> MessageTextColor;
	PhobosPCXFile SuperWeaponSidebar_OnPCX;
	PhobosPCXFile SuperWeaponSidebar_OffPCX;
	PhobosPCXFile SuperWeaponSidebar_TopPCX;
	PhobosPCXFile SuperWeaponSidebar_CenterPCX;
	PhobosPCXFile SuperWeaponSidebar_BottomPCX;
	Valueable<ResourceDisplayOrientation> Display_ResourceTypes_Orientation;
	Valueable<ResourceDisplayAnchor> Display_ResourceTypes_Anchor;
	Valueable<Point2D> Display_ResourceTypes_BaseOffset;
	Valueable<int> Display_ResourceTypes_Spacing;
	Nullable<TextAlign> Display_ResourceTypes_Align;
	PhobosPCXFile Display_ResourceTypes_Background_PCX;
	Valueable<SHPStruct*> Display_ResourceTypes_Background_SHP;
	CustomPalette Display_ResourceTypes_Background_Palette;
	Valueable<Point2D> Display_ResourceTypes_Background_Offset;
	Valueable<bool> Display_ResourceTypes_Background_Horizontal_ResourcesInside;

	SideExt(SideClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, ArrayIndex { -1 }
		, Sidebar_GDIPositions { false }
		, IngameScore_WinTheme { -2 }
		, IngameScore_LoseTheme { -2 }
		, Sidebar_HarvesterCounter_Offset { { 0, 0 } }
		, Sidebar_HarvesterCounter_HideMaxValue { false }
		, Sidebar_HarvesterCounter_OnlyMaxValue { false }
		, Sidebar_HarvesterCounter_ColorGreen { }
		, Sidebar_HarvesterCounter_ColorYellow { { 255, 255, 0 } }
		, Sidebar_HarvesterCounter_ColorRed { { 255, 0, 0 } }
		, Sidebar_WeedsCounter_Offset { { 0, 0 } }
		, Sidebar_WeedsCounter_Color {}
		, Sidebar_ProducingProgress_Offset { { 0, 0 } }
		, Sidebar_PowerDelta_Offset { { 0, 0 } }
		, Sidebar_PowerDelta_ColorGreen { { 0, 255, 0 } }
		, Sidebar_PowerDelta_ColorYellow { { 255, 255, 0 } }
		, Sidebar_PowerDelta_ColorRed { { 255, 0, 0 } }
		, Sidebar_PowerDelta_ColorGrey { { 0x80,0x80,0x80 } }
		, Sidebar_PowerDelta_Align { TextAlign::Left }
		, Sidebar_ResourceTypes_Offset { { 0, 0 } }
		, Sidebar_ResourceTypes_Color {}
		, Sidebar_ResourceTypes_Align { TextAlign::Left }
		, Sidebar_ResourceTypes_Types {}
		, ToolTip_Background_Color { }
		, ToolTip_Background_Opacity { }
		, ToolTip_Background_BlurSize { }
		, BriefingTheme { -1 }
		, MessageTextColor { -1 }
		, SuperWeaponSidebar_OnPCX {}
		, SuperWeaponSidebar_OffPCX {}
		, SuperWeaponSidebar_TopPCX {}
		, SuperWeaponSidebar_CenterPCX {}
		, SuperWeaponSidebar_BottomPCX {}
		, Display_ResourceTypes_Orientation { ResourceDisplayOrientation::Vertical }
		, Display_ResourceTypes_Anchor { ResourceDisplayAnchor::TopRight }
		, Display_ResourceTypes_BaseOffset { { 0, 0 } }
		, Display_ResourceTypes_Spacing { 14 }
		, Display_ResourceTypes_Align {}
		, Display_ResourceTypes_Background_PCX {}
		, Display_ResourceTypes_Background_SHP { nullptr }
		, Display_ResourceTypes_Background_Palette {}
		, Display_ResourceTypes_Background_Offset { { 0, 0 } }
		, Display_ResourceTypes_Background_Horizontal_ResourcesInside { false }
	{ }

	virtual ~SideExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<SideExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static SideExt* Fetch(const SideClass* pThis)
	{
		return AbstractExt::Fetch<SideExt>(pThis);
	}

	static SideExt* TryFetch(const SideClass* pThis)
	{
		return AbstractExt::TryFetch<SideExt>(pThis);
	}
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};


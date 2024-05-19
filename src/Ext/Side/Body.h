#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SideExt
{
public:
	using base_type = SideClass;

	static constexpr DWORD Canary = 0x05B10501;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<SideClass>
	{
	public:
		Valueable<int> ArrayIndex;
		Valueable<bool> Sidebar_GDIPositions;
		Valueable<int> IngameScore_WinTheme;
		Valueable<int> IngameScore_LoseTheme;
		Valueable<Point2D> Sidebar_HarvesterCounter_Offset;
		Valueable<ColorStruct> Sidebar_HarvesterCounter_Yellow;
		Valueable<ColorStruct> Sidebar_HarvesterCounter_Red;
		Valueable<Point2D> Sidebar_WeedsCounter_Offset;
		Nullable<ColorStruct> Sidebar_WeedsCounter_Color;
		Valueable<Point2D> Sidebar_ProducingProgress_Offset;
		Valueable<Point2D> Sidebar_PowerDelta_Offset;
		Valueable<ColorStruct> Sidebar_PowerDelta_Green;
		Valueable<ColorStruct> Sidebar_PowerDelta_Yellow;
		Valueable<ColorStruct> Sidebar_PowerDelta_Red;
		Valueable<ColorStruct> Sidebar_PowerDelta_Grey;
		Valueable<TextAlign> Sidebar_PowerDelta_Align;
		Nullable<ColorStruct> ToolTip_Background_Color;
		Nullable<int> ToolTip_Background_Opacity;
		Nullable<float> ToolTip_Background_BlurSize;
		Valueable<int> BriefingTheme;

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject)
			, ArrayIndex { -1 }
			, Sidebar_GDIPositions { false }
			, IngameScore_WinTheme { -2 }
			, IngameScore_LoseTheme { -2 }
			, Sidebar_HarvesterCounter_Offset { { 0, 0 } }
			, Sidebar_HarvesterCounter_Yellow { { 255, 255, 0 } }
			, Sidebar_HarvesterCounter_Red { { 255, 0, 0 } }
			, Sidebar_WeedsCounter_Offset { { 0, 0 } }
			, Sidebar_WeedsCounter_Color {}
			, Sidebar_ProducingProgress_Offset { { 0, 0 } }
			, Sidebar_PowerDelta_Offset { { 0, 0 } }
			, Sidebar_PowerDelta_Green { { 0, 255, 0 } }
			, Sidebar_PowerDelta_Yellow { { 255, 255, 0 } }
			, Sidebar_PowerDelta_Red { { 255, 0, 0 } }
			, Sidebar_PowerDelta_Grey { { 0x80,0x80,0x80 } }
			, Sidebar_PowerDelta_Align { TextAlign::Left }
			, ToolTip_Background_Color { }
			, ToolTip_Background_Opacity { }
			, ToolTip_Background_BlurSize { }
			, BriefingTheme { -1 }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SideExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

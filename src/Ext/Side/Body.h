// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
		ValueableIdx<ColorScheme> MessageTextColor;
		PhobosPCXFile SuperWeaponSidebar_OnPCX;
		PhobosPCXFile SuperWeaponSidebar_OffPCX;
		PhobosPCXFile SuperWeaponSidebar_TopPCX;
		PhobosPCXFile SuperWeaponSidebar_CenterPCX;
		PhobosPCXFile SuperWeaponSidebar_BottomPCX;

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
			, MessageTextColor { -1 }
			, SuperWeaponSidebar_OnPCX {}
			, SuperWeaponSidebar_OffPCX {}
			, SuperWeaponSidebar_TopPCX {}
			, SuperWeaponSidebar_CenterPCX {}
			, SuperWeaponSidebar_BottomPCX {}
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

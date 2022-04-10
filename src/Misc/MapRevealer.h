#pragma once

#include <TacticalClass.h>
#include <HouseClass.h>
#include <Utilities/Helpers.Alex.h>

class MapRevealer
{
public:
	MapRevealer(const CoordStruct& coords);

	MapRevealer(const CellStruct& cell);

	const CellStruct& Base() const
	{
		return this->BaseCell;
	}

	void Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const;

	void Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const;

	void UpdateShroud(size_t start, size_t radius, bool fog = false) const;

	void Process0(CellClass* const pCell, bool unknown, bool fog, bool add) const;

	void Process1(CellClass* const pCell, bool fog, bool add) const;

	bool IsCellAllowed(const CellStruct& cell) const
	{
		if (this->RequiredChecks)
		{
			for (const auto& checkedCell : CheckedCells)
			{
				if (checkedCell == cell)
				{
					return false;
				}
			}
		}
		return true;
	}

	bool IsCellAvailable(const CellStruct& cell) const
	{
		auto const sum = cell.X + cell.Y;

		return sum > this->MapWidth
			&& cell.X - cell.Y < this->MapWidth
			&& cell.Y - cell.X < this->MapWidth
			&& sum <= this->MapWidth + 2 * this->MapHeight;
	}

	bool CheckLevel(const CellStruct& offset, int level) const
	{
		auto const cellLevel = this->Base() + offset + GetRelation(offset) - this->CellOffset;
		return MapClass::Instance->GetCellAt(cellLevel)->Level < level + CellClass::BridgeLevels;
	}

	static bool AffectsHouse(HouseClass* const pHouse)
	{
		auto& Player = HouseClass::Player;

		if (pHouse == Player)
		{
			return true;
		}

		if (!pHouse || !Player)
		{
			return false;
		}

		return pHouse->RadarVisibleTo.Contains(Player) ||
			(RulesClass::Instance->AllyReveal && pHouse->IsAlliedWith(Player));
	}

	static bool RequiresExtraChecks()
	{
		auto& Session = SessionClass::Instance;
		return Helpers::Alex::is_any_of(Session->GameMode, GameMode::LAN, GameMode::Internet) &&
			Session->MPGameMode && !Session->MPGameMode->vt_entry_04();
	}

	static CellStruct GetRelation(const CellStruct& offset)
	{
		return{ static_cast<short>(Math::sgn(-offset.X)),
			static_cast<short>(Math::sgn(-offset.Y)) };
	}

private:
	CellStruct TranslateBaseCell(const CoordStruct& coords) const
	{
		auto const adjust = (TacticalClass::AdjustForZ(coords.Z) / -30) << 8;
		auto const baseCoords = coords + CoordStruct { adjust, adjust, 0 };
		return CellClass::Coord2Cell(baseCoords);
	}

	CellStruct GetOffset(const CoordStruct& coords, const CellStruct& base) const
	{
		return base - CellClass::Coord2Cell(coords) - CellStruct { 2, 2 };
	}

	template <typename T>
	void RevealImpl(const CoordStruct& coords, int radius, HouseClass* pHouse, bool onlyOutline, bool allowRevealByHeight, T func) const;

	CellStruct BaseCell;
	CellStruct CellOffset;
	CellStruct CheckedCells[3];
	bool RequiredChecks;
	int MapWidth;
	int MapHeight;

public:
	// Reveal_Area
	static void __fastcall MapClass_RevealArea0(MapClass* pThis, void*, CoordStruct* pCoord,
		int nRadius, HouseClass* pHouse, int bOutlineOnly, bool bNoShroudUpdate, bool bFog,
		bool bAllowRevealByHeight, bool bHideOnRadar)
	{
		MapRevealer const revealer(*pCoord);
		revealer.Reveal0(*pCoord, nRadius, pHouse, bOutlineOnly, bNoShroudUpdate, bFog, bAllowRevealByHeight, bHideOnRadar);
		revealer.UpdateShroud(0, static_cast<size_t>(std::max(nRadius, 0)), false);
	}

	// Sight_From
	static void __fastcall MapClass_RevealArea1(MapClass* pThis, void*, CoordStruct* pCoord,
		int nRadius, HouseClass* pHouse, int bOutlineOnly, bool bNoShroudUpdate, bool bFog,
		bool bAllowRevealByHeight, bool bIncreaseShroudCounter)
	{
		MapRevealer const revealer(*pCoord);
		revealer.Reveal1(*pCoord, nRadius, pHouse, bOutlineOnly, bFog, bAllowRevealByHeight, bIncreaseShroudCounter);
	}

	static void __fastcall MapClass_RevealArea2(MapClass* pThis, void*,
		CoordStruct* Coords, int Height, int Radius, bool bSkipReveal)
	{
		MapRevealer const revealer(*Coords);
		revealer.UpdateShroud(static_cast<size_t>(std::max(Height, 0)), static_cast<size_t>(std::max(Radius, 0)), bSkipReveal);
	}
};

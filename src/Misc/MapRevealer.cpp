#include "MapRevealer.h"

#include <MouseClass.h>

MapRevealer::MapRevealer(const CoordStruct& coords) :
	BaseCell(this->TranslateBaseCell(coords)),
	CellOffset(this->GetOffset(coords, this->Base())),
	RequiredChecks(RequiresExtraChecks())
{
	auto const& Rect = MapClass::Instance->MapRect;
	this->MapWidth = Rect.Width;
	this->MapHeight = Rect.Height;

	this->CheckedCells[0] = { 7, static_cast<short>(this->MapWidth + 5) };
	this->CheckedCells[1] = { 13, static_cast<short>(this->MapWidth + 11) };
	this->CheckedCells[2] = { static_cast<short>(this->MapHeight + 13),
		static_cast<short>(this->MapHeight + this->MapWidth - 15) };
}

MapRevealer::MapRevealer(const CellStruct& cell)
	: MapRevealer(MapClass::Instance->GetCellAt(cell)->GetCoordsWithBridge())
{
}

template <typename T>
void MapRevealer::RevealImpl(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool const onlyOutline, bool const allowRevealByHeight, T func) const
{
	auto const level = coords.Z / *reinterpret_cast<int*>(0xABDE88);
	auto const& base = this->Base();

	if (this->AffectsHouse(pHouse) && this->IsCellAvailable(base) && radius > 0)
	{
		auto const spread = std::min(static_cast<size_t>(radius), CellSpreadEnumerator::Max);
		auto const spread_limit_sqr = (spread + 1) * (spread + 1);

		auto const start = (!RulesClass::Instance->RevealByHeight && onlyOutline && spread > 2)
			? spread - 3 : 0u;

		auto const checkLevel = allowRevealByHeight && RulesClass::Instance->RevealByHeight;

		for (CellSpreadEnumerator it(spread, start); it; ++it)
		{
			auto const& offset = *it;
			auto const cell = base + offset;
			if (this->IsCellAvailable(cell))
			{
				if (std::abs(offset.X) <= static_cast<int>(spread) && offset.MagnitudeSquared() < spread_limit_sqr)
				{
					if (!checkLevel || this->CheckLevel(offset, level))
					{
						auto pCell = MapClass::Instance->GetCellAt(cell);
						func(pCell);
					}
				}
			}
		}
	}
};

void MapRevealer::Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell)
 {
	 this->Process0(pCell, unknown, fog, add);
	});
}

void MapRevealer::Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell)
 {
	 this->Process1(pCell, fog, add);
	});
}

void MapRevealer::UpdateShroud(size_t start, size_t radius, bool fog) const
{
	if (!fog)
	{
		auto const& base = this->Base();
		radius = std::min(radius, CellSpreadEnumerator::Max);
		start = std::min(start, CellSpreadEnumerator::Max - 3);

		for (CellSpreadEnumerator it(radius, start); it; ++it)
		{
			auto const& offset = *it;
			auto const cell = base + offset;

			auto const pCell = MapClass::Instance->GetCellAt(cell);

			auto shroudedness = TacticalClass::Instance->GetOcclusion(cell, false);
			if (pCell->Foggedness != shroudedness)
			{
				pCell->Foggedness = static_cast<char>(shroudedness);
				pCell->VisibilityChanged = true;
				TacticalClass::Instance->RegisterCellAsVisible(pCell);
			}
		}
	}
}

void MapRevealer::Process0(CellClass* const pCell, bool unknown, bool fog, bool add) const
{
	pCell->Flags &= ~0x40;

	if (this->IsCellAllowed(pCell->MapCoords))
	{
		if (fog)
		{
			if ((pCell->Flags & 3) != 3 && pCell->CopyFlags & cf2_NoShadow)
			{
				MouseClass::Instance->vt_entry_98(pCell->MapCoords, HouseClass::Player);
			}
		}
		else
		{
			if ((pCell->CopyFlags & 0x18) != 0x18 || (pCell->Flags & 3) != 3)
			{
				if (!unknown)
				{
					if (add)
					{
						MouseClass::Instance->vt_entry_94(pCell->MapCoords, HouseClass::Player, false);
					}
					else
					{
						pCell->Unshroud();
					}
				}
			}
		}
	}
}

void MapRevealer::Process1(CellClass* const pCell, bool fog, bool add) const
{
	pCell->Flags &= ~0x40;

	if (fog)
	{
		if ((pCell->Flags & 3) != 3 && pCell->CopyFlags & cf2_NoShadow)
		{
			MouseClass::Instance->vt_entry_98(pCell->MapCoords, HouseClass::Player);
		}
	}
	else
	{
		if (this->IsCellAllowed(pCell->MapCoords))
		{
			MouseClass::Instance->vt_entry_94(pCell->MapCoords, HouseClass::Player, add);
		}
	}
}

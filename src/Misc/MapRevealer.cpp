#include "MapRevealer.h"

#include <MouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/Macro.h>

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

MapRevealer::MapRevealer(const CellStruct& cell) :
	MapRevealer(MapClass::Instance->GetCellAt(cell)->GetCoordsWithBridge())
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

			bool bFlag = false;
			if (pCell->Visibility != 0xFF)
			{
				auto shroudOcculusion = TacticalClass::Instance->GetOcclusion(cell, false);
				if (pCell->Visibility != shroudOcculusion)
				{
					pCell->Visibility = shroudOcculusion;
					pCell->VisibilityChanged = true;
					bFlag = true;
				}
			}

			if (!ScenarioClass::Instance->SpecialFlags.FogOfWar && !bFlag)
			{
				continue;
			}

			if (pCell->Foggedness != 0xFF)
			{
				auto foggedOcclusion = TacticalClass::Instance->GetOcclusion(cell, true);
				if (pCell->Foggedness != foggedOcclusion)
				{
					pCell->Foggedness = foggedOcclusion;
					bFlag = true;
				}
			}

			if (bFlag)
			{
				TacticalClass::Instance->RegisterCellAsVisible(pCell);
			}

			/*auto shroudOcclusion = TacticalClass::Instance->GetOcclusion(cell, false);
			if (pCell->Visibility != shroudOcclusion) {
				pCell->Visibility = shroudOcclusion;
				pCell->VisibilityChanged = true;
				TacticalClass::Instance->RegisterCellAsVisible(pCell);
			}*/
		}
	}
}

void MapRevealer::Process0(CellClass* const pCell, bool unknown, bool fog, bool add) const
{
	pCell->Flags &= ~CellFlags::IsPlot;

	if (this->IsCellAllowed(pCell->MapCoords))
	{
		if (fog)
		{
			if ((pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed && pCell->AltFlags & AltCellFlags::Mapped)
			{
				MouseClass::Instance->MapCellFoggedness(&pCell->MapCoords, HouseClass::Player);
			}
		}
		else
		{
			if ((pCell->AltFlags & AltCellFlags::Clear) != AltCellFlags::Clear || (pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed)
			{
				if (!unknown)
				{
					if (add)
					{
						MouseClass::Instance->RevealFogShroud(&pCell->MapCoords, HouseClass::Player, false);
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
	pCell->Flags &= ~CellFlags::IsPlot;

	if (fog)
	{
		if ((pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed && pCell->AltFlags & AltCellFlags::Mapped)
		{
			MouseClass::Instance->MapCellFoggedness(&pCell->MapCoords, HouseClass::Player);
		}
	}
	else
	{
		if (this->IsCellAllowed(pCell->MapCoords))
		{
			MouseClass::Instance->RevealFogShroud(&pCell->MapCoords, HouseClass::Player, add);
		}
	}
}

DEFINE_POINTER_LJMP(0x5673A0, MapRevealer::MapClass_RevealArea0);

DEFINE_POINTER_LJMP(0x5678E0, MapRevealer::MapClass_RevealArea1);

DEFINE_POINTER_LJMP(0x567DA0, MapRevealer::MapClass_RevealArea2);
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
#include "SelectBoxTypeClass.h"

template<>
const char* Enumerable<SelectBoxTypeClass>::GetMainSection()
{
	return "SelectBoxTypes";
}

void SelectBoxTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (!_stricmp(pSection, NONE_STR) || !pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Shape.Read(exINI, pSection, "Shape");
	this->Palette.LoadFromINI(pINI, pSection, "Palette");
	this->Frames.Read(exINI, pSection, "Frames");
	this->Offset.Read(exINI, pSection, "Offset");
	this->Translucency.Read(exINI, pSection, "Translucency");
	this->VisibleToHouses.Read(exINI, pSection, "VisibleToHouses");
	this->VisibleToHouses_Observer.Read(exINI, pSection, "VisibleToHouses.Observer");
	this->DrawAboveTechno.Read(exINI, pSection, "DrawAboveTechno");
	this->GroundShape.Read(exINI, pSection, "GroundShape");
	this->GroundPalette.LoadFromINI(pINI, pSection, "GroundPalette");
	this->GroundFrames.Read(exINI, pSection, "GroundFrames");
	this->GroundOffset.Read(exINI, pSection, "GroundOffset");
	this->Ground_AlwaysDraw.Read(exINI, pSection, "Ground.AlwaysDraw");
	this->GroundLine.Read(exINI, pSection, "GroundLine");
	this->GroundLineColor.Read(exINI, pSection, "GroundLineColor.%s");
	this->GroundLine_Dashed.Read(exINI, pSection, "GroundLine.Dashed");
}

template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Frames)
		.Process(this->Offset)
		.Process(this->Translucency)
		.Process(this->VisibleToHouses)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->DrawAboveTechno)
		.Process(this->GroundShape)
		.Process(this->GroundPalette)
		.Process(this->GroundFrames)
		.Process(this->GroundOffset)
		.Process(this->Ground_AlwaysDraw)
		.Process(this->GroundLine)
		.Process(this->GroundLineColor)
		.Process(this->GroundLine_Dashed)
		;
}

void SelectBoxTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void SelectBoxTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

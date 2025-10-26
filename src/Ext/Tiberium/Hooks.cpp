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
#include "Body.h"

#include <CellClass.h>
#include <OverlayClass.h>

#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x47C210, CellClass_CellColor_TiberiumRadarColor, 0x6)
{
	enum { ReturnFromFunction = 0x47C23F };

	GET(CellClass*, pThis, ESI);
	GET_STACK(ColorStruct*, arg0, STACK_OFFSET(0x14, 0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFSET(0x14, 0x8));

	const int tiberiumType = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);

	if (tiberiumType < 0)
		return 0;

	const auto pTiberium = TiberiumClass::Array.GetItem(tiberiumType);

	if (const auto pTiberiumExt = TiberiumExt::ExtMap.TryFind(pTiberium))
	{
		if (pTiberiumExt->MinimapColor.isset())
		{
			auto& color = pTiberiumExt->MinimapColor.Get();

			arg0->R = color.R;
			arg0->G = color.G;
			arg0->B = color.B;

			arg4->R = color.R;
			arg4->G = color.G;
			arg4->B = color.B;

			R->ECX(arg4);
			R->AL(color.B);

			return ReturnFromFunction;
		}
	}

	return 0;
}

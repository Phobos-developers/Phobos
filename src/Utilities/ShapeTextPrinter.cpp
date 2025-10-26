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


#include "ShapeTextPrinter.h"

void ShapeTextPrinter::PrintShape
(
	const char* text,
	ShapeTextPrintData& data,
	Point2D position,
	RectangleStruct& bounds,
	DSurface* pSurface,
	Point2D offset,
	BlitterFlags blitterFlags,
	int brightness,
	int tintColor
)
{
	const int length = strlen(text);
	std::vector<int> frames;
	frames.reserve(length);

	for (int i = 0; i < length; i++)
	{
		int frame = 0;

		if (isdigit(text[i]))
		{
			frame = data.BaseNumberFrame + text[i] - '0';
		}
		else
		{
			size_t signIndex = SignSequence.find(text[i]);

			if (signIndex < SignSequence.size())
				frame = data.BaseExtraFrame + signIndex;
			else
				return;
		}

		frames.emplace_back(frame);
	}

	for (int frame : frames)
	{
		pSurface->DrawSHP
		(
			const_cast<ConvertClass*>(data.Palette),
			const_cast<SHPStruct*>(data.Shape),
			frame,
			&position,
			&bounds,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			brightness,
			tintColor,
			nullptr,
			0,
			0,
			0
		);

		position.X += data.Spacing.X;
		position.Y -= data.Spacing.Y;
	}
}
